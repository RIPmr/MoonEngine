#include "Material.h"
#include "SceneMgr.h"
#include "Renderer.h"

#define HAVE_STRUCT_TIMESPEC
#include <pthread.h>
#pragma comment(lib, "pthreadVC2.lib")

namespace MOON {
	Vector2 Material::PREVSIZE = Vector2(124, 124);

	void Material::ListPreview() {
		ImGui::Text("Preview: ");
		if (prevNeedUpdate && !MOON_InputManager::mouse_left_hold) {
			GeneratePreview();
			UpdatePreview();
		}

		float centering = (ImGui::GetContentRegionAvailWidth() - preview->width) / 2.0f;
		ImGui::Indent(centering);
		ImGui::Image((void*)(intptr_t)preview->localID, ImVec2(preview->width, preview->height));
		ImGui::Unindent(centering);
	}

	void Material::UpdatePreview() {
		if (Renderer::progress) {
			//glBindTexture(GL_TEXTURE_2D, preview->localID);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, Material::PREVSIZE.x, Material::PREVSIZE.y, 0,
				GL_RGB, GL_UNSIGNED_BYTE, Renderer::matPrevImage);
		}
		if (Renderer::progress < 0) {
			Renderer::progress = 0;
			Renderer::prevInQueue = false;
			prevNeedUpdate = false;
			glBindTexture(GL_TEXTURE_2D, 0);
		}
	}

	void Material::GeneratePreview() {
		if (preview == MOON_UNSPECIFIEDID)
			preview = new Texture(Material::PREVSIZE.x, Material::PREVSIZE.y, "preview_for_" + name, MOON_UNSPECIFIEDID);

		if (Renderer::prevInQueue) return;
		else Renderer::prevInQueue = true;

		if (Renderer::PrepareMatPrevRendering(preview)) {
			pthread_t prevThread;
			int ret = pthread_create(&prevThread, NULL, Renderer::renderingMatPreview, this);
			if (!ret) std::cout << "renderer thread created!" << std::endl;
			else std::cout << "renderer thread error! pthread_create error: error_code=" << ret << std::endl;
		}
	}

	MoonMtl::MoonMtl() : Ns(0.0f), Ni(0.0f), d(0.0f), illum(0) {
		Kd.setValue(0.8, 0.8, 0.8);
		shader = MOON_ShaderManager::CreateShader("SimplePhong", "SimplePhong.vs", "SimplePhong.fs");
	}

	MoonMtl::MoonMtl(const std::string &name) : Ns(0.0f), Ni(0.0f), d(0.0f), illum(0), Material(name) {
		Kd.setValue(0.8, 0.8, 0.8);
		shader = MOON_ShaderManager::CreateShader("SimplePhong", "SimplePhong.vs", "SimplePhong.fs");
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