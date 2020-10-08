#pragma once
#include <imgui.h>
#include "Texture.h"
#include "Graphics.h"
#include "ButtonEx.h"
#include "PostFactory.h"

namespace MOON {

	enum ToneMappingMethod {
		Reinhard,
		Cineon,
		ACES,
		Filmic
	};

	class ToneMapping : public PostEffect {
	public:
		ToneMappingMethod type;
		float gamma;
		Texture* lut;

		ToneMapping() : PostProcessing("ToneMapping", "ToneMapping"), lut(nullptr) {
			type = Reinhard; gamma = 2.2f;
		}
		~ToneMapping() override {
			//if (lut != nullptr) delete lut;
		}

		void ListProperties() override {
			const char* items[] = { "Reinhard", "Cineon", "ACES", "Filmic" };
			ImGui::AlignTextToFramePadding();
			ImGui::Text("Method"); ImGui::SameLine(80.0f);
			ImGui::PushID("tonemapType");
			ImGui::Combo("", (int*)&type, items, IM_ARRAYSIZE(items));
			ImGui::PopID();

			ImGui::Text("Gamma"); ImGui::SameLine(80.0f);
			ImGui::DragFloat("gamma", &gamma, 0.1f, 0, 0, "%.1f", 1.0f, true);

			if (type == Cineon) {
				ImGui::Text("LUT"); ImGui::SameLine(80.0f);
				ImVec2 size{ ImGui::GetContentRegionAvailWidth(), 22.0f };
				ButtonEx::TexFileBtnWithPrev(lut, TexType::defaultType, size);
			}
		}
		void ConfigureProps() override {
			shader->setInt("type", type);
			shader->setFloat("gamma", gamma);
			if (lut != nullptr) {
				shader->setTexture("FilmLut", lut, 1);
			}
		}
	};

	class Exposure : public PostEffect {
	public:
		float gamma;
		float exposure;

		Exposure() : PostProcessing("Exposure", "Exposure") {
			gamma = 2.2f; exposure = 1.0f;
		}
		~Exposure() override {}

		void ListProperties() override {
			ImGui::Text("Gamma"); ImGui::SameLine(80.0f);
			ImGui::DragFloat("gamma", &gamma, 0.1f, 0, 0, "%.1f", 1.0f, true);

			ImGui::Text("Exposure"); ImGui::SameLine(80.0f);
			ImGui::DragFloat("exp", &exposure, 0.1f, 0, 0, "%.1f", 1.0f, true);
		}
		void ConfigureProps() override {
			shader->setFloat("gamma", gamma);
			shader->setFloat("exposure", exposure);
		}
	};

	class Bloom : public PostEffect {
	public:

		Bloom() : PostProcessing("Bloom", "Bloom") {

		}
		~Bloom() override {}

		void ListProperties() override {

		}
		void ConfigureProps() override {

		}
	};

	class Curve : public PostEffect {
	public:

		Curve() : PostProcessing("Curve", "Curve") {

		}
		~Curve() override {}

		void ListProperties() override {

		}
		void ConfigureProps() override {

		}
	};

	class SSAO : public PostEffect {
	public:

		SSAO() : PostProcessing("SSAO", "SSAO") {

		}
		~SSAO() override {}

		void ListProperties() override {

		}
		void ConfigureProps() override {

		}
	};

	class ScreenSpaceReflection : public PostEffect {
	public:

		ScreenSpaceReflection() : PostProcessing("ScreenSpaceReflection", "SSR") {

		}
		~ScreenSpaceReflection() override {}

		void ListProperties() override {

		}
		void ConfigureProps() override {

		}
	};

}