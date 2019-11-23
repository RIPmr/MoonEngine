#pragma once
#include <string>

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

		Material() : ObjectBase(MOON_AUTOID) {}
		Material(const std::string &name) : ObjectBase(name, MOON_AUTOID) {}
		//~Material() { delete shader; }
		~Material() override {}

		void ListShader() {
			if (ImGui::TreeNode(UniquePropName("Shader"))) {
				ImGui::Text((std::string(ICON_FA_FILE_CODE_O) + " " + shader->name).c_str());
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

			// Ambient
			ImGui::Text("Ambient "); ImGui::SameLine(125.0f);
			ImGui::ColorEdit3(UniquePropName("Ka"), (float*)&Ka, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel);
			ImGui::SameLine();
			ImGui::Button("[Texture]");
			// Diffuse
			ImGui::Text("Diffuse "); ImGui::SameLine(125.0f);
			ImGui::ColorEdit3(UniquePropName("Kd"), (float*)&Kd, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel);
			ImGui::SameLine();
			ImGui::Button("[Texture]");
			// Specular
			ImGui::Text("Specular "); ImGui::SameLine(125.0f);
			ImGui::ColorEdit3(UniquePropName("Ks"), (float*)&Ks, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel);
			ImGui::SameLine();
			ImGui::Button("[Texture]");
			// Specular Exponent
			ImGui::Text("Specular Exponent "); ImGui::SameLine(125.0f);
			ImGui::ColorEdit3(UniquePropName("Ns"), (float*)&Ns, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel);
			ImGui::SameLine();
			ImGui::Button("[Texture]");
			// Optical Density
			ImGui::Text("Optical Density "); ImGui::SameLine(125.0f);
			ImGui::ColorEdit3(UniquePropName("Ni"), (float*)&Ni, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel);
			ImGui::SameLine();
			ImGui::Button("[Texture]");
			// Dissolve
			ImGui::Text("Dissolve "); ImGui::SameLine(125.0f);
			ImGui::ColorEdit3(UniquePropName("d"), (float*)&d, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel);
			ImGui::SameLine();
			ImGui::Button("[Texture]");
			// Illumination
			ImGui::Text("Illumination "); ImGui::SameLine(125.0f);
			ImGui::ColorEdit3(UniquePropName("illum"), (float*)&illum, ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel);
			ImGui::SameLine();
			ImGui::Button("[Texture]");

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