#include "PathTracer.h"

#include <UBL/Image.h>

#include <iostream>

#include <thread>

using namespace Ubpa;
using namespace std;

PathTracer::PathTracer(const Scene* scene, const SObj* cam_obj, Image* img)
	: scene{ scene },
	bvh{ const_cast<Scene*>(scene) },
	img{ img },
	cam{ cam_obj->Get<Cmpt::Camera>() },
	ccs{ cam->GenCoordinateSystem(cam_obj->Get<Cmpt::L2W>()->value) }
{
	IntersectorVisibility::Instance();
	IntersectorClosest::Instance();

	scene->Each([this](const Cmpt::Light* light) ->bool {
		if (!vtable_is<EnvLight>(light->light.get()))
			return true; // continue

		env_light = static_cast<const EnvLight*>(light->light.get());
		return false; // stop
	});

	// TODO: preprocess env_light here
	const int env_prob_granularity = 100;
	float* env_prob_distr = (float*)malloc(sizeof(float) * env_prob_granularity * env_prob_granularity);
	memset(env_prob_distr, 0, sizeof(float) * env_prob_granularity * env_prob_granularity);

	float weight_total = 0;
	for (int i = 0; i < env_prob_granularity; i++) {
		for (int j = 0; j < env_prob_granularity; j++) {
			float illu = env_light->texture->Sample(
							pointf2{ (float)i / env_prob_granularity, (float)j / env_prob_granularity }
							).to_rgb().illumination();
			env_prob_distr[i * env_prob_granularity + j] = illu;

			weight_total += illu;
		}
	}

	// p_img(i, j)
	for (int i = 0; i < env_prob_granularity; i++) {
		for (int j = 0; j < env_prob_granularity; j++) {
			env_prob_distr[i * env_prob_granularity + j] /= weight_total;
		}
	}
}

void PathTracer::Run() {
	img->SetAll(0.f);

#ifndef NDEBUG
	const size_t spp = 2; // samples per pixel
#else
	const size_t spp = 256;
#endif

#ifdef NDEBUG
	const size_t core_num = std::thread::hardware_concurrency();
	auto work = [this, core_num, spp](size_t id) {
		for (size_t j = id; j < img->height; j += core_num) {
			for (size_t i = 0; i < img->width; i++) {
				for (size_t k = 0; k < spp; k++) {
					float u = (i + rand01<float>() - 0.5f) / img->width;
					float v = (j + rand01<float>() - 0.5f) / img->height;
					rayf3 r = cam->GenRay(u, v, ccs);
					rgbf Lo;
					do { Lo = Shade(IntersectorClosest::Instance().Visit(&bvh, r), -r.dir, true); }
					while (Lo.has_nan());
					img->At<rgbf>(i, j) += Lo / float(spp);
				}
			}
			float progress = (j + 1) / float(img->height);
			cout << progress << endl;
		}
	};
	vector<thread> workers;
	for (size_t i = 0; i < core_num; i++)
		workers.emplace_back(work, i);
	for (auto& worker : workers)
		worker.join();
#else
	for (size_t j = 0; j < img->height; j++) {
		for (size_t i = 0; i < img->width; i++) {
			for (size_t k = 0; k < spp; k++) {
				float u = (i + rand01<float>() - 0.5f) / img->width;
				float v = (j + rand01<float>() - 0.5f) / img->height;
				rayf3 r = cam->GenRay(u, v, ccs);
				rgbf Lo;
				do { Lo = Shade(IntersectorClosest::Instance().Visit(&bvh, r), -r.dir, true); }
				while (Lo.has_nan());
				img->At<rgbf>(i, j) += Lo / spp;
			}
		}
		float progress = (j + 1) / float(img->height);
		cout << progress << endl;
	}
#endif
}

