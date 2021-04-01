#include "Material.h"
#include "SceneMgr.h"
#include "Renderer.h"
#include "ThreadPool.h"

namespace MOON {
#pragma region params
	Vector2 Material::PREVSIZE		= Vector2(124, 124);
	bool Material::autoPrevUpdate	= true;
#pragma endregion

#pragma region material
	void Material::SetShaderProps(Shader* shader) {
		if (shader == nullptr) {
			shader = MOON_ShaderManager::CreateShader("BlinnPhong", "SimplePhong.vs", "BlinnPhong.fs");
		}

		shader->setFloat("inputGamma", 2.2f);

		shader->setTexture("irradianceMap", MOON_TextureManager::Irradiance, 0);
		shader->setTexture("prefilterMap", MOON_TextureManager::prefilterMap, 1);
		shader->setTexture("brdfLUT", MOON_TextureManager::brdfLUT, 2);

		// legacy shader compatible
		shader->setVec3("objectColor", Vector3(0.8f, 0.8f, 0.8f));
		shader->setFloat("specularLV", 0.8f * 1024);
	}

	void Material::ListShader() {
		if (shader == nullptr) {
			shader = MOON_ShaderManager::CreateShader("BlinnPhong", "SimplePhong.vs", "BlinnPhong.fs");
		}

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
		if (ImGui::Button("Update", ImVec2(preview->width, 0), this->ID)) {
			GeneratePreview(true);
			prevNeedUpdate = true;
		}
		ImGui::Image((void*)(intptr_t)preview->localID, ImVec2(preview->width, preview->height));
		ImGui::Unindent(centering);
	}

	void Material::UpdatePreview() {
		if (MOON_InputManager::mouse_left_hold) return;

		if (Renderer::progress) {
			glBindTexture(GL_TEXTURE_2D, preview->localID);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, Material::PREVSIZE.x, Material::PREVSIZE.y, 0,
				GL_RGB, GL_FLOAT, Renderer::matPrevRaw);
		}
		if (Renderer::progress < 0) {
			Renderer::progress = 0;
			Renderer::prevInQueue = false;
			prevNeedUpdate = false;
			glBindTexture(GL_TEXTURE_2D, 0);
		}
	}

	void Material::GeneratePreview(bool manualUpdate) {
		if (preview == MOON_UNSPECIFIEDID)
			preview = new Texture(Material::PREVSIZE.x, Material::PREVSIZE.y, "preview_for_" + name, MOON_UNSPECIFIEDID);
		
		if (!autoPrevUpdate && !manualUpdate) return;
		if (MOON_InputManager::mouse_left_hold) return;
		if (Renderer::prevInQueue) return;
		else Renderer::prevInQueue = true;

		if (Renderer::PrepareMatPrevRendering(preview)) {
			ThreadPool::CreateThread(Renderer::renderingMatPreview, this);
		}
	}
#pragma endregion

