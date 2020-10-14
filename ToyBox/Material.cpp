#include "Material.h"
#include "SceneMgr.h"
#include "Renderer.h"
#include "ThreadPool.h"
#include "Utility.h"

namespace MOON {
	Vector2 Material::PREVSIZE = Vector2(124, 124);

	#pragma region material
	void Material::ListShader() {
		if (ImGui::TreeNode("Shader", ID)) {
			auto width = ImGui::GetContentRegionAvailWidth();
			ImGui::Text(Icon_Name_To_ID(ICON_FA_FILE_CODE_O, " " + shader->name));
			ImGui::SameLine(width - 22.0f);
			ImGui::PushID(shader->ID);
			if (ImGui::SmallButton(ICON_FA_CROSSHAIRS))
				MOON_InputManager::Select_Append(shader->ID);
			ImGui::PopID();
			ImGui::TreePop();
		}
	}

	void Material::ListPreview() {
		ImGui::Text("Preview: ");

		if (prevNeedUpdate) {
			GeneratePreview();
			UpdatePreview();
		}

		float centering = (ImGui::GetContentRegionAvailWidth() - preview->width) / 2.0f;
		ImGui::Indent(centering);
		if (ImGui::Button("Update", ImVec2(preview->width, 0), this->ID)) prevNeedUpdate = true;
		ImGui::Image((void*)(intptr_t)preview->localID, ImVec2(preview->width, preview->height));
		ImGui::Unindent(centering);
	}

