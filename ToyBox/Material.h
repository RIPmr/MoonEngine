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
		light,
		sem
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

		Material() : ObjectBase(MOON_AUTOID), preview(MOON_UNSPECIFIEDID), prevNeedUpdate(true) {}
		Material(const std::string &name) : ObjectBase(name, MOON_AUTOID), preview(MOON_UNSPECIFIEDID), prevNeedUpdate(true) {}

		virtual ~Material() override {
			if (preview != NULL) delete preview;
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

		virtual void SetShaderProps(Shader* shader) {}
		virtual bool scatter(const Ray &r_in, const HitRecord &rec, Vector3 &attenuation, Ray &scattered) const = 0;
	};

	class LightMtl : public Material {
	public:
		LightMtl();
		LightMtl(const std::string &name);

		virtual bool scatter(const Ray &r_in, const HitRecord &rec, Vector3 &attenuation, Ray &scattered) const;
	};

	class MoonMtl : public Material {
	public:
		// C: color, W: weight
		Vector3 ambientC;

		Vector3 diffuseC;
		float roughness;

		Vector3 reflectW;
		Vector3 refractW;
		float fresnel;
		float glossiness;
		float IOR;

		Vector3 translucency;
		Vector3 opacity;
		Vector3 fogC;
		float fogW;

		float dispW;

		Vector3 illumination;

		Vector3 metalness;
		float anisotropy;
		float an_rotation;

		std::vector<Texture*> textures;

		MoonMtl();
		MoonMtl(const std::string &name);
		~MoonMtl() override;

		int GetTextureIndex(const TexType& type) {
			for (int i = 0; i < textures.size(); i++) {
				if (type == textures[i]->type) return i;
			}
			return -1;
		}

		void TextureButton(const TexType& type, const std::string& title, float* colorRef,
			const ImVec2& btnSize, const float& interval, unsigned int& loopID) {
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

			TextureButton(ambientMap,		"Ambient",		(float*)&ambientC,		btnSize, interval, bid);
			TextureButton(diffuseMap,		"Diffuse",		(float*)&diffuseC,		btnSize, interval, bid);
			TextureButton(normalMap,		"Normal",	nullptr, ImVec2(btnSize.x + 30, btnSize.y), interval, bid);
			TextureButton(reflectMap,		"Reflect",		(float*)&reflectW,		btnSize, interval, bid);
			TextureButton(refractMap,		"Refract",		(float*)&refractW,		btnSize, interval, bid);
			TextureButton(metallicMap,		"Metalness",	(float*)&metalness,		btnSize, interval, bid);
			TextureButton(translucentMap,	"Translucent",	(float*)&translucency,	btnSize, interval, bid);
			TextureButton(displaceMap,		"Displace", nullptr, ImVec2(btnSize.x + 30, btnSize.y), interval, bid);
			TextureButton(illuminaMap,		"Illuminant",	(float*)&illumination,	btnSize, interval, bid);
			TextureButton(alphaMap,			"Opacity",		(float*)&opacity,		btnSize, interval, bid);

			ImGui::Text("Fog/Multi"); ImGui::SameLine(interval);
			ImGui::ColorEdit3(UniquePropName("fog"), (float*)&fogC, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel);
			ImGui::SameLine();
			ImGui::SetNextItemWidth(btnSize.x);
			ButtonEx::DragFloatNoLabel("fog_drag", &fogW, 0.001f, 0.0f, 10.0f, "%.3f", 1.0f);

			ImGui::Text("Roughness"); ImGui::SameLine(interval);
			ImGui::SetNextItemWidth(btnSize.x + 30);
			ButtonEx::DragFloatNoLabel("rough_drag", &roughness, 0.001f, 0.0f, 1.0f, "%.3f", 1.0f);

			ImGui::Text("Glossiness"); ImGui::SameLine(interval);
			ImGui::SetNextItemWidth(btnSize.x + 30);
			if (ButtonEx::DragFloatNoLabel("gloss_drag", &glossiness, 0.001f, 0.0f, 1.0f, "%.3f", 1.0f)) {
				this->prevNeedUpdate = true;
			}

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

			ImGui::Text("DispMulti"); ImGui::SameLine(interval);
			ImGui::SetNextItemWidth(btnSize.x + 30);
			ButtonEx::DragFloatNoLabel("dispW_drag", &dispW, 0.001f, 0, 0, "%.3f", 1.0f);

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

		virtual bool scatter(const Ray &r_in, const HitRecord &rec, Vector3 &attenuation, Ray &scattered) const;
	};

	class Lambertian : public Material {
	public:
		Vector3 albedo;

		Lambertian(const Vector3 &albedo) : albedo(albedo) {}
		Lambertian(const std::string &name, const Vector3 &albedo) : albedo(albedo), Material(name) {}

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
		Metal(const std::string &name, const Vector3 &albedo, float fuzz) : albedo(albedo), Material(name) {
			if (fuzz < 1) this->fuzz = fuzz;
			else this->fuzz = 1;
		}

		virtual bool scatter(const Ray &r_in, const HitRecord &rec, Vector3 &attenuation, Ray &scattered) const;
	};

	class Dielectric : public Material {
	public:
		float ref_idx;

		Dielectric(float ri) : ref_idx(ri) {}
		Dielectric(const std::string &name, float ri) : ref_idx(ri), Material(name) {}

		virtual bool scatter(const Ray &r_in, const HitRecord &rec, Vector3 &attenuation, Ray &scattered) const;
	};

	class SEM : public Material {
	public:
		Texture* normal;
		Texture* matcap;

		SEM();
		SEM(const std::string &name);

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

		virtual bool scatter(const Ray &r_in, const HitRecord &rec, Vector3 &attenuation, Ray &scattered) const;
	};
}