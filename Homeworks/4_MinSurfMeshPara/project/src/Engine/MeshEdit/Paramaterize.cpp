#include <Engine/MeshEdit/Paramaterize.h>
#include <Engine/MeshEdit/MinSurf.h>
#include <Engine/Primitive/TriMesh.h>
#include <Eigen/Sparse>
#include <Eigen/SparseQR>

using namespace Ubpa;
using namespace Eigen;
using namespace std;

Paramaterize::Paramaterize(Ptr<TriMesh> triMesh, bool uSquare, bool uUniform) 	
    : heMesh(make_shared<HEMesh<V>>()), useSquare(uSquare), useUniform(uUniform) {
	Init(triMesh);
}

void Paramaterize::Clear() {
	heMesh->Clear();
	triMesh = nullptr;
}

bool Paramaterize::Init(Ptr<TriMesh> triMesh) {
	Clear();

	if (triMesh == nullptr)
		return true;

	if (triMesh->GetType() == TriMesh::INVALID) {
		printf("ERROR::Parameterize::Init:\n"
			"\t""trimesh is invalid\n");
		return false;
	}

	// init half-edge structure
	size_t nV = triMesh->GetPositions().size();
	vector<vector<size_t>> triangles;
	triangles.reserve(triMesh->GetTriangles().size());
	for (auto triangle : triMesh->GetTriangles())
		triangles.push_back({ triangle->idx[0], triangle->idx[1], triangle->idx[2] });
	heMesh->Reserve(nV);
	heMesh->Init(triangles);

	if (!heMesh->IsTriMesh() || !heMesh->HaveBoundary()) {
		printf("ERROR::Parameterize::Init:\n"
			"\t""trimesh is not a triangle mesh or hasn't a boundaries\n");
		heMesh->Clear();
		return false;
	}

	// triangle mesh's positions ->  half-edge structure's positions
	for (int i = 0; i < nV; i++) {
		auto v = heMesh->Vertices().at(i);
		v->pos = triMesh->GetPositions()[i].cast_to<vecf3>();
	}

	this->triMesh = triMesh;
	return true;
}

void Paramaterize::DoPara() {
	// First, detect and fix boundary
	random_set<V*> boundary_points;
	random_set<V*> inner_points;

	auto boundaries = this->heMesh->Boundaries();
	if (boundaries.size() != 1) {
		cout << "ERROR::Parameterize::DoPara:" << endl
			 << "\t" << "got boundaries = " << boundaries.size()
			 << " (expect 1)" << endl;
		return;
	}

	for (auto v: boundaries[0]) {
		boundary_points.insert(v->Origin());
	}

	for (auto v: heMesh->Vertices()) {
		if (!boundary_points.contains(v)) {
			inner_points.insert(v);
		}
	}

	//const float boost_factor = 100;
	const float boost_factor = 1;
	// Fix our boundary
	if (useSquare) {
		int points_total = boundary_points.size();
		float step = 4.0f / points_total;

		float curr = 0;
		for (auto v: boundary_points) {
			vecf3 new_pos;
			if (curr >= 0 && curr < 1) {
				new_pos[0] = curr;
				new_pos[1] = new_pos[2] = 0;
			} else if (curr >= 1 && curr < 2) {
				new_pos[0] = 1;
				new_pos[1] = curr - 1;
				new_pos[2] = 0;
			} else if (curr >= 2 && curr < 3) {
				new_pos[0] = 1 - (curr - 2);
				new_pos[1] = 1;
				new_pos[2] = 0;
			} else { // curr >= 3; remember to cut off as fp precision is an issue
				new_pos[0] = 0;
				new_pos[1] = curr > 4 ? 4 : 4 - curr;
				new_pos[2] = 0;
			}
			v->pos = new_pos * boost_factor;
			curr += step;
		}

	} else {
		int points_total = boundary_points.size();
		float step = 2 * 3.1415926f / points_total;

		float curr = 0;
		for (auto v: boundary_points) {
			vecf3 new_pos;
			new_pos[0] = cosf(curr);
			new_pos[1] = sinf(curr);
			new_pos[2] = 0;

			v->pos = new_pos * boost_factor;
			curr += step;
		}
	}

	// Build sparse matrix
	size_t n = inner_points.size();
	SparseMatrix<float> coeff_mat(n, n);
	coeff_mat.setZero();
	VectorXf b_vec_x = VectorXf::Zero(n);
	VectorXf b_vec_y = VectorXf::Zero(n);
	VectorXf b_vec_z = VectorXf::Zero(n);

	cout << "coeff mat build start" << endl;

	if (useUniform) {
		int current_row = 0;
		for (auto v: inner_points) {
			// vidx CERTAINLY follows order (and it's redundant)
			size_t vidx = inner_points.idx(v);
			auto adj = v->AdjVertices();
			size_t degree = v->Degree();
			for (auto adjv : adj) {
				// check type
				if (boundary_points.contains(adjv)) { // this set is usually smaller
					b_vec_x(current_row) += (1.0f / degree) * adjv->pos[0];
					b_vec_y(current_row) += (1.0f / degree) * adjv->pos[1];
					b_vec_z(current_row) += (1.0f / degree) * adjv->pos[2];
				} else { // inner
					assert(inner_points.contains(adjv));
					size_t adjidx = inner_points.idx(adjv);
					// todo add assert = 0
					coeff_mat.insert(current_row, adjidx) = - 1.0f / degree;
				}
			}

			// add itself
			// todo add assert
			coeff_mat.insert(current_row, vidx) = 1;
			current_row++;
		}
	} else { // use cotangent weight

		int fallback_met = 0;
		int current_row = 0;
		for (auto v: inner_points) {
			// vidx CERTAINLY follows order (and it's redundant)
			size_t vidx = inner_points.idx(v);
			auto adj = v->AdjVertices();
			size_t degree = v->Degree();

			float weight_sum = 0;
			bool needFallback = false;
			for (auto adjv : adj) {
				// check type

				float weight = 0;  // this is required to be positive
				auto adjadjv = adjv->AdjVertices();

				decltype(adjadjv) intersect;
				for (auto vv : adjadjv) {
					for (auto vvv : adj) {
						if (vv == vvv && vv != v) {
							intersect.push_back(vv);
						}
					}
				}
				assert(intersect.size() == 2);
					
				// calculate angels of two
				// (v_i - adjadjv[0]) & (adjadjv[0] - adjv)
				// float cos_alpha = (v->pos - adjadjv[0]->pos).cos_theta(adjadjv[0]->pos - adjv->pos) 
				// 				/ (v->pos - adjadjv[0]->pos).sin_theta(adjadjv[0]->pos - adjv->pos);
				float ctheta_alpha = -1 * (v->pos - intersect[0]->pos).cos_theta(intersect[0]->pos - adjv->pos);
				float ctg_alpha = ctheta_alpha / (sqrtf(1 - ctheta_alpha * ctheta_alpha) + 1e-5);
				
				float ctheta_beta = -1 * (v->pos - intersect[1]->pos).cos_theta(intersect[1]->pos - adjv->pos);
				float ctg_beta = ctheta_beta / (sqrtf(1 - ctheta_beta * ctheta_beta) + 1e-5);

				// needFallback = true;
				// break;
				if (ctg_alpha + ctg_beta <= 0) {
					needFallback = true;
					break;
				} else {
					weight = ctg_alpha + ctg_beta;
					weight_sum += weight;
				}

				if (boundary_points.contains(adjv)) { // this set is usually smaller
					b_vec_x(current_row) += weight * adjv->pos[0];
					b_vec_y(current_row) += weight * adjv->pos[1];
					b_vec_z(current_row) += weight * adjv->pos[2];
				} else {
					assert(inner_points.contains(adjv));
					size_t adjidx = inner_points.idx(adjv);

					coeff_mat.insert(current_row, adjidx) = - weight;
				}
			}

			if (needFallback) {
				// update all b_vec, reassign coeff_mat and update weight_sum
				b_vec_x(current_row) = 0;
				b_vec_y(current_row) = 0;
				b_vec_z(current_row) = 0;

				for (auto adjv : adj) {
					// check type
					if (boundary_points.contains(adjv)) { // this set is usually smaller
						b_vec_x(current_row) += (1.0f / degree) * adjv->pos[0];
						b_vec_y(current_row) += (1.0f / degree) * adjv->pos[1];
						b_vec_z(current_row) += (1.0f / degree) * adjv->pos[2];
					} else { // inner
						assert(inner_points.contains(adjv));
						size_t adjidx = inner_points.idx(adjv);
						// todo add assert = 0
						coeff_mat.coeffRef(current_row, adjidx) = - 1.0f / degree; // since no one knows whether it's inserted or not
					}
				}

				weight_sum = 1;
				fallback_met++;
			}

			// add itself
			// todo add assert
			coeff_mat.insert(current_row, vidx) = weight_sum;
			current_row++;
		}
		cout << "met " << fallback_met << " fallback(s)." << endl;
	}

	cout << "coeff mat build complete" << endl;

	// Solve
	SparseQR<SparseMatrix<float>, COLAMDOrdering<int>> solver;

	cout << "begin makeCompressed()" << endl;
	coeff_mat.makeCompressed();

	cout << "begin compute()" << endl;
	solver.compute(coeff_mat);
	if (solver.info() != Eigen::Success) {
		cout << "solver: decomposition was not successful." << endl;
		return;
	}

	cout << "begin solve() for x" << endl;
	VectorXf res_x = solver.solve(b_vec_x);

	cout << "begin solve() for y" << endl;
	VectorXf res_y = solver.solve(b_vec_y);

	cout << "begin solve() for z" << endl;
	VectorXf res_z = solver.solve(b_vec_z);

	// Update vertex coordinates
	for (int i = 0; i < n; i++) {
		// find the corresponding point
		auto v = inner_points[i];
		vecf3 new_pos = { res_x(i), res_y(i), res_z(i) }; // works?

		//cout << new_pos << endl;
		v->pos = new_pos;
	}

}

bool Paramaterize::Run() {
	if (heMesh->IsEmpty() || !triMesh) {
		printf("ERROR::Parameterize::Run\n"
			"\t""heMesh->IsEmpty() || !triMesh\n");
		return false;
	}

	DoPara();

	// half-edge structure -> triangle mesh
	size_t nV = heMesh->NumVertices();
	size_t nF = heMesh->NumPolygons();
	vector<pointf3> positions;
	vector<unsigned> indice;
	positions.reserve(nV);
	indice.reserve(3 * nF);
	for (auto v : heMesh->Vertices())
		positions.push_back(v->pos.cast_to<pointf3>());
	for (auto f : heMesh->Polygons()) { // f is triangle
		for (auto v : f->BoundaryVertice()) // vertices of the triangle
			indice.push_back(static_cast<unsigned>(heMesh->Index(v)));
	}

	triMesh->Init(indice, positions);

	return true;
}