	void Material::UpdatePreview() {
		if (MOON_InputManager::mouse_left_hold) return;

		if (Renderer::progress) {
			glBindTexture(GL_TEXTURE_2D, preview->localID);
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
		
		if (MOON_InputManager::mouse_left_hold) return;
		if (Renderer::prevInQueue) return;
		else Renderer::prevInQueue = true;

		if (Renderer::PrepareMatPrevRendering(preview))
			ThreadPool::CreateThread(Renderer::renderingMatPreview, this);
	}
	#pragma endregion

	bool RefractScatter(const float& ref_idx, const Ray &r_in, const HitRecord &rec, Vector3 &attenuation, Ray &scattered) {
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

	bool LambertScatter(const Vector3& diffuseC, const Ray &r_in, const HitRecord &rec, Vector3 &attenuation, Ray &scattered) {
		Vector3 target = rec.p + rec.normal + MoonMath::RandomInUnitSphere();
		scattered = Ray(rec.p, target - rec.p);
		attenuation = diffuseC;

		return true;
	}
	
	bool ReflectScatter(const Vector3& diffuseC, const float& glossiness, const Ray &r_in, const HitRecord &rec, Vector3 &attenuation, Ray &scattered) {
		Vector3 normDir = Vector3::Normalize(r_in.dir);
		Vector3 reflected = MoonMath::Reflect(normDir, rec.normal);
		scattered = Ray(rec.p, reflected + glossiness * MoonMath::RandomInUnitSphere());
		attenuation = diffuseC;

		return (scattered.dir.dot(rec.normal) > 0);
	}

	#pragma region moonmtl
	MoonMtl::MoonMtl() {
		ambientC.setValue(0.8f, 0.8f, 0.8f); diffuseC.setValue(0.8f, 0.8f, 0.8f);
		reflectW.setValue(0.0f, 0.0f, 0.0f); refractW.setValue(0.0f, 0.0f, 0.0f);
		translucency.setValue(1.0f, 1.0f, 1.0f); opacity.setValue(1.0f, 1.0f, 1.0f);
		fogC.setValue(1.0f, 1.0f, 1.0f); illumination.setValue(0.0f, 0.0f, 0.0f); 
		metalness.setValue(0.0f, 0.0f, 0.0f);

		roughness = 0.0f; glossiness = 0.0f; IOR = 1.33f; fogW = 0.0f;
		dispW = 1.0f; anisotropy = 0.0f; an_rotation = 0.0f;

		//shader = MOON_ShaderManager::CreateShader("BlinnPhong", "SimplePhong.vs", "BlinnPhong.fs");
		shader = MOON_ShaderManager::CreateShader("PBR", "PBR.vs", "PBR.fs");
	}

	MoonMtl::MoonMtl(const std::string &name) : Material(name) {
		ambientC.setValue(0.8f, 0.8f, 0.8f); diffuseC.setValue(0.8f, 0.8f, 0.8f);
		reflectW.setValue(1.0f, 1.0f, 1.0f); refractW.setValue(1.0f, 1.0f, 1.0f);
		translucency.setValue(1.0f, 1.0f, 1.0f); opacity.setValue(1.0f, 1.0f, 1.0f);
		fogC.setValue(1.0f, 1.0f, 1.0f); illumination.setValue(0.0f, 0.0f, 0.0f);
		metalness.setValue(0.0f, 0.0f, 0.0f);

		roughness = 0.0f; glossiness = 0.0f; IOR = 1.33f; fogW = 0.0f;
		dispW = 1.0f; anisotropy = 0.0f; an_rotation = 0.0f;

		//shader = MOON_ShaderManager::CreateShader("BlinnPhong", "SimplePhong.vs", "BlinnPhong.fs");
		shader = MOON_ShaderManager::CreateShader("PBR", "PBR.vs", "PBR.fs");
	}

	MoonMtl::~MoonMtl() {
		//Utility::ReleaseVector(textures);
	}

	// TODO
	bool MoonMtl::scatter(const Ray &r_in, const HitRecord &rec, Vector3 &attenuation, Ray &scattered) const {
		if (name._Equal("default")) {
			return LambertScatter(Vector3(0.8f, 0.8f, 0.8f), r_in, rec, attenuation, scattered);
		}
		
		Vector3 lamb_att(attenuation), ref_att(attenuation);
		Ray lamb_sca(scattered), refr_sca(scattered);

		float refractProb = std::sqrtf(refractW.x);

		RefractScatter(IOR, r_in, rec, ref_att, refr_sca);
		LambertScatter(diffuseC, r_in, rec, lamb_att, lamb_sca);

		//attenuation = lamb_att;
		//attenuation = Vector3::Lerp(lamb_att, ref_att, refractProb);
		attenuation = lamb_att + (ref_att - lamb_att) * refractProb;

		Vector3 reflected = MoonMath::Reflect(Vector3::Normalize(r_in.dir), rec.normal);
		auto refl_sca = Ray(rec.p, reflected + glossiness * MoonMath::RandomInUnitSphere());

		auto rand = MoonMath::drand48();
		if (rand < reflectW.x) scattered = refl_sca;
		else scattered = lamb_sca;

		if (rand < refractProb) scattered = refr_sca;
		//else scattered = lamb_sca;

		if (scattered.dir == refl_sca.dir) return (scattered.dir.dot(rec.normal) > 0);
		else return true;
	}
	#pragma endregion

	#pragma region simple_lambertian
	bool Lambertian::scatter(const Ray &r_in, const HitRecord &rec, Vector3 &attenuation, Ray &scattered) const {
		Vector3 target = rec.p + rec.normal + MoonMath::RandomInUnitSphere();
		scattered = Ray(rec.p, target - rec.p);
		attenuation = albedo;

		return true;
	}
	#pragma endregion

	#pragma region metal
	bool Metal::scatter(const Ray &r_in, const HitRecord &rec, Vector3 &attenuation, Ray &scattered) const {
		Vector3 normDir = Vector3::Normalize(r_in.dir);
		Vector3 reflected = MoonMath::Reflect(normDir, rec.normal);
		scattered = Ray(rec.p, reflected + fuzz * MoonMath::RandomInUnitSphere());
		attenuation = albedo;

		return (scattered.dir.dot(rec.normal) > 0);
	}
	#pragma endregion

	#pragma region dielectric
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
	#pragma endregion

	#pragma region lightmtl
	LightMtl::LightMtl() {}
	LightMtl::LightMtl(const std::string &name) : Material(name) {}

	bool LightMtl::scatter(const Ray &r_in, const HitRecord &rec, Vector3 &attenuation, Ray &scattered) const {

		return true;
	}
	#pragma endregion

	#pragma region SEM
	SEM::SEM() {
		matcap = MOON_TextureManager::LoadTexture("./Assets/Textures/MatCap/matcap.jpg");
		shader = MOON_ShaderManager::CreateShader("SEM", "SEM.vs", "SEM.fs");
	}
	SEM::SEM(const std::string &name) : Material(name) {
		matcap = MOON_TextureManager::LoadTexture("./Assets/Textures/MatCap/matcap.jpg");
		shader = MOON_ShaderManager::CreateShader("SEM", "SEM.vs", "SEM.fs");
	}

	bool SEM::scatter(const Ray &r_in, const HitRecord &rec, Vector3 &attenuation, Ray &scattered) const {

		return true;
	}
	#pragma endregion

}