#include <Engine/MeshEdit/MinSurf.h>

#include <Engine/Primitive/TriMesh.h>

#include <Eigen/Sparse>
#include <Eigen/SparseQR>

using namespace Ubpa;

using namespace std;
using namespace Eigen;

MinSurf::MinSurf(Ptr<TriMesh> triMesh)
	: heMesh(make_shared<HEMesh<V>>())
{
	Init(triMesh);
}

void MinSurf::Clear() {
	heMesh->Clear();
	triMesh = nullptr;
}

bool MinSurf::Init(Ptr<TriMesh> triMesh) {
	Clear();

	if (triMesh == nullptr)
		return true;

	if (triMesh->GetType() == TriMesh::INVALID) {
		printf("ERROR::MinSurf::Init:\n"
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
		printf("ERROR::MinSurf::Init:\n"
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

bool MinSurf::Run() {
	if (heMesh->IsEmpty() || !triMesh) {
		printf("ERROR::MinSurf::Run\n"
			"\t""heMesh->IsEmpty() || !triMesh\n");
		return false;
	}

	Minimize();

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

void MinSurf::Minimize() {
	// First, detect and fix boundary
	random_set<V*> boundary_points;
	random_set<V*> inner_points;

	auto boundaries = this->heMesh->Boundaries();
	if (boundaries.size() != 1) {
		cout << "ERROR::MinSurf::Minimize:" << endl
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
	// Build sparse matrix
	size_t n = inner_points.size();
	SparseMatrix<float> coeff_mat(n, n);
	coeff_mat.setZero();
	VectorXf b_vec_x = VectorXf::Zero(n);
	VectorXf b_vec_y = VectorXf::Zero(n);
	VectorXf b_vec_z = VectorXf::Zero(n);

	cout << "coeff mat build start" << endl;

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

	cout << "coeff mat build complete" << endl;

	// Solve
	SparseQR<SparseMatrix<float>, COLAMDOrdering<int>> solver;

	cout << "begin makeCompressed()" << endl;
	coeff_mat.makeCompressed();

	cout << "begin compute()" << endl;
	solver.compute(coeff_mat);
	if (solver.info() != Eigen::Success) {
		cout << "solver: decomposition was not successfull." << endl;
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
