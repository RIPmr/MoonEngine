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

		Shader* shader;
		Texture* preview;

		// if any param is updated, set it true
		bool prevNeedUpdate;

		Material() : ObjectBase(MOON_AUTOID), preview(MOON_UNSPECIFIEDID), prevNeedUpdate(true) {}
		Material(const std::string &name) : ObjectBase(name, MOON_AUTOID), preview(MOON_UNSPECIFIEDID), prevNeedUpdate(true) {}
		//~Material() { delete shader; }
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

		virtual void GeneratePreview();
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

		virtual void SetShaderProps(Shader* shader) override {
			// colors
			shader->setVec3("ambientC",		ambientC);
			shader->setVec3("objectColor",	diffuseC);
			shader->setVec3("reflectW",		reflectW);
			shader->setVec3("refractW",		refractW);
			shader->setVec3("translucency", translucency);
			shader->setVec3("opacity",		opacity);
			shader->setVec3("fogC",			fogC);
			shader->setVec3("illumination", illumination);
			shader->setVec3("metalness",	metalness);

			// floats
			shader->setFloat("roughness",	roughness);
			shader->setFloat("glossiness",	glossiness);
			shader->setFloat("IOR",			IOR);
			shader->setFloat("fogW",		fogW);
			shader->setFloat("dispW",		dispW);
			shader->setFloat("anisotropy",	anisotropy);
			shader->setFloat("an_rotation", an_rotation);

			// textures
			for (int i = 0; i < textures.size(); i++) {
				shader->setTexture(enum_to_string(textures[i]->type), textures[i], i);
			}
		}

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

			TextureButton(ambient,		"Ambient",		(float*)&ambientC,		btnSize, interval, bid);
			TextureButton(diffuse,		"Diffuse",		(float*)&diffuseC,		btnSize, interval, bid);
			TextureButton(normal,	"Normal",	nullptr, ImVec2(btnSize.x + 30, btnSize.y), interval, bid);
			TextureButton(reflect,		"Reflect",		(float*)&reflectW,		btnSize, interval, bid);
			TextureButton(refract,		"Refract",		(float*)&refractW,		btnSize, interval, bid);
			TextureButton(metallic,		"Metalness",	(float*)&metalness,		btnSize, interval, bid);
			TextureButton(translucent,	"Translucent",	(float*)&translucency,	btnSize, interval, bid);
			TextureButton(displacement, "Displace", nullptr, ImVec2(btnSize.x + 30, btnSize.y), interval, bid);
			TextureButton(illuminant,	"Illuminant",	(float*)&illumination,	btnSize, interval, bid);
			TextureButton(alpha,		"Opacity",		(float*)&opacity,		btnSize, interval, bid);

			ImGui::PushID("fog");
			ImGui::Text("Fog/Multi"); ImGui::SameLine(interval);
			ImGui::ColorEdit3(UniquePropName("fog"), (float*)&fogC, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel);
			ImGui::SameLine();
			ImGui::SetNextItemWidth(btnSize.x);
			ImGui::DragFloat("fog_drag", &fogW, 0.1f, 0.0f, 10.0f, "%.1f", 1.0f, true);
			ImGui::PopID();

			ImGui::PushID("rough");
			ImGui::Text("Roughness"); ImGui::SameLine(interval);
			ImGui::SetNextItemWidth(btnSize.x + 30);
			ImGui::DragFloat("rough_drag", &roughness, 0.1f, 0.0f, 1.0f, "%.1f", 1.0f, true);
			ImGui::PopID();

			ImGui::PushID("gloss");
			ImGui::Text("Glossiness"); ImGui::SameLine(interval);
			ImGui::SetNextItemWidth(btnSize.x + 30);
			if (ImGui::DragFloat("gloss_drag", &glossiness, 0.1f, 0.0f, 1.0f, "%.1f", 1.0f, true)) {
				this->prevNeedUpdate = true;
			}
			ImGui::PopID();

			ImGui::PushID("ior");
			ImGui::Text("IOR"); ImGui::SameLine(interval);
			ImGui::SetNextItemWidth(btnSize.x + 30);
			if (ImGui::DragFloat("ior_drag", &IOR, 0.1f, 1.0f, 10.0f, "%.1f", 1.0f, true)) {
				this->prevNeedUpdate = true;
			}
			ImGui::PopID();

			ImGui::PushID("dispW");
			ImGui::Text("DispMulti"); ImGui::SameLine(interval);
			ImGui::SetNextItemWidth(btnSize.x + 30);
			ImGui::DragFloat("dispW_drag", &dispW, 0.1f, 0, 0, "%.1f", 1.0f, true);
			ImGui::PopID();

			ImGui::PushID("aniso");
			ImGui::Text("Anisotropy"); ImGui::SameLine(interval);
			ImGui::SetNextItemWidth(btnSize.x + 30);
			ImGui::DragFloat("aniso_drag", &anisotropy, 0.1f, 0.0f, 1.0f, "%.1f", 1.0f, true);
			ImGui::PopID();

			ImGui::PushID("anirot");
			ImGui::Text("AnRotation"); ImGui::SameLine(interval);
			ImGui::SetNextItemWidth(btnSize.x + 30);
			ImGui::DragFloat("anirot_drag", &an_rotation, 0.1f, 0.0f, 1.0f, "%.1f", 1.0f, true);
			ImGui::PopID();

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