#pragma once

#include <Basic/HeapObj.h>
#include <UHEMesh/HEMesh.h>
#include <UGM/UGM>
#include <vector>
#include <Eigen/Dense>

namespace Ubpa {
	class TriMesh;
	class MinSurf;

	// mesh boundary == 1
	class ARAP : public HeapObj {
	public:
		ARAP(Ptr<TriMesh> triMesh, int iter_n, int log_verbosity);
	public:
		static const Ptr<ARAP> New(Ptr<TriMesh> triMesh, int iter_n, int log_verbosity) {
			return Ubpa::New<ARAP>(triMesh, iter_n, log_verbosity);
		}

	private:
		class V;
		class E;
		class P;
		class V : public TVertex<V, E, P> {
		public:
			vecf3 pos;
		};
		class E : public TEdge<V, E, P> { };
		class P : public TPolygon<V, E, P> { };


		void print(Ubpa::ARAP::V* v);
		void print(Ubpa::ARAP::P* p);
		void print(pointf2 &v);
		void print(std::array<pointf2, 3> &p);

	public:
		void Clear();
		bool Init(Ptr<TriMesh> triMesh, bool noClear, bool initOrig);
		void DoARAP();
		bool Run();
		double calcEnergy(std::vector<Eigen::Matrix2f> L_t);
		std::tuple<Eigen::Vector2f, Eigen::Vector2f> calcXVecFull(int xi, int xj);
		std::tuple<Eigen::Vector2f, double, double> calcXVec(int xi, int xj);
		std::array<pointf2, 3> genEmbed(pointf3 v0, pointf3 v1, pointf3 v2);
		V* findOpp(P* trig, V* v1, V* v2);

		void UniformPara();

	private:
		friend class Paramaterize; // *What is the use of this?*

		Ptr<TriMesh> triMesh;
		const Ptr<HEMesh<V>> heMesh; // vertice order is same with triMesh
		const Ptr<HEMesh<V>> heMesh_orig; // the original heMesh before parameterization
									      // with each vertice corresponding to the current
		int iter_n;
		int log_verbosity;
	};
}
