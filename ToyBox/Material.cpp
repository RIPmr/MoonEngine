#include "Material.h"
#include "SceneMgr.h"

namespace moon {
	MoonMtl::MoonMtl() : Ns(0.0f), Ni(0.0f), d(0.0f), illum(0) {
		Kd.setValue(0.8, 0.8, 0.8);
		shader = SceneManager::ShaderManager::CreateShader("SimplePhong", "SimplePhong.vs", "SimplePhong.fs");
	}

	MoonMtl::MoonMtl(const std::string &name) : Ns(0.0f), Ni(0.0f), d(0.0f), illum(0), Material(name) {
		Kd.setValue(0.8, 0.8, 0.8);
		shader = SceneManager::ShaderManager::CreateShader("SimplePhong", "SimplePhong.vs", "SimplePhong.fs");
	}

	// TODO
	bool MoonMtl::scatter(const Ray &r_in, const HitRecord &rec, Vector3 &attenuation, Ray &scattered) const {
		Vector3 target = rec.p + rec.normal + MoonMath::RandomInUnitSphere();
		scattered = Ray(rec.p, target - rec.p);
		attenuation = Kd;

		return true;
	}

	bool Lambertian::scatter(const Ray &r_in, const HitRecord &rec, Vector3 &attenuation, Ray &scattered) const {
		Vector3 target = rec.p + rec.normal + MoonMath::RandomInUnitSphere();
		scattered = Ray(rec.p, target - rec.p);
		attenuation = albedo;

		return true;
	}

	bool Metal::scatter(const Ray &r_in, const HitRecord &rec, Vector3 &attenuation, Ray &scattered) const {
		Vector3 normDir = Vector3::Normalize(r_in.dir);
		Vector3 reflected = MoonMath::Reflect(normDir, rec.normal);
		scattered = Ray(rec.p, reflected + fuzz * MoonMath::RandomInUnitSphere());
		attenuation = albedo;

		return (scattered.dir.dot(rec.normal) > 0);
	}

	bool Dielectric::scatter(const Ray &r_in, const HitRecord &rec, Vector3 &attenuation, Ray &scattered) const {
		Vector3 outward_normal;
		Vector3 reflected = MoonMath::Reflect(r_in.dir, rec.normal);
		float ni_over_nt;
		attenuation = Vector3(1.0, 1.0, 1.0);
		Vector3 refracted;
		float reflect_prob;
		float cosine;

		if (r_in.dir.dot(rec.normal) > 0) {
			outward_normal = -1 * rec.normal;
			ni_over_nt = ref_idx;
			cosine = ref_idx * r_in.dir.dot(rec.normal) / r_in.dir.magnitude();
		} else {
			outward_normal = rec.normal;
			ni_over_nt = 1.0 / ref_idx;
			cosine = -r_in.dir.dot(rec.normal) / r_in.dir.magnitude();
		}

		if (MoonMath::Refract(r_in.dir, outward_normal, ni_over_nt, refracted)) {
			reflect_prob = MoonMath::Schlick(cosine, ref_idx);
		} else {
			scattered = Ray(rec.p, reflected);
			reflect_prob = 1.0;
		}

		if (MoonMath::drand48() < reflect_prob) {
			scattered = Ray(rec.p, reflected);
		} else scattered = Ray(rec.p, refracted);

		return true;
	}
}