#pragma once
#include <string>

#include "Ray.h"
#include "MathUtils.h"
#include "MShader.h"
#include "Hitable.h"
#include "Texture.h"
#include "ObjectBase.h"
#include "MatSphere.h"
#include "ButtonEx.h"

namespace MOON {

	enum MatType {
		moonMtl,
		lambertian,
		metal,
		dielectric,
		lightMtl,
		skinMtl,
		hairMtl,
		matteMtl,
		semMtl
	};

	extern class renderer;
	class Material : public ObjectBase {
	public:
		static Vector2 PREVSIZE;
		static bool autoPrevUpdate;

		Shader* shader;
		Texture* preview;

		// if any param is updated, set it true
		bool prevNeedUpdate;

		std::vector<Texture*> textures;
		std::vector<std::string> texTypeList;

		Material() : ObjectBase(MOON_AUTOID), preview(MOON_UNSPECIFIEDID), prevNeedUpdate(true) {}
		Material(const std::string &name) : ObjectBase(name, MOON_AUTOID), preview(MOON_UNSPECIFIEDID), prevNeedUpdate(true) {}

		virtual ~Material() override {
			if (preview != NULL) delete preview;
		}

		int GetTextureID(const std::string& type) {
			for (int i = 0; i < texTypeList.size(); i++) {
				if (texTypeList[i]._Equal(type)) return i;
			}
			return -1;
		}

		Texture* GetTexture(const std::string& type) {
			for (int i = 0; i < texTypeList.size(); i++) {
				if (texTypeList[i]._Equal(type)) return textures[i];
			}
			return nullptr;
		}

		void DefineTextures(const std::vector<std::string>& texTypeList) {
			this->texTypeList = texTypeList;
			textures.resize(texTypeList.size(), nullptr);
		}

		void ListShader();

		virtual void ListProperties() override {
			// list name
			ListName();
			ImGui::Separator();

			// list shader
			ListShader();
			ImGui::Separator();
		}

		virtual void GeneratePreview(bool manualUpdate = false);
		virtual void UpdatePreview();
		virtual void ListPreview();

		virtual void SetShaderProps(Shader* shader);
		virtual Vector3 Emitted(const Vector2& uv) const { return Vector3::ZERO(); }
		virtual bool PDF(const Ray& r_in, const HitRecord& rec, const Ray& scattered) const { return false; }
		virtual bool Scatter_IS(const Ray &r_in, const HitRecord &rec, Vector3 &attenuation, Ray &scattered, float& pdf) const { return false; }
		virtual bool Scatter(const Ray &r_in, const HitRecord &rec, Vector3 &attenuation, Ray &scattered) const = 0;
	};

	class LightMtl : public Material {
	public:
		Texture* texture;

		Vector3 color;
		float power;

		LightMtl();
		LightMtl(const std::string &name);
		~LightMtl() override = default;

		virtual void SetShaderProps(Shader* shader) override;

		virtual void ListProperties() override {
			Material::ListProperties();

			// list parameters
			ImGui::Text("Parameters:");
			ImGui::Indent(10.0f);

			// Ambient
			float interval = 90.0f; unsigned int bid = 0;
			ImVec2 btnSize{ ImGui::GetContentRegionAvailWidth() - interval - 20, 22 };

			ImGui::Text("Color"); ImGui::SameLine(interval);
			if (ImGui::ColorEdit3(UniquePropName("col"), (float*)&color, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel)) {
				this->prevNeedUpdate = true;
			}
			ImGui::SameLine();
			ImGui::SetNextItemWidth(btnSize.x);
			if (ButtonEx::DragFloatNoLabel("Power", &power, 0.01f, 0.0f, INFINITY, "%.3f", 1.0f)) {
				this->prevNeedUpdate = true;
			}

			ImGui::Unindent(10.0f);
			ImGui::Separator();

			// list preview
			ListPreview();

			ImGui::Spacing();
		}

		virtual Vector3 Emitted(const Vector2& uv) const override;
		virtual bool Scatter(const Ray &r_in, const HitRecord &rec, Vector3 &attenuation, Ray &scattered) const override;
	};

	class MoonMtl : public Material {
	public:
		// C: color, W: weight
		float ambientW;
		float normalW;
		float dispW;

		Vector3 diffuseC;
		float roughness;

		Vector3 reflectW;
		Vector3 refractW;
		Vector3 glossiness;
		float fresnel;
		float IOR;

		Vector3 translucency;
		Vector3 opacity;
		Vector3 fogC;
		float fogW;

		Vector3 illumination;
		float illumMulti;

		Vector3 metalness;
		float anisotropy;
		float an_rotation;

		MoonMtl();
		MoonMtl(const std::string &name);
		~MoonMtl() override = default;

		/*void TextureButton(const TexType& type, const std::string& title, float* colorRef,
			const ImVec2& btnSize, const float& interval, unsigned int& loopID) {
			ImGui::PushID(title.c_str());
			auto tid = GetTextureIndex(type);
			ImGui::Text((title + " ").c_str()); ImGui::SameLine(interval);
			if (colorRef != nullptr) {
				if (ImGui::ColorEdit3(UniquePropName(title), colorRef,
					ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel)) {
					this->prevNeedUpdate = true;
				} ImGui::SameLine();
			}
			if (tid < 0) {
				Texture* tex = nullptr;
				if (ButtonEx::TexFileBtnWithPrev(tex, type, btnSize, loopID++)) {
					textures.push_back(tex);
					this->prevNeedUpdate = true;
				}
			} else {
				if (ButtonEx::TexFileBtnWithPrev(textures[tid], type, btnSize, loopID++)) {
					if (textures[tid] == nullptr) Utility::RemoveElemAt(textures, tid);
					this->prevNeedUpdate = true;
				}
			}
			ImGui::PopID();
		}*/

		void TexColButton(const std::string& type, float* colorRef,
			const ImVec2& btnSize, const float& interval, unsigned int& loopID) {
			ImGui::PushID(type.c_str());
			ImGui::Text((type + " ").c_str()); ImGui::SameLine(interval);
			if (colorRef != nullptr) {
				if (ImGui::ColorEdit3(UniquePropName(type), colorRef,
					ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel)) {
					this->prevNeedUpdate = true;
				} ImGui::SameLine();
			}
			if (ButtonEx::TexFileBtnWithPrev(textures[GetTextureID(type)], btnSize, loopID++)) {
				this->prevNeedUpdate = true;
			}
			ImGui::PopID();
		}

		void TexSpiButton(const std::string& type, const char* spinTitle, 
			float* spinRef, const Vector2& spinRange, const ImVec2& btnSize, 
			const float& interval, unsigned int& loopID) {
			ImGui::PushID(type.c_str());
			ImGui::Text((type + " ").c_str()); ImGui::SameLine(interval);
			if (ButtonEx::TexFileBtnWithPrev(textures[GetTextureID(type)], btnSize, loopID++)) {
				this->prevNeedUpdate = true;
			}
			if (spinRef != nullptr) {
				ImGui::Text(spinTitle); ImGui::SameLine(interval);
				ImGui::SetNextItemWidth(btnSize.x);
				if (ButtonEx::SliderFloatNoLabel(UniquePropName(type),
					spinRef, spinRange.x, spinRange.y, "%.3f", 1.0f)) {
					this->prevNeedUpdate = true;
				}
			}
			ImGui::PopID();
		}

		void TexDraButton(const std::string& type, const char* draTitle, 
			float* draRef, const Vector2& draRange, const ImVec2& btnSize,
			const float& interval, unsigned int& loopID) {
			ImGui::PushID(type.c_str());
			ImGui::Text((type + " ").c_str()); ImGui::SameLine(interval);
			if (ButtonEx::TexFileBtnWithPrev(textures[GetTextureID(type)], btnSize, loopID++)) {
				this->prevNeedUpdate = true;
			}
			if (draRef != nullptr) {
				ImGui::Text(draTitle); ImGui::SameLine(interval);
				ImGui::SetNextItemWidth(btnSize.x);
				if (ButtonEx::DragFloatNoLabel(UniquePropName(type),
					draRef, 0.01f, draRange.x, draRange.y)) {
					this->prevNeedUpdate = true;
				}
			}
			ImGui::PopID();
		}

		virtual void SetShaderProps(Shader* shader) override;

		virtual void ListProperties() override {
			Material::ListProperties();

			if (name._Equal("default")) {
				ListPreview();
				ImGui::Spacing();
				return ;
			}

			// list parameters
			ImGui::Text("Parameters:");
			ImGui::Indent(10.0f);

			// Ambient
			float interval = 90.0f; unsigned int bid = 0;
			ImVec2 btnSize{ ImGui::GetContentRegionAvailWidth() - interval - 20, 22 };

			TexSpiButton("ambient",		"AOWeight", &ambientW, Vector2(0, 1),	ImVec2(btnSize.x + 30, btnSize.y), interval, bid);
			TexColButton("albedo",		(float*)&diffuseC,		btnSize, interval, bid);
			TexDraButton("normal",		"NrmMulti", &normalW,  Vector2::ZERO(), ImVec2(btnSize.x + 30, btnSize.y), interval, bid);
			TexColButton("reflect",		(float*)&reflectW,		btnSize, interval, bid);
			TexColButton("refract",		(float*)&refractW,		btnSize, interval, bid);
			TexColButton("metallic",	(float*)&metalness,		btnSize, interval, bid);
			TexColButton("translucent",	(float*)&translucency,	btnSize, interval, bid);
			TexDraButton("displace",	"DispMulti",&dispW,   Vector2::ZERO(), ImVec2(btnSize.x + 30, btnSize.y), interval, bid);
			
			TexColButton("illumina",	(float*)&illumination,	btnSize, interval, bid);
			ImGui::Text("IllumMulti"); ImGui::SameLine(interval); 
			ImGui::SetNextItemWidth(btnSize.x + 30);
			ButtonEx::DragFloatNoLabel("illum_multi", &illumMulti, 0.1f, 0.0f, 0.0f, "%.3f", 1.0f);

			TexColButton("alpha",		(float*)&opacity,		btnSize, interval, bid);

			ImGui::Text("Fog/Multi"); ImGui::SameLine(interval);
			ImGui::ColorEdit3(UniquePropName("fog"), (float*)&fogC, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel);
			ImGui::SameLine();
			ImGui::SetNextItemWidth(btnSize.x);
			ButtonEx::DragFloatNoLabel("fog_drag", &fogW, 0.001f, 0.0f, 10.0f, "%.3f", 1.0f);

			TexColButton("roughness",	(float*)&glossiness,	btnSize, interval, bid);

			ImGui::Text("Roughness"); ImGui::SameLine(interval);
			ImGui::SetNextItemWidth(btnSize.x + 30);
			ButtonEx::DragFloatNoLabel("rough_drag", &roughness, 0.001f, 0.0f, 1.0f, "%.3f", 1.0f);

			ImGui::Text("Fresnel"); ImGui::SameLine(interval);
			ImGui::SetNextItemWidth(btnSize.x + 30);
			if (ButtonEx::DragFloatNoLabel("fresnel_drag", &fresnel, 0.001f, 0.0f, 0.0f, "%.3f", 1.0f)) {
				this->prevNeedUpdate = true;
			}

			ImGui::Text("IOR"); ImGui::SameLine(interval);
			ImGui::SetNextItemWidth(btnSize.x + 30);
			if (ButtonEx::DragFloatNoLabel("ior_drag", &IOR, 0.001f, 1.0f, 10.0f, "%.3f", 1.0f)) {
				this->prevNeedUpdate = true;
			}

			ImGui::Text("Anisotropy"); ImGui::SameLine(interval);
			ImGui::SetNextItemWidth(btnSize.x + 30);
			ButtonEx::DragFloatNoLabel("aniso_drag", &anisotropy, 0.001f, 0.0f, 1.0f, "%.3f", 1.0f);

			ImGui::Text("AnRotation"); ImGui::SameLine(interval);
			ImGui::SetNextItemWidth(btnSize.x + 30);
			ButtonEx::DragFloatNoLabel("anirot_drag", &an_rotation, 0.001f, 0.0f, 1.0f, "%.3f", 1.0f);

			ImGui::Unindent(10.0f);
			ImGui::Separator();

			// list preview
			ListPreview();

			ImGui::Spacing();
		}

		virtual Vector3 Emitted(const Vector2& uv) const override;
		virtual bool PDF(const Ray& r_in, const HitRecord& rec, const Ray& scattered) const override;
		virtual bool Scatter_IS(const Ray &r_in, const HitRecord &rec, Vector3 &attenuation, Ray &scattered, float& pdf) const override;
		virtual bool Scatter(const Ray &r_in, const HitRecord &rec, Vector3 &attenuation, Ray &scattered) const override;
	};

	class Lambertian : public Material {
	public:
		Vector3 albedo;

		Lambertian(const Vector3 &albedo) : albedo(albedo) {}
		Lambertian(const std::string &name, const Vector3 &albedo) : albedo(albedo), Material(name) {}
		~Lambertian() override = default;

		virtual bool Scatter(const Ray &r_in, const HitRecord &rec, Vector3 &attenuation, Ray &scattered) const;
	};

	class Metal : public Material {
	public:
		Vector3 albedo;
		float fuzz;

		Metal(const Vector3 &albedo, float fuzz) : albedo(albedo) {
			if (fuzz < 1) this->fuzz = fuzz;
			else this->fuzz = 1;
		}
		Metal(const std::string &name, const Vector3 &albedo, float fuzz) : albedo(albedo), Material(name) {
			if (fuzz < 1) this->fuzz = fuzz;
			else this->fuzz = 1;
		}
		~Metal() override = default;

		virtual bool Scatter(const Ray &r_in, const HitRecord &rec, Vector3 &attenuation, Ray &scattered) const;
	};

	class Dielectric : public Material {
	public:
		float ref_idx;

		Dielectric(float ri) : ref_idx(ri) {}
		Dielectric(const std::string &name, float ri) : ref_idx(ri), Material(name) {}
		~Dielectric() override = default;

		virtual bool Scatter(const Ray &r_in, const HitRecord &rec, Vector3 &attenuation, Ray &scattered) const;
	};

	class SEM : public Material {
	public:
		Texture* normal;
		Texture* matcap;

		SEM();
		SEM(const std::string &name);
		~SEM() override = default;

		virtual void SetShaderProps(Shader* shader) override {
			shader->setTexture("tMatCap", matcap, 0);
		}

		virtual void ListProperties() override {
			Material::ListProperties();

			// show matcap
			ImGui::Text("Matcap: ");
			float maxPrevWidth = 124.0f;
			float centering = (ImGui::GetContentRegionAvailWidth() - maxPrevWidth) / 2.0f;
			ImGui::Indent(centering);
			ButtonEx::FileButtonEx((void**)&matcap, matcap->path.c_str(), ImVec2(maxPrevWidth, 0), this->ID);
			ImGui::Image((void*)(intptr_t)matcap->localID, ImVec2(maxPrevWidth, matcap->height * maxPrevWidth / matcap->width));
			ImGui::Unindent(centering);
		}

		virtual bool Scatter(const Ray &r_in, const HitRecord &rec, Vector3 &attenuation, Ray &scattered) const;
	};

	class MatteMtl : public Material {
	public:
		bool writeAlpha;

		MatteMtl();
		MatteMtl(const std::string &name);
		~MatteMtl() override = default;

		virtual bool Scatter(const Ray &r_in, const HitRecord &rec, Vector3 &attenuation, Ray &scattered) const;
	};

	class HairMtl : public Material {
	public:
		Vector3 color;

		HairMtl();
		HairMtl(const std::string &name);
		~HairMtl() override = default;

		virtual bool Scatter(const Ray &r_in, const HitRecord &rec, Vector3 &attenuation, Ray &scattered) const;
	};

	class SkinMtl : public Material {
	public:
		int specType;

		Vector3 tintColor;
		Vector3 specColor;
		Vector3 scatColor;

		Texture* albedoMap;
		Texture* normalMap;
		Texture* roughnessMap;
		Texture* scatterMap;
		Texture* SSSLUT;
		Texture* kelemenLUT;

		float roughness;
		float specMulti;
		float curveFactor;

		float distortion;
		float scatMulti;
		float scatScale;

		SkinMtl();
		SkinMtl(const std::string &name);
		~SkinMtl() override = default;

		virtual void SetShaderProps(Shader* shader) override;

		virtual void ListProperties() override {
			Material::ListProperties();

			if (name._Equal("default")) {
				ListPreview();
				ImGui::Spacing();
				return;
			}

			// list parameters
			ImGui::Text("Parameters:");
			ImGui::Indent(10.0f);

			// Ambient
			float interval = 90.0f; unsigned int bid = 0;
			ImVec2 btnSize{ ImGui::GetContentRegionAvailWidth() - interval - 20, 22 };


			
			ImGui::Unindent(10.0f);
			ImGui::Separator();

			// list preview
			ListPreview();

			ImGui::Spacing();
		}

		virtual bool Scatter(const Ray &r_in, const HitRecord &rec, Vector3 &attenuation, Ray &scattered) const;
	};

}