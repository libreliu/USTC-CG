#pragma once

#include <Engine/Light/Light.h>

namespace Ubpa {
	class SphereLight : public Light {
	public:
		SphereLight(const rgbf& color = 1.f, float intensity = 1.f, float radius = 1.f)
			: color(color), intensity(intensity), radius(radius) { }

	public:
		static Ptr<SphereLight> New(const rgbf& color = 1.f, float intensity = 1.f, float radius = 1.f) {
			return Ubpa::New<SphereLight>(color, intensity, radius);
		}

	protected:
		virtual ~SphereLight() = default;

	public:
		float Area() const { return 4 * PI<float> * radius * radius; }
		const rgbf LuminancePower() const { return intensity * color; }
		const rgbf Luminance() const { return LuminancePower() / (Area() * PI<float>); }

	public:
		// ���� L ����
		// !!! p��wi ���ڹ�Դ������ռ���
		// @arg0  in���� p �������� distToLight �� PD
		// @arg1 out��wi ָ���Դ��Ϊ��λ����
		// @arg2 out��p �㵽��Դ������ľ���
		// @arg3 out�������ܶ� probability density
		virtual const rgbf Sample_L(const pointf3& p, normalf& wi, float& distToLight, float& PD) const override;

		// �����ܶȺ���
		// !!! p��wi ���ڵƵ�����ռ���
		virtual float PDF(const pointf3& p, const normalf& wi) const override;

		virtual bool IsDelta() const override { return false; }

	public:
		rgbf color;
		float intensity;
		float radius;
	};
}