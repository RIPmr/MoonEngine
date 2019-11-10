#pragma once
#include "Ray.h"
#include "MathUtils.h"
#include "MShader.h"
#include "Hitable.h"
#include "Texture.h"
#include "ObjectBase.h"

namespace moon {
	enum MatType {
		moonMtl,
		lambertian,
		metal,
		dielectric,
		light
	};

	class Material : public ObjectBase {
	public:
		Shader* shader;

		//Material()
		~Material() { delete shader; }

		virtual bool scatter(const Ray &r_in, const HitRecord &rec, Vector3 &attenuation, Ray &scattered) const = 0;
	};

	class LightMtl : public Material {
	public:
		virtual bool scatter(const Ray &r_in, const HitRecord &rec, Vector3 &attenuation, Ray &scattered) const = 0;
	};

	class MoonMtl : public Material {
	public:
		Vector3 Ka;		// Ambient Color
		Vector3 Kd;		// Diffuse Color
		Vector3 Ks;		// Specular Color
		float Ns;		// Specular Exponent
		float Ni;		// Optical Density
		float d;		// Dissolve
		int illum;		// Illumination

		Texture* map_Ka;		// Ambient Texture Map
		Texture* map_Kd;		// Diffuse Texture Map
		Texture* map_Ks;		// Specular Texture Map
		Texture* map_Ns;		// Specular Hightlight Map
		Texture* map_d;		// Alpha Texture Map
		Texture* map_bump;	// Bump Map

		MoonMtl(): Ns(0.0f), Ni(0.0f), d(0.0f), illum(0) {}
		virtual bool scatter(const Ray &r_in, const HitRecord &rec, Vector3 &attenuation, Ray &scattered) const;
	};

	class Lambertian : public Material {
	public:
		Vector3 albedo;

		Lambertian(const Vector3 &albedo) : albedo(albedo) {}
		virtual bool scatter(const Ray &r_in, const HitRecord &rec, Vector3 &attenuation, Ray &scattered) const;
	};

	class Metal : public Material {
	public:
		Vector3 albedo;
		float fuzz;

		Metal(const Vector3 &albedo, float fuzz) : albedo(albedo) {
			if (fuzz < 1) this->fuzz = fuzz;
			else this->fuzz = 1;
		}
		virtual bool scatter(const Ray &r_in, const HitRecord &rec, Vector3 &attenuation, Ray &scattered) const;
	};

	class Dielectric : public Material {
	public:
		float ref_idx;

		Dielectric(float ri) : ref_idx(ri) {}
		virtual bool scatter(const Ray &r_in, const HitRecord &rec, Vector3 &attenuation, Ray &scattered) const;
	};
}