rgbf PathTracer::Shade(const IntersectorClosest::Rst& intersection, const vecf3& wo, bool last_bounce_specular) const {
	// TODO: HW9 - Trace
	// [ Tips ]
	// - EnvLight::Radiance(<direction>), <direction> is pointing to environment light
	// - AreaLight::Radiance(<uv>)
	// - rayf3: point, dir, tmin, **tmax**
	// - IntersectorVisibility::Instance().Visit(&bvh, <rayf3>)
	//   - tmin = EPSILON<float>
	//   - tmax = distance to light - EPSILON<float>
	// - IntersectorCloest::Instance().Visit(&bvh, <rayf3>)
	//   - tmin as default (EPSILON<float>)
	//   - tmax as default (FLT_MAX)
	//
	// struct IntersectorClosest::Rst {
	//	 bool IsIntersected() const noexcept { return sobj != nullptr; }
	//	 const SObj* sobj{ nullptr }; // intersection sobj
	//	 pointf3 pos; // intersection point's position
	//	 pointf2 uv; // texcoord
	//   normalf n; // normal, normalized
	//	 vecf3 tangent; // perpendicular to normal, normalized
	// };

	constexpr rgbf error_color = rgbf{ 1.f,0.f,1.f };
	constexpr rgbf todo_color = rgbf{ 0.f,1.f,0.f };
	constexpr rgbf zero_color = rgbf{ 0.f,0.f,0.f };

	if (!intersection.IsIntersected()) {
		if (last_bounce_specular && env_light != nullptr) {
			// TODO: environment light
			return env_light->Radiance(wo);
			//return todo_color;
		}
		else
			return zero_color;
	}
	
	if (!intersection.sobj->Get<Cmpt::Material>()) {
		auto light = intersection.sobj->Get<Cmpt::Light>();
		if(!light) return error_color;

		if (last_bounce_specular) { // avoid double-count
			auto area_light = dynamic_cast<const AreaLight*>(light->light.get());
			if (!area_light) return error_color;

			// TODO: area light

			return area_light->RadianceFactor() 
				* area_light->Radiance(intersection.uv);

			//return todo_color;
		}else
			return zero_color;
	}
	
	rgbf L_dir{ 0.f };
	rgbf L_indir{ 0.f };

	scene->Each([=, &L_dir](const Cmpt::Light* light, const Cmpt::L2W* l2w, const Cmpt::SObjPtr* ptr) {
		// TODO: L_dir += ...
		// - use PathTracer::BRDF to get BRDF value
		SampleLightResult sample_light_rst = SampleLight(intersection, wo, light, l2w, ptr);
		if (sample_light_rst.pd <= 0)
			return;
		if (sample_light_rst.is_infinity) {
			// TODO: L_dir of environment light
			// - only use SampleLightResult::L, n, pd
			// - SampleLightResult::x is useless

			// light - surface pos
			vecf3 wi = -sample_light_rst.n.cast_to<vecf3>();

			float cos_theta_xy = wi.cos_theta(intersection.n.cast_to<vecf3>());
			vecf3 G = std::abs(cos_theta_xy) / sample_light_rst.pd;

			rgbf f_r = this->BRDF(intersection, wi, wo);
			
			L_dir += f_r * sample_light_rst.L * G.cast_to<rgbf>();
		}
		else {
			// TODO: L_dir of area light
			
			// light - surface pos
			vecf3 y_x = sample_light_rst.x - intersection.pos;
			float dist_y_x = std::sqrt(y_x.norm2());
			vecf3 wi = y_x.normalize();

			// Check for visibility
			float V = 1;
			bool light_path_intersection = 
				IntersectorVisibility::Instance().Visit(
					&bvh, rayf3(intersection.pos, wi, 2 * EPSILON<float>, dist_y_x - 2 * EPSILON<float>));
			if (!light_path_intersection) {
				V = 0;
			}
			//else if (std::abs((-wi).cos_theta(intersection.n.cast_to<vecf3>())) < 0.5) {
			//	V = 0;
			//}
			else if ((-wi).cos_theta(sample_light_rst.n.cast_to<vecf3>()) < 0) {
				V = 0;
			}

			//else if (wi.cos_theta(intersection.n.cast_to<vecf3>()) < 0.5
			//	&& vecf3{ 0.0f, 1.0f, 0.0f }.cos_theta(intersection.n.cast_to<vecf3>()) < 0.95) {
			//	V = 0;
			//}

			float cos_theta_yx = (-y_x).cos_theta(sample_light_rst.n.cast_to<vecf3>());
			float cos_theta_xy = y_x.cos_theta(intersection.n.cast_to<vecf3>());
			vecf3 G = V * std::abs(cos_theta_xy * cos_theta_yx) / y_x.norm2() * sample_light_rst.pd;

			rgbf f_r = this->BRDF(intersection, wi, wo);
			
			L_dir += f_r * sample_light_rst.L * G.cast_to<rgbf>();
		}
	});

	// TODO: Russian Roulette
	// - rand01<float>() : random in [0, 1)
	// **TODO: adjust**
	float pdf_thres = 0.5;
	if (rand01<float>() > pdf_thres) {
		return L_dir;
	}

	// wi, pdf
	std::tuple<vecf3, float> brdf_res = SampleBRDF(intersection, wo);

	auto wi = std::get<0>(brdf_res);

	//static int counter_1 = 0;
	//static int counter_2 = 0;
	if (wi.cos_theta(intersection.n.cast_to<vecf3>()) < 0) {
		//counter_1++;
		return L_dir;
	}
	else {
		//counter_2++;

	}
	//printf("C1: %d, C2: %d\n", counter_1, counter_2);

	// emit a brand new ray
	auto higher_order_intersection =
		IntersectorClosest::Instance().Visit(
			&bvh, rayf3(intersection.pos, wi, EPSILON<float>));

	rgbf higher_order_L_r;

	higher_order_L_r = Shade(higher_order_intersection, -wi, false);

	//auto n_p' = higher_order_intersection.n;
	//auto higher_order_pos = higher_order_intersection.pos;

	float cos_theta_wi_np = (wi).cos_theta(intersection.n.cast_to<vecf3>());
	rgbf f_r = this->BRDF(intersection, wi, wo);

	L_indir = f_r * higher_order_L_r * cos_theta_wi_np / std::get<1>(brdf_res) / pdf_thres;

	// TODO: recursion
	// - use PathTracer::SampleBRDF to get wi and pd (probability density)
	// wi may be **under** the surface
	// - use PathTracer::BRDF to get BRDF value

	// TODO: combine L_dir and L_indir
	return L_dir + L_indir;

	return todo_color; // you should commemt this line
}

PathTracer::SampleLightResult PathTracer::SampleLight(IntersectorClosest::Rst intersection, const vecf3& wo, const Cmpt::Light* light, const Cmpt::L2W* l2w, const Cmpt::SObjPtr* ptr) const {
	PathTracer::SampleLightResult rst;
	if (vtable_is<AreaLight>(light->light.get())) {
		auto area_light = static_cast<const AreaLight*>(light->light.get());
		auto geo = ptr->value->Get<Cmpt::Geometry>();
		if (!geo) return rst; // invalid
		if (!vtable_is<Square>(geo->primitive.get())) return rst; // not support

		auto Xi = uniform_in_square<float>(); // [0, 1] x [0, 1]
		pointf3 pos_light_space{ 2 * Xi[0] - 1, 0, 2 * Xi[1] - 1 };
		scalef3 s = l2w->WorldScale();
		float area = s[0] * s[1] * Square::area;

		rst.L = area_light->Radiance(Xi.cast_to<pointf2>());
		rst.pd = 1.f / area;
		rst.x = l2w->value * pos_light_space;
		rst.is_infinity = false;
		rst.n = l2w->UpInWorld().cast_to<normalf>();
	}
	else if (vtable_is<EnvLight>(light->light.get())) {
		rst.is_infinity = true;

		auto mat = intersection.sobj->Get<Cmpt::Material>();
		if (!mat) return rst; // invalid
		auto brdf = dynamic_cast<const stdBRDF*>(mat->material.get());
		if (!brdf) return rst; // not support

		// multi-importance sampling, MIS

		auto env_light = static_cast<const EnvLight*>(light->light.get());

		float metalness = brdf->Metalness(intersection.uv);
		float roughness = brdf->Roughness(intersection.uv);
		float lambda = metalness * (1 - stdBRDF::Alpha(roughness)); // 0 - 1
		float p_mat = 1 / (2 - lambda); // 0.5 - 1
		float pd_mat, pd_env;
		vecf3 wi;
		rgbf Le;

		if (rand01<float>() < p_mat) {
			tie(wi, pd_mat) = SampleBRDF(intersection, wo);
			Le = env_light->Radiance(wi);
			pd_env = env_light->PDF(wi, intersection.n); // TODO: use your PDF
			//pd_env = 

		}
		else {
			tie(Le, wi, pd_env) = env_light->Sample(intersection.n); // TODO: use your sampling method

			//// use my sampling

			matf3 surface_to_world = svecf::TBN(intersection.n.cast_to<vecf3>(), intersection.tangent);
			matf3 world_to_surface = surface_to_world.inverse();
			svecf s_wo = (world_to_surface * wo).cast_to<svecf>();
			svecf s_wi = (world_to_surface * wi).cast_to<svecf>();
			rgbf albedo = brdf->Albedo(intersection.uv);
			pd_mat = brdf->PDF(albedo, metalness, roughness, s_wi, s_wo);
		}

		rst.L = Le;
		rst.n = -wi.cast_to<normalf>();
		rst.pd = p_mat * pd_mat + (1 - p_mat) * pd_env;
		rst.x = pointf3{ std::numeric_limits<float>::max() };
	}
	return rst;
}