#pragma region scatter_functions
	bool RefractScatter(const float& ref_idx, const float& glossiness, const Ray &r_in, const HitRecord &rec, Vector3 &attenuation, Ray &scattered) {
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

		if (MoonMath::drand48() < reflect_prob) scattered = Ray(rec.p, reflected);
		else scattered = Ray(rec.p, refracted + glossiness * MoonMath::RandomInUnitSphere());

		return true;
	}

	bool LambertScatter(const Vector3& diffuseC, const float& roughness, const Ray &r_in, const HitRecord &rec, Vector3 &attenuation, Ray &scattered) {
		Vector3 target = rec.p + rec.normal + roughness * MoonMath::RandomInUnitSphere();
		scattered = Ray(rec.p, target - rec.p);
		attenuation = diffuseC;

		return true;
	}

	bool MetalReflectScatter(const Vector3& diffuseC, const float& glossiness, const Ray &r_in, const HitRecord &rec, Vector3 &attenuation, Ray &scattered) {
		Vector3 normDir = Vector3::Normalize(r_in.dir);
		Vector3 reflected = MoonMath::Reflect(normDir, rec.normal);

		scattered = Ray(rec.p, reflected + glossiness * MoonMath::RandomInUnitSphere());
		attenuation = diffuseC;

		return scattered.dir.dot(rec.normal) > 0;
	}

	bool ReflectScatter(const Vector3& diffuseC, const float& glossiness, const float& metalness, const Ray &r_in, const HitRecord &rec, Vector3 &attenuation, Ray &scattered) {
		Vector3 normDir = Vector3::Normalize(r_in.dir);
		Vector3 reflected = MoonMath::Reflect(normDir, rec.normal);

		Vector3 Fdielectric(0.04f, 0.04f, 0.04f);
		Vector3 F0 = MoonMath::lerp(Fdielectric, diffuseC, metalness);
		// Angle between surface normal and outgoing light direction
		float cosLo = std::max(Vector3::Normalize(rec.normal).dot(-normDir), 0.0f);
		Vector3 F = MoonMath::FresnelSchlick(F0, cosLo);
		// Get diffuse contribution factor
		Vector3 kd = MoonMath::lerp(Vector3::ONE() - F, Vector3::ZERO(), metalness);

		// Ray(rec.p, -rec.normal): a decent approximate to get the albedo color of object itself
		if (MoonMath::drand48() < kd.x) scattered = Ray(rec.p, -rec.normal);
		else scattered = Ray(rec.p, reflected + glossiness * MoonMath::RandomInUnitSphere());

		attenuation = diffuseC;

		return scattered.dir.dot(rec.normal) > 0;
	}

	bool UniversalScatter(const Vector3& diffuseC, const float& roughness, 
		const float& reflectW, const float& refractW, const float& glossiness, 
		const float& metalness, const float& IOR, const Ray &r_in, const HitRecord &rec,
		Vector3 &attenuation, Ray &scattered) {
		Vector3 lamb_att(attenuation), refr_att(attenuation), refl_att(attenuation);
		Ray lamb_sca(scattered), refr_sca(scattered), refl_sca(scattered);

		LambertScatter(diffuseC, roughness, r_in, rec, lamb_att, lamb_sca);

		auto rand = MoonMath::drand48();
		if (reflectW > 0.001f) {
			ReflectScatter(diffuseC, glossiness, metalness, r_in, rec, refl_att, refl_sca);
			if (refl_sca.dir.dot(rec.normal) > 0 && rand <= reflectW) scattered = refl_sca;
			else scattered = lamb_sca;
		} else scattered = lamb_sca;

		float refractProb = std::sqrtf(refractW);
		if (refractProb > 0.001f) {
			RefractScatter(IOR, glossiness, r_in, rec, refr_att, refr_sca);
			if (rand <= refractProb) {
				attenuation = lamb_att + (refr_att - lamb_att) * refractProb;
				scattered = refr_sca;
			} else attenuation = lamb_att;
		} else attenuation = lamb_att;

		return true;
	}
#pragma endregion

#pragma region scatter_functions_IS
	bool LambertScatter_IS(const Vector3& diffuseC, const float& roughness, const Ray &r_in, const HitRecord &rec, Vector3 &albedo, Ray &scattered, float& pdf) {
		Vector3 target = rec.p + rec.normal + roughness * MoonMath::RandomInUnitSphere();
		scattered = Ray(rec.p, Vector3::Normalize(target - rec.p));
		albedo = diffuseC;

		pdf = rec.normal.dot(scattered.dir) / PI;
		return true;
	}
#pragma endregion

