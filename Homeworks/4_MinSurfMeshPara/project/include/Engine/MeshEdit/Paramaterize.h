#pragma once

#include <Basic/HeapObj.h>
#include <UHEMesh/HEMesh.h>
#include <UGM/UGM>


namespace Ubpa {
	class TriMesh;
	class MinSurf;

	// mesh boundary == 1
	class Paramaterize : public HeapObj {
	public:
		Paramaterize(Ptr<TriMesh> triMesh, bool uSquare, bool uUniform);
	public:
		static const Ptr<Paramaterize> New(Ptr<TriMesh> triMesh, bool uSquare, bool uUniform) {
			return Ubpa::New<Paramaterize>(triMesh, uSquare, uUniform);
		}
	public:
		void Clear();
		bool Init(Ptr<TriMesh> triMesh);
		void DoPara();
		bool Run();

	private:
		class V;
		class E;
		class P;
		class V : public TVertex<V, E, P> {
		public:
			vecf3 pos;
		};
		class E : public TEdge<V, E, P> { };
		class P :public TPolygon<V, E, P> { };
	private:
		friend class MinSurf; // *What is the use of this?*

		Ptr<TriMesh> triMesh;
		const Ptr<HEMesh<V>> heMesh; // vertice order is same with triMesh
		bool useSquare;
		bool useUniform;
	};
}
