#pragma once

#include <Eigen/Sparse>
#include <Basic/HeapObj.h>
//#include <Engine/Primitive/MassSpring.h>
#include <UGM/UGM>
#include <set>

namespace Ubpa {
	class Simulate : public HeapObj {
	public:
		Simulate(const std::vector<pointf3>& plist,
			const std::vector<unsigned>& elist) {
			edgelist = elist;
			this->positions.resize(plist.size());
			for (int i = 0; i < plist.size(); i++)
			{
				for (int j = 0; j < 3; j++)
				{
					this->positions[i][j] = plist[i][j];
				}
			}
			this->stiff = 0.1;
			this->ext_force = 0;
			this->curr_time = 0;
		};
	public:
		static const Ptr<Simulate> New(const std::vector<pointf3>& plist,
			const std::vector<unsigned> &elist) {
			return Ubpa::New<Simulate>(plist, elist);
		}
	public:
		// clear cache data
		void Clear();

		// init cache data (eg. half-edge structure) for Run()
		bool Init();
		//bool Init();

		// call it after Init()
		bool Run();
		
		const std::vector<pointf3>& GetPositions() const { return positions; };

		const float GetStiff() { return stiff; };
		void SetStiff(float k) { stiff = k; Init();};
		const float GetExtForce() { return ext_force; }
		void SetExtForce(float k) { ext_force = k; 
			Init();
		};

		void SetParam(const char* key, int value) {
			if (!strcmp(key, "boundary-radius")) {
				boundary_radius = value;
				std::cout << "Radius: " << boundary_radius << std::endl;
			}
			else if (!strcmp(key, "log-verbosity")) {
				log_verbosity = value;
				std::cout << "Verbosity: " << log_verbosity << std::endl;
			}
		}

		void updateExtForce() {
			for (int i = 0; i < positions.size(); i++) {
				if (i == 0) {
					this->f_ext[i] = vecf3{ 0, ext_force * sin(5 * curr_time), 0 };
				}
				else {
					this->f_ext[i] = vecf3{ 0, 0, 0 };
				}
			}
		}

		const float GetTimeStep() { return h; };
		void SetTimeStep(float k) { h = k; Init();};

		// patched by libre liu..
		const std::vector<unsigned>& GetFix() {
			static std::vector<unsigned> ret = std::vector<unsigned>(this->fixed_id.begin(), this->fixed_id.end());
			return ret;
		};
		void SetFix(const std::vector<unsigned>& f) { 
			this->fixed_id.clear();
			for (auto id : f) {
				this->fixed_id.insert(id);
			}
			Init();
		};
		const std::vector<pointf3>& GetVelocity() { return velocity; };
		//void SetVelocity(const std::vector<pointf3>& v) { velocity = v; };

		void SetLeftFix();

	private:
		// kernel part of the algorithm
		void SimulateOnce();
		Eigen::Matrix3f calcNablaFintComponent(int i, int j, const std::vector<pointf3>& pos_step_k);
		float calcResidual(const std::vector<vecf3>& f_int, const Eigen::VectorXf& x_new, const std::vector<pointf3>& pos_step_k);
		float getOrigLen(int i, int j);
		float getK(int i, int j);
		std::vector<unsigned> getAdjV(int vi);
		void updateKMat();
		Eigen::Vector3f calcFint(int i, int j, const std::vector<pointf3>& pos_step_k);


		// i is the global index
		inline Eigen::Vector3f calcY(int i) {
			auto v_res = this->positions[i].cast_to<vecf3>()
				+ h * this->velocity[i].cast_to<vecf3>()
				+ h * h
				* vecf3{ this->f_ext[i][0] / this->mass[i], this->f_ext[i][1] / this->mass[i] , this->f_ext[i][2] / this->mass[i] };
			return Eigen::Vector3f{ v_res[0], v_res[1], v_res[2] };
		}

		inline Eigen::Vector3f calcG(int i, const Eigen::Vector3f &x_n, const Eigen::Vector3f& f_int) {
			return Eigen::Matrix3f::Identity() * this->mass[i] * (x_n - calcY(i)) - h * h * f_int;
		}

	private:
		float h = 0.03f;  //²½³¤
		float ext_force;
		float stiff;
		float curr_time;
		std::set<unsigned> fixed_id;  //fixed point id
		std::map<unsigned, unsigned> xf_to_x;
		int boundary_radius = 0;
		int log_verbosity = 5;
		Eigen::SparseMatrix<float> K_mat;
		

		//mesh data
		std::vector<unsigned> edgelist;

		struct pair_hash
		{
			template<class T1, class T2>
			std::size_t operator() (const std::pair<T1, T2>& p) const
			{
				auto h1 = std::hash<T1>{}(p.first);
				auto h2 = std::hash<T2>{}(p.second);
				return h1 ^ h2;
			}
		};

		std::unordered_map<std::pair<unsigned, unsigned>, float, pair_hash> adj_map;

		//simulation data
		std::vector<pointf3> positions;
		std::vector<pointf3> velocity;
		std::vector<float> mass;
		std::vector<vecf3> f_ext;

		
	};
}
