#include <Engine/MeshEdit/ARAP.h>
#include <Engine/MeshEdit/MinSurf.h>
#include <Engine/MeshEdit/Paramaterize.h>
#include <Engine/Primitive/TriMesh.h>
#include <Eigen/Sparse>
#include <Eigen/SparseQR>
#include <set>

using namespace Ubpa;
using namespace Eigen;
using namespace std;

Ubpa::ARAP::ARAP(Ptr<TriMesh> triMesh, int iter_n, int log_verbosity)
	: heMesh(make_shared<HEMesh<V>>())
	, heMesh_orig(make_shared<HEMesh<V>>())
	, iter_n(iter_n), log_verbosity(log_verbosity)
{
	Init(triMesh, false, true);

	// copy it into triMesh_orig
}

void Ubpa::ARAP::Clear()
{
	heMesh->Clear();
	heMesh_orig->Clear();
	triMesh = nullptr;
}

std::array<pointf2, 3> Ubpa::ARAP::genEmbed(pointf3 v0, pointf3 v1, pointf3 v2) {
	vecf3 v10 = v1 - v0;
	vecf3 v20 = v2 - v0;
	vecf3 v21 = v2 - v1;

	float cos_theta = v10.cos_theta(v20);
	return std::array{ pointf2(0, 0), pointf2(v10.norm(), 0)
		, pointf2(v20.norm() * cos_theta, v20.norm() * sqrtf(1 - (cos_theta * cos_theta))) };
}

bool Ubpa::ARAP::Init(Ptr<TriMesh> triMesh, bool noClear, bool initOrig)
{
	if (!noClear)
		Clear();

	if (triMesh == nullptr)
		return true;

	if (triMesh->GetType() == TriMesh::INVALID) {
		printf("ERROR::ARAP::Init:\n"
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

	if (initOrig) {
		heMesh_orig->Clear();
		heMesh_orig->Reserve(nV);
		heMesh_orig->Init(triangles);
	}

	if (!heMesh->IsTriMesh() || !heMesh->HaveBoundary()) {
		printf("ERROR::ARAP::Init:\n"
			"\t""trimesh is not a triangle mesh or hasn't a boundaries\n");
		heMesh->Clear();
		if (initOrig) {
			heMesh_orig->Clear();
		}
		return false;
	}

	// triangle mesh's positions ->  half-edge structure's positions
	for (int i = 0; i < nV; i++) {
		auto v = heMesh->Vertices().at(i);
		v->pos = triMesh->GetPositions()[i].cast_to<vecf3>();
		if (initOrig) {
			auto u = heMesh_orig->Vertices().at(i);
			u->pos = triMesh->GetPositions()[i].cast_to<vecf3>();
		}
	}

	this->triMesh = triMesh;
	return true;
}

// calculate energy with HeMesh

//namespace Ubpa {
//	template<typename V, typename E, typename P>
//	const std::vector<E*> TPolygon<V, E, P>::BoundaryEdges() {
//		std::vector<E*> edges;
//		for (auto he : BoundaryHEs())
//			edges.push_back(he->Edge());
//		return edges;
//	}
//
//	template<typename V, typename E, typename P>
//	const std::vector<V*> TPolygon<V, E, P>::BoundaryVertice() {
//		std::vector<V*> vertices;
//		for (auto he : BoundaryHEs())
//			vertices.push_back(he->Origin());
//		return vertices;
//	}
//}

Ubpa::ARAP::V* Ubpa::ARAP::findOpp(P* trig, V* v1, V* v2) {
	decltype(v1) opp_v = nullptr;;
	if (trig != nullptr) {
		for (auto vv : trig->BoundaryVertice()) {
			if (vv != v1 && vv != v2) {
				opp_v = vv;
			}
		}
		assert(opp_v != nullptr);
	}
	return opp_v;
}

std::tuple<Vector2f, Vector2f> Ubpa::ARAP::calcXVecFull(int xi, int xj) {
	auto vi = heMesh_orig->Vertices()[xi];
	auto vj = heMesh_orig->Vertices()[xj];

	auto tri_ij = vi->HalfEdgeTo(vj)->Polygon();
	auto tri_ji = vj->HalfEdgeTo(vi)->Polygon();

	Vector2f x_vec_ij = Vector2f::Zero();
	Vector2f x_vec_ji = Vector2f::Zero();

	// calculate x vec in mapped 2d space
	auto getIdx = [](decltype(tri_ij) trig, decltype(vi) vert) {
		int i = 0;
		for (auto vv : trig->BoundaryVertice()) {
			if (vv == vert) {
				break;
			}
			else {
				i++;
			}
		}
		assert(i < 3);
		return i;
	};

	if (tri_ij != nullptr) {
		auto tri_ij_embed = genEmbed(tri_ij->BoundaryVertice()[0]->pos.cast_to<pointf3>()
			, tri_ij->BoundaryVertice()[1]->pos.cast_to<pointf3>()
			, tri_ij->BoundaryVertice()[2]->pos.cast_to<pointf3>());

		int vi_idx = getIdx(tri_ij, vi);
		int vj_idx = getIdx(tri_ij, vj);
		x_vec_ij = { (tri_ij_embed[vi_idx] - tri_ij_embed[vj_idx])[0]
						 , (tri_ij_embed[vi_idx] - tri_ij_embed[vj_idx])[1] };
		
		if (log_verbosity > 40) {
			printf("- embed_ij: ");
			print(tri_ij_embed);
			printf("- x_vec_ij: \n");
			cout << x_vec_ij << endl;
		}
	}
	
	if (tri_ji != nullptr) {
		auto tri_ji_embed = genEmbed(tri_ji->BoundaryVertice()[0]->pos.cast_to<pointf3>()
			, tri_ji->BoundaryVertice()[1]->pos.cast_to<pointf3>()
			, tri_ji->BoundaryVertice()[2]->pos.cast_to<pointf3>());

		int vi_idx = getIdx(tri_ji, vi);
		int vj_idx = getIdx(tri_ji, vj);
		x_vec_ji = { (tri_ji_embed[vi_idx] - tri_ji_embed[vj_idx])[0]
						 , (tri_ji_embed[vi_idx] - tri_ji_embed[vj_idx])[1] };

		if (log_verbosity > 40) {
			printf("- embed_ji: ");
			print(tri_ji_embed);
			printf("- x_vec_ji: \n");
			cout << x_vec_ji << endl;
		}
	}

	return std::make_tuple(x_vec_ij, x_vec_ji);

}

// x_vec, cotij, cotji
std::tuple<Vector2f, double, double> Ubpa::ARAP::calcXVec(int xi, int xj) {
	auto vi = heMesh_orig->Vertices()[xi];
	auto vj = heMesh_orig->Vertices()[xj];

	auto tri_ij = vi->HalfEdgeTo(vj)->Polygon();
	auto tri_ji = vj->HalfEdgeTo(vi)->Polygon();

	if (log_verbosity > 50) {
		cout << "- tri_ij: ";
		print(tri_ij);

		cout << "- tri_ji: ";
		print(tri_ji);
	}

	// mapped NaN (inf) to zero
	auto getCtgf = [](float cosine) {
		return (cosine == 1 || cosine == -1) ? 0 : (cosine / (sqrtf(1 - cosine * cosine)));
	};

	decltype(vj) opp_v_ji = findOpp(tri_ji, vi, vj);
	decltype(vj) opp_v_ij = findOpp(tri_ij, vi, vj);

	double cos_theta_ij = opp_v_ij != nullptr ? (vi->pos - opp_v_ij->pos).cos_theta(vj->pos - opp_v_ij->pos) : 1;
	double cos_theta_ji = opp_v_ji != nullptr ? (vi->pos - opp_v_ji->pos).cos_theta(vj->pos - opp_v_ji->pos) : 1;

	double cot_theta_ij = getCtgf(cos_theta_ij);
	double cot_theta_ji = getCtgf(cos_theta_ji);

	// calculate x vec in mapped 2d space
	auto getIdx = [](decltype(tri_ij) trig, decltype(vi) vert) {
		int i = 0;
		for (auto vv : trig->BoundaryVertice()) {
			if (vv == vert) {
				break;
			} else {
				i++;
			}
		}
		assert(i < 3);
		return i;
	};

	decltype(tri_ij) trig;
	// only one trig necessary
	if (tri_ij == nullptr) {
		trig = tri_ji;
	} else {
		trig = tri_ij;
	}

	if (log_verbosity > 50) {
		printf("- embed_input: ");
		print(trig);
	}

	auto tri_embed = genEmbed(trig->BoundaryVertice()[0]->pos.cast_to<pointf3>()
				, trig->BoundaryVertice()[1]->pos.cast_to<pointf3>()
				, trig->BoundaryVertice()[2]->pos.cast_to<pointf3>());

	if (log_verbosity > 40) {
		printf("- embed: ");
		print(tri_embed);
	}

	int vi_idx = getIdx(trig, vi);
	int vj_idx = getIdx(trig, vj);
	Vector2f x_vec = { (tri_embed[vi_idx] - tri_embed[vj_idx])[0]
					 , (tri_embed[vi_idx] - tri_embed[vj_idx])[1] };

	return std::make_tuple(x_vec, cot_theta_ij, cot_theta_ji);
}

void Ubpa::ARAP::UniformPara() {
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

	for (auto v : boundaries[0]) {
		boundary_points.insert(v->Origin());
	}

	for (auto v : heMesh->Vertices()) {
		if (!boundary_points.contains(v)) {
			inner_points.insert(v);
		}
	}

	//const float boost_factor = 100;
	const float boost_factor = 1;
	// Fix our boundary

	int points_total = boundary_points.size();
	float step = 4.0f / points_total;

	float curr = 0;
	for (auto v : boundary_points) {
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

	// Build sparse matrix
	size_t n = inner_points.size();
	SparseMatrix<float> coeff_mat(n, n);
	coeff_mat.setZero();
	VectorXf b_vec_x = VectorXf::Zero(n);
	VectorXf b_vec_y = VectorXf::Zero(n);
	VectorXf b_vec_z = VectorXf::Zero(n);

	cout << "coeff mat build start" << endl;

	int current_row = 0;
	for (auto v : inner_points) {
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
				coeff_mat.insert(current_row, adjidx) = -1.0f / degree;
			}
		}

		// add itself
		// todo add assert
		coeff_mat.insert(current_row, vidx) = 1;
		current_row++;
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

void Ubpa::ARAP::print(Ubpa::ARAP::V* v) {
	cout << "V: (" << v->pos[0] << ", " << v->pos[1] << ", " << v->pos[2] << ")";
}
void Ubpa::ARAP::print(Ubpa::ARAP::P* p) {
	if (p == nullptr) {
		cout << "P: nullptr" << endl;
		return;
	}
	cout << "P: [";
	print(p->BoundaryVertice()[0]);
	cout << ", ";
	print(p->BoundaryVertice()[1]);
	cout << ", ";
	print(p->BoundaryVertice()[2]);
	cout << "]" << endl;
}


void Ubpa::ARAP::print(pointf2& v) {
	cout << "V: (" << v[0] << ", " << v[1] << ")";
}

void Ubpa::ARAP::print(std::array<pointf2, 3>& p) {
	cout << "P: [";
	print(p[0]);
	cout << ", ";
	print(p[1]);
	cout << ", ";
	print(p[2]);
	cout << "]" << endl;
}

double Ubpa::ARAP::calcEnergy(vector<Matrix2f> L_t)
{
	double energy = 0;

	auto polys = heMesh->Polygons();
	for (int poly_id = 0; poly_id < polys.size(); poly_id++) {
		if (log_verbosity > 50) {
			print(polys[poly_id]);
		}

		auto hes = polys[poly_id]->BoundaryHEs();
		for (auto he : hes) {
			
			double cotij;
			Vector2f x_vec;
			std::tie(x_vec, cotij, std::ignore) = calcXVec(heMesh->Index(he->Origin()), heMesh->Index(he->End()));

			if (log_verbosity > 50) {
				cout << x_vec << endl;
			}

			Vector2f u_vec = { he->Origin()->pos[0] - he->End()->pos[0] , he->Origin()->pos[1] - he->End()->pos[1] };

			double energy_component = cotij * (u_vec - L_t[poly_id] * x_vec).squaredNorm();
			if (log_verbosity > 50) {
				printf("* energy += %lf\n", energy_component);
			}
			energy += energy_component;
		}
	}

	energy /= 2;

	return energy;
}

void Ubpa::ARAP::DoARAP()
{
	cout << "DoARAP() called." << endl;

	// initialize 2d mapping
	UniformPara();
	//for (auto v : heMesh->Vertices()) {
	//	v->pos[2] = 0;
	//}

	for (auto v : heMesh->Vertices()) {
		assert(v->pos[2] == 0);
	}
	
	// envvars
	int total_polys = heMesh->NumPolygons();
	auto polys = heMesh->Polygons();
	auto polys_orig = heMesh_orig->Polygons();

	// initialize L_t
	vector<Matrix2f> L_t;
	for (int i = 0; i < total_polys; i++) {
		L_t.push_back(Eigen::Matrix2f::Identity());
		//L_t.push_back(Eigen::Matrix2f::Zero());
	}

	printf("Start with energy=%lf\n", calcEnergy(L_t));

	for (int iter_count = 0; iter_count < iter_n; iter_count++) {
		{
			// local phase, update L_t
			for (int poly_id = 0; poly_id < total_polys; poly_id++) {
				// calculate S_t(u) in place of J_t(u)
				Matrix2f S_u = Eigen::Matrix2f::Zero();

				auto verts = polys[poly_id]->BoundaryVertice();
				auto verts_orig = polys_orig[poly_id]->BoundaryVertice();
				// todo assuming counterclockwise
				// actually okay for both

				for (int i = 0; i < 3; i++) {
					// cot = cos / sin
					int dest_angle = (i + 2) % 3;
					double cos_theta;
					auto embed = genEmbed(verts_orig[0]->pos.cast_to<pointf3>()
						, verts_orig[1]->pos.cast_to<pointf3>()
						, verts_orig[2]->pos.cast_to<pointf3>());

					if (dest_angle == 0) {
						cos_theta = (embed[2] - embed[0]).cos_theta(embed[1] - embed[0]);
					}
					else if (dest_angle == 1) {
						cos_theta = (embed[0] - embed[1]).cos_theta(embed[2] - embed[1]);
					}
					else {
						cos_theta = (embed[0] - embed[2]).cos_theta(embed[1] - embed[2]);
					}

					double cot_theta = cos_theta / (sqrt(1 - cos_theta * cos_theta) + 1e-5);  // todo check 1e-5

					Vector2f u_vec = { verts[i]->pos[0] - verts[(i + 1) % 3]->pos[0] , verts[i]->pos[1] - verts[(i + 1) % 3]->pos[1] };
					Vector2f x_vec = { embed[i][0] - embed[(i + 1) % 3][0] , embed[i][1] - embed[(i + 1) % 3][1] };

					S_u += cot_theta * (u_vec * x_vec.transpose());
				}

				JacobiSVD<Matrix2f> svd(S_u, ComputeFullU | ComputeFullV);
				assert(svd.singularValues()(0) >= 0 && svd.singularValues()(1) >= 0);

				// the ARAP case
				Matrix2f res = svd.matrixU() * svd.matrixV().transpose();
				L_t[poly_id] = res;
			}
		}
		printf("[%d] local iteration done, energy=%lf\n", iter_count, calcEnergy(L_t));
		// global iteration
		{
			// anchors containing vertices index
			std::set<int> anchors;
			//anchors.insert(0);
			//anchors.insert(1);

			int total_vertices = heMesh->NumVertices();
			int vertices_unknown = total_vertices - anchors.size();
			//assert(total_polys == vertices_unknown + 2);

			std::unordered_map<int, int> index_map;
			{
				int cur_row = 0;
				for (int i = 0; i < total_vertices; i++) {
					if (anchors.count(i) > 0) {
						continue;
					}
					else {
						index_map[i] = cur_row;
						cur_row++;
					}
				}
				assert(cur_row == vertices_unknown);
			}

			Eigen::SparseMatrix<float> coeff_mat(vertices_unknown, vertices_unknown);
			Eigen::VectorXf b_vec_x = VectorXf::Zero(vertices_unknown);
			Eigen::VectorXf b_vec_y = VectorXf::Zero(vertices_unknown);
			coeff_mat.setZero();

			int current_row = 0;
			for (int i = 0; i < total_vertices; i++) {
				if (anchors.count(i) > 0) {
					continue;
				}

				// prepare coefficient
				// calculate cos_theta_ij
				auto v = heMesh->Vertices()[i];
				//cout << "Processing vertice: " << v << endl;

				for (auto adjv : v->AdjVertices()) {
					int j = heMesh->Index(adjv);

					//auto he_ji = v->HalfEdgeTo(adjv);
					//auto he_ij = adjv->HalfEdgeTo(v);
					auto he_ji = adjv->HalfEdgeTo(v);
					auto he_ij = v->HalfEdgeTo(adjv);

					auto tri_ji = he_ji->Polygon();
					auto tri_ij = he_ij->Polygon();

					double ctg_ij, ctg_ji;

					if (log_verbosity > 50) {
						cout << "* i: ";
						print(v);
						cout << endl;

						cout << "* j: ";
						print(adjv);
						cout << endl;
					}



					std::tie(std::ignore, ctg_ij, ctg_ji) = calcXVec(heMesh->Index(v), heMesh->Index(adjv));

					if (log_verbosity > 50) {
						printf("* ctg_ij=%lf , ctg_ji=%lf\n", ctg_ij, ctg_ji);
					}

					// the u stuff
					if (anchors.count(j) > 0) {
						// put into b
						b_vec_x(current_row) += (ctg_ij + ctg_ji) * adjv->pos[0];
						b_vec_y(current_row) += (ctg_ij + ctg_ji) * adjv->pos[1];
					}
					else {
						coeff_mat.coeffRef(current_row, index_map[j]) += -(ctg_ij + ctg_ji);
					}

					coeff_mat.coeffRef(current_row, index_map[i]) += (ctg_ij + ctg_ji);

					// the x stuff

					Vector2f x_vec_ij, x_vec_ji;
					std::tie(x_vec_ij, x_vec_ji) = calcXVecFull(heMesh->Index(v), heMesh->Index(adjv));

					Vector2f rhs = Vector2f::Zero();

					if (tri_ij != nullptr) {
						rhs += ctg_ij * L_t[heMesh->Index(he_ij->Polygon())] * x_vec_ij;
					}

					if (tri_ji != nullptr) {
						rhs += ctg_ji * L_t[heMesh->Index(he_ji->Polygon())] * x_vec_ji;
					}

					if (log_verbosity > 50) {
						cout << "rhs: " << endl;
						cout << rhs << endl;
					}

					b_vec_x(current_row) += rhs(0);
					b_vec_y(current_row) += rhs(1);
				}
				current_row++;
			}

			// Solve
			SparseQR<SparseMatrix<float>, COLAMDOrdering<int>> solver;

			cout << "begin makeCompressed()" << endl;
			coeff_mat.makeCompressed();

			if (log_verbosity > 50) {
				cout << coeff_mat << endl;
			}

			cout << "begin compute()" << endl;
			solver.compute(coeff_mat);
			if (solver.info() != Eigen::Success) {
				cout << "solver: decomposition was not successful." << endl;
				return;
			}

			if (log_verbosity > 50) {
				cout << "b_vec_x: " << endl;
				cout << b_vec_x << endl;

				cout << "b_vec_y: " << endl;
				cout << b_vec_y << endl;
			}

			VectorXf x_res = solver.solve(b_vec_x);
			cout << "error: " << (coeff_mat * x_res - b_vec_x).squaredNorm() << " (x_res)" << endl;

			VectorXf y_res = solver.solve(b_vec_y);
			cout << "error: " << (coeff_mat * y_res - b_vec_y).squaredNorm() << " (y_res)" << endl;;

			// write back
			for (int i = 0; i < total_vertices; i++) {
				if (anchors.count(i) > 0) {
					continue;
				}
				else {
					heMesh->Vertices()[i]->pos = { x_res(index_map[i]), y_res(index_map[i]), 0 };
				}
			}

		}
		printf("[%d] global iteration done, energy=%lf\n", iter_count, calcEnergy(L_t));
	}
}

bool Ubpa::ARAP::Run()
{
	if (heMesh->IsEmpty() || !triMesh) {
		printf("ERROR::ARAP::Run\n"
			"\t""heMesh->IsEmpty() || !triMesh\n");
		return false;
	}

	DoARAP();

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


	//vector<pointf2> texcoords;
	//for (auto v : heMesh->Vertices())
	//	texcoords.push_back(v->pos.cast_to<pointf2>());

	//triMesh->Update(texcoords);

	return true;
}