std::tuple<vecf3, float> PathTracer::SampleBRDF(IntersectorClosest::Rst intersection, const vecf3& wo) {
	auto mat = intersection.sobj->Get<Cmpt::Material>();
	if (!mat) return { vecf3{0.f}, 0.f };
	auto brdf = dynamic_cast<const stdBRDF*>(mat->material.get());
	if (!brdf) return { vecf3{0.f}, 0.f };

	matf3 surface_to_world = svecf::TBN(intersection.n.cast_to<vecf3>(), intersection.tangent);
	matf3 world_to_surface = surface_to_world.inverse();
	svecf s_wo = (world_to_surface * wo).cast_to<svecf>();

	rgbf albedo = brdf->Albedo(intersection.uv);
	float metalness = brdf->Metalness(intersection.uv);
	float roughness = brdf->Roughness(intersection.uv);

	auto [s_wi, pdf] = brdf->Sample(albedo, metalness, roughness, s_wo);
	vecf3 wi = surface_to_world * s_wi;

	return { wi,pdf };
}

rgbf PathTracer::BRDF(IntersectorClosest::Rst intersection, const vecf3& wi, const vecf3& wo) {
	auto mat = intersection.sobj->Get<Cmpt::Material>();
	if (!mat) return rgbf{ 1.f,0.f,1.f };
	auto brdf = dynamic_cast<const stdBRDF*>(mat->material.get());
	if (!brdf) return rgbf{ 1.f,0.f,1.f };

	matf3 surface_to_world = svecf::TBN(intersection.n.cast_to<vecf3>(), intersection.tangent);
	matf3 world_to_surface = surface_to_world.inverse();
	svecf s_wi = (world_to_surface * wi).cast_to<svecf>();
	svecf s_wo = (world_to_surface * wo).cast_to<svecf>();

	rgbf albedo = brdf->Albedo(intersection.uv);
	float metalness = brdf->Metalness(intersection.uv);
	float roughness = brdf->Roughness(intersection.uv);

	return brdf->BRDF(albedo, metalness, roughness, s_wi, s_wo);
}