#pragma region moonmtl
	MoonMtl::MoonMtl() {
		DefineTextures({
			"ambient", "albedo", "reflect", "refract", "roughness",
			"normal", "displace", "metallic", "translucent", "illumina", "alpha"
		});

		diffuseC.setValue(0.8f, 0.8f, 0.8f); reflectW.setValue(1.0f, 1.0f, 1.0f); 
		refractW.setValue(0.0f, 0.0f, 0.0f); translucency.setValue(1.0f, 1.0f, 1.0f); 
		opacity.setValue(1.0f, 1.0f, 1.0f); fogC.setValue(1.0f, 1.0f, 1.0f); 
		illumination.setValue(0.0f, 0.0f, 0.0f); metalness.setValue(0.0f, 0.0f, 0.0f); 
		glossiness.setValue(0.8f, 0.8f, 0.8f);

		roughness = 1.0f; illumMulti = 1.0f; IOR = 1.33f; fogW = 0.0f;
		dispW = 1.0f; anisotropy = 0.0f; an_rotation = 0.0f; fresnel = 5.0f;
		ambientW = 0.0f; normalW = 1.0f;

		shader = MOON_ShaderManager::CreateShader("PBR", "PBR.vs", "PBR_Ref.fs");
	}

	MoonMtl::MoonMtl(const std::string &name) : Material(name) {
		DefineTextures({
			"ambient", "albedo", "reflect", "refract", "roughness",
			"normal", "displace", "metallic", "translucent", "illumina", "alpha"
		});

		diffuseC.setValue(0.8f, 0.8f, 0.8f); reflectW.setValue(1.0f, 1.0f, 1.0f); 
		refractW.setValue(0.0f, 0.0f, 0.0f); translucency.setValue(1.0f, 1.0f, 1.0f); 
		opacity.setValue(1.0f, 1.0f, 1.0f); fogC.setValue(1.0f, 1.0f, 1.0f); 
		illumination.setValue(0.0f, 0.0f, 0.0f); metalness.setValue(0.0f, 0.0f, 0.0f); 
		glossiness.setValue(0.8f, 0.8f, 0.8f);

		roughness = 1.0f; illumMulti = 1.0f; IOR = 1.33f; fogW = 0.0f;
		dispW = 1.0f; anisotropy = 0.0f; an_rotation = 0.0f; fresnel = 5.0f;
		ambientW = 0.0f; normalW = 1.0f;

		shader = MOON_ShaderManager::CreateShader("PBR", "PBR.vs", "PBR_Ref.fs");
	}

	void MoonMtl::SetShaderProps(Shader* shader) {
		Material::SetShaderProps(shader);

		// legacy shader compatible ---------------------------------------------
		shader->setVec3 ("objectColor",		diffuseC);
		shader->setFloat("specularLV",		glossiness.x * 1024);

		// colors ---------------------------------------------------------------
		shader->setVec3 ("_albedo",			diffuseC);
		shader->setVec3 ("_illumination",	illumination);
		shader->setVec3 ("_translucency",	translucency);
		shader->setVec3 ("_alpha",			opacity);
		shader->setVec3 ("_fogC",			fogC);

		// other props ----------------------------------------------------------
		shader->setFloat("_reflectance",	reflectW.x);
		shader->setFloat("_refraction",		refractW.x);
		shader->setFloat("_illumMulti",		illumMulti);
		shader->setFloat("_metalness",		metalness.x);
		shader->setFloat("_roughness",		glossiness.x);
		shader->setFloat("_IOR",			3.0f - IOR);
		shader->setFloat("_fresnel",		fresnel);
		shader->setFloat("_fogW",			fogW);
		shader->setFloat("_dispW",			dispW);
		shader->setFloat("_aoW",			ambientW);
		shader->setFloat("_nrmW",			normalW);
		//shader->setFloat("_anisotropy",	anisotropy);
		//shader->setFloat("_an_rotation", an_rotation);

		// textures -------------------------------------------------------------
		for (int i = 0; i < textures.size(); i++) {
			shader->setBool(texTypeList[i] + "Switch", textures[i] != nullptr);
			if (textures[i] != nullptr) shader->setTexture(texTypeList[i] + "Map", textures[i], i + 3);
		}
	}

	bool MoonMtl::PDF(const Ray& r_in, const HitRecord& rec, const Ray& scattered) const {
		float cosine = rec.normal.dot(Vector3::Normalize(scattered.dir));
		cosine = std::max(cosine, 0.0f);
		return cosine / PI;
	}

	bool MoonMtl::Scatter_IS(const Ray &r_in, const HitRecord &rec, Vector3 &attenuation, Ray &scattered, float& pdf) const {
		//if (this == MOON_MaterialManager::defaultMat) {
			return LambertScatter_IS(
				diffuseC, glossiness.x,
				r_in, rec, attenuation, scattered, pdf
			);
		//}
		return true;
	}

	Vector3 MoonMtl::Emitted(const Vector2& uv) const {
		return illumination * illumMulti;
	}

	bool MoonMtl::Scatter(const Ray &r_in, const HitRecord &rec, Vector3 &attenuation, Ray &scattered) const {
		if (this == MOON_MaterialManager::defaultMat) {
			return LambertScatter(
				diffuseC, glossiness.x, 
				r_in, rec, attenuation, scattered
			);
		}

		return UniversalScatter(
			diffuseC, roughness, reflectW.x, refractW.x, glossiness.x, metalness.x, IOR, 
			r_in, rec, attenuation, scattered
		);
	}
#pragma endregion

#pragma region lightmtl
	LightMtl::LightMtl() : color(Color::WHITE()), power(1.0f) {}
	LightMtl::LightMtl(const std::string &name) : Material(name), color(Color::WHITE()), power(1.0f) {}

	void LightMtl::SetShaderProps(Shader* shader) {
		Material::SetShaderProps(shader);

		shader->setVec3("illumination", color * power);
	}

	Vector3 LightMtl::Emitted(const Vector2& uv) const {
		return color * power;
	}

	bool LightMtl::Scatter(const Ray &r_in, const HitRecord &rec, Vector3 &attenuation, Ray &scattered) const {
		return false;
	}
#pragma endregion

#pragma region simple_lambertian
	bool Lambertian::Scatter(const Ray &r_in, const HitRecord &rec, Vector3 &attenuation, Ray &scattered) const {
		Vector3 target = rec.p + rec.normal + MoonMath::RandomInUnitSphere();
		scattered = Ray(rec.p, target - rec.p);
		attenuation = albedo;

		return true;
	}
#pragma endregion

#pragma region metal
	bool Metal::Scatter(const Ray &r_in, const HitRecord &rec, Vector3 &attenuation, Ray &scattered) const {
		Vector3 normDir = Vector3::Normalize(r_in.dir);
		Vector3 reflected = MoonMath::Reflect(normDir, rec.normal);
		scattered = Ray(rec.p, reflected + fuzz * MoonMath::RandomInUnitSphere());
		attenuation = albedo;

		return (scattered.dir.dot(rec.normal) > 0);
	}
#pragma endregion

#pragma region dielectric
	bool Dielectric::Scatter(const Ray &r_in, const HitRecord &rec, Vector3 &attenuation, Ray &scattered) const {
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

#pragma region SEM
	SEM::SEM() {
		matcap = MOON_TextureManager::LoadTexture("./Assets/Textures/MatCap/matcap.jpg");
		shader = MOON_ShaderManager::CreateShader("SEM", "SEM.vs", "SEM.fs");
	}
	SEM::SEM(const std::string &name) : Material(name) {
		matcap = MOON_TextureManager::LoadTexture("./Assets/Textures/MatCap/matcap.jpg");
		shader = MOON_ShaderManager::CreateShader("SEM", "SEM.vs", "SEM.fs");
	}

	bool SEM::Scatter(const Ray &r_in, const HitRecord &rec, Vector3 &attenuation, Ray &scattered) const {

		return true;
	}
#pragma endregion

#pragma region Skin
	SkinMtl::SkinMtl() {
		DefineTextures({
			"albedo", "normal", "roughness", "scatter", "SSSLUT", "kelemenLUT"
		});

		tintColor.setValue(0.8f, 0.8f, 0.8f); specColor.setValue(0.8f, 0.8f, 0.8f);
		scatColor.setValue(0.8f, 0.8f, 0.8f);

		roughness = 0.1f; specMulti = 1.0f; curveFactor = 1.0f;
		distortion = 0.0f; scatMulti = 1.0f; scatScale = 1.0f;

		shader = MOON_ShaderManager::CreateShader("PreIntegerSSS", "PBR.vs", "PreIntegerSSS.fs");
	}

	SkinMtl::SkinMtl(const std::string &name) : Material(name) {
		DefineTextures({
			"albedo", "normal", "roughness", "scatter", "SSSLUT", "kelemenLUT"
		});

		tintColor.setValue(0.8f, 0.8f, 0.8f); specColor.setValue(0.8f, 0.8f, 0.8f);
		scatColor.setValue(0.8f, 0.8f, 0.8f);

		roughness = 0.1f; specMulti = 1.0f; curveFactor = 1.0f;
		distortion = 0.0f; scatMulti = 1.0f; scatScale = 1.0f;

		shader = MOON_ShaderManager::CreateShader("PreIntegerSSS", "PBR.vs", "PreIntegerSSS.fs");
	}

	void SkinMtl::SetShaderProps(Shader* shader) {
		Material::SetShaderProps(shader);

		// legacy shader compatible ---------------------------------------------
		shader->setVec3("objectColor", tintColor);
		shader->setFloat("specularLV", roughness * 1024);

		// colors ---------------------------------------------------------------
		shader->setVec3("tintColor", tintColor);
		shader->setVec3("specColor", specColor);
		shader->setVec3("scatColor", scatColor);

		// other props ----------------------------------------------------------
		shader->setFloat("_roughness", roughness);
		shader->setFloat("_specMulti", specMulti);
		shader->setFloat("_curveFactor", curveFactor);
		shader->setFloat("_distortion", distortion);
		shader->setFloat("_scatMulti", scatMulti);
		shader->setFloat("_scatScale", scatScale);

		// textures -------------------------------------------------------------
		for (int i = 0; i < textures.size(); i++) {
			shader->setBool(texTypeList[i] + "Switch", textures[i] != nullptr);
			if (textures[i] != nullptr) shader->setTexture(texTypeList[i] + "Map", textures[i], i + 3);
		}
	}

	bool SkinMtl::Scatter(const Ray &r_in, const HitRecord &rec, Vector3 &attenuation, Ray &scattered) const {
		return LambertScatter(
			Vector3::ONE() * 0.8f, 0.8f,
			r_in, rec, attenuation, scattered
		);

		return true;
	}
#pragma endregion

}