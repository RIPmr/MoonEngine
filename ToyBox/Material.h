#pragma once
#include <string>

#include "Ray.h"
#include "MathUtils.h"
#include "MShader.h"
#include "Hitable.h"
#include "Texture.h"
#include "ObjectBase.h"
#include "MatSphere.h"

namespace MOON {
	enum MatType {
		moonMtl,
		lambertian,
		metal,
		dielectric,
		light
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
		~Material() override {
			if (preview != NULL) delete preview;
		}

		void ListShader() {
			if (ImGui::TreeNode("Shader", ID)) {
				ImGui::Text(Icon_Name_To_ID(ICON_FA_FILE_CODE_O, " " + shader->name));
				ImGui::TreePop();
			}
		}

		virtual void ListProperties() override {
			// list name
			ListName();
			ImGui::Separator();

			// list shader
			ListShader();
			ImGui::Spacing();
		}

		virtual void GeneratePreview();
		virtual void UpdatePreview();
		virtual void ListPreview();

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
		Texture* map_d;			// Alpha Texture Map
		Texture* map_bump;		// Bump Map

		MoonMtl();
		MoonMtl(const std::string &name);

		virtual void ListProperties() override {
			// list name
			ListName();
			ImGui::Separator();

			// list shader
			ListShader();
			ImGui::Separator();

			// list parameters
			ImGui::Text("Parameters:");

			ImGui::Indent(10.0f);
			// Ambient
			ImGui::Text("Ambient "); ImGui::SameLine(100.0f);
			if (ImGui::ColorEdit3(UniquePropName("Ka"), (float*)&Ka, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel)) {
				this->prevNeedUpdate = true;
			}
			ImGui::SameLine();
			float btnWidth = ImGui::GetContentRegionAvailWidth() - 20;
			ImGui::Button("[Texture]", ImVec2(btnWidth, 20));
			// Diffuse
			ImGui::Text("Diffuse "); ImGui::SameLine(100.0f);
			if (ImGui::ColorEdit3(UniquePropName("Kd"), (float*)&Kd, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel)) {
				this->prevNeedUpdate = true;
			}
			ImGui::SameLine();
			ImGui::Button("[Texture]", ImVec2(btnWidth, 20));
			// Specular
			ImGui::Text("Specular "); ImGui::SameLine(100.0f);
			if (ImGui::ColorEdit3(UniquePropName("Ks"), (float*)&Ks, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel)) {
				this->prevNeedUpdate = true;
			}
			ImGui::SameLine();
			ImGui::Button("[Texture]", ImVec2(btnWidth, 20));
			// Specular Exponent
			ImGui::Text("SpecExpo "); ImGui::SameLine(100.0f);
			if (ImGui::ColorEdit3(UniquePropName("Ns"), (float*)&Ns, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel)) {
				this->prevNeedUpdate = true;
			}
			ImGui::SameLine();
			ImGui::Button("[Texture]", ImVec2(btnWidth, 20));
			// Optical Density
			ImGui::Text("OptiDens "); ImGui::SameLine(100.0f);
			if (ImGui::ColorEdit3(UniquePropName("Ni"), (float*)&Ni, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel)) {
				this->prevNeedUpdate = true;
			}
			ImGui::SameLine();
			ImGui::Button("[Texture]", ImVec2(btnWidth, 20));
			// Dissolve
			ImGui::Text("Dissolve "); ImGui::SameLine(100.0f);
			if (ImGui::ColorEdit3(UniquePropName("d"), (float*)&d, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel)) {
				this->prevNeedUpdate = true;
			}
			ImGui::SameLine();
			ImGui::Button("[Texture]", ImVec2(btnWidth, 20));
			// Illumination
			ImGui::Text("Illumina "); ImGui::SameLine(100.0f);
			if (ImGui::ColorEdit3(UniquePropName("illum"), (float*)&illum, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel)) {
				this->prevNeedUpdate = true;
			}
			ImGui::SameLine();
			ImGui::Button("[Texture]", ImVec2(btnWidth, 20));

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
}