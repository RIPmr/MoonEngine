#pragma once
#include <imgui.h>
#include "Icons.h"
#include "MoonEnums.h"
#include "ButtonEx.h"
#include "Texture.h"
#include "Graphics.h"
#include "ButtonEx.h"
#include "PostFactory.h"

namespace MOON {

	enum ToneMappingMethod {
		Tone_Reinhard,
		Tone_Cineon,
		Tone_ACES,
		Tone_Filmic,
		Tone_LUT
	};

	class ColorSpaceConverter : public PostEffect {
	private:
		ColorSpace colSpace;
		bool direction;

	public:

		ColorSpaceConverter() : PostProcessing("ColorSpaceConverter", "ColorSpace") {
			colSpace = ColorSpace::Linear;
			direction = true; // from linear
		}
		~ColorSpaceConverter() override {}

		bool ListProperties() override {
			bool changeFlag = false;
			const static char* items[] = { 
				"Linear", "sRGB", "Gamma", "Cineon",
				"Canon_CLog", "AlexaV3LogC", "Panalog", "PLogLin",
				"RedLog", "Sony_SLog", "Sony_SLog2", "ViperLog",
				"rec709", "rec2020", "ACES", "ACEScg" 
			};
			ImGui::AlignTextToFramePadding();
			ImGui::Text("Color Space"); ImGui::SameLine(80.0f);
			if (ButtonEx::ComboNoLabel("colorSpace", (int*)&colSpace, items, IM_ARRAYSIZE(items))) {
				changeFlag = true;
			}

			const static char* dir[] = {"To Linear (In)", "From Linear (Out)"};
			ImGui::AlignTextToFramePadding();
			ImGui::Text("Direction"); ImGui::SameLine(80.0f);
			if (ButtonEx::ComboNoLabel("direction", (int*)&direction, dir, IM_ARRAYSIZE(dir))) {
				changeFlag = true;
			}

			return changeFlag;
		}
		void ConfigureProps() override {

		}
	};

	class ToneMapping : public PostEffect {
	public:
		ToneMappingMethod type;
		float gamma;
		Texture* lut;

		std::string lutPath;
		bool convertToLog;

		ToneMapping() : PostProcessing("ToneMapping", "ToneMapping"), lut(nullptr) {
			type = Tone_Reinhard; gamma = 2.2f;
			lutPath = "[LUT]";
			convertToLog = false;
		}
		~ToneMapping() override {
			//if (lut != nullptr) delete lut;
		}

		bool ListProperties() override {
			bool changeFlag = false;
			const static char* items[] = { "Reinhard", "Cineon", "ACES", "Filmic" };
			ImGui::AlignTextToFramePadding();
			ImGui::Text("Method"); ImGui::SameLine(80.0f);
			if (ButtonEx::ComboNoLabel("tonemapType", (int*)&type, items, IM_ARRAYSIZE(items))) {
				changeFlag = true;
			}

			ImGui::Text("Gamma"); ImGui::SameLine(80.0f);
			if (ImGui::DragFloat("gamma", &gamma, 0.1f, 0, 0, "%.1f", 1.0f, true)) {
				changeFlag = true;
			}

			if (type == Tone_Cineon) {
				ImGui::Text("LUT"); ImGui::SameLine(80.0f);
				ImVec2 size{ ImGui::GetContentRegionAvailWidth(), 22.0f };
				if (ButtonEx::TexFileBtnWithPrev(lut, TexType::defaultType, size)) {
					changeFlag = true;
				}
			} else if (type == Tone_LUT) {
				if (ButtonEx::FileButton(lutPath, ImVec2(ImGui::GetContentRegionAvailWidth() - 32, 22))) {
					changeFlag = true;
				}
				ImGui::SameLine();
				if (ImGui::Button(ICON_FA_TIMES, ImVec2(22, 22))) {
					lutPath = "[LUT]";
					changeFlag = true;
				}
				if (ImGui::Checkbox("Convert To Log Before Applying LUT", &convertToLog)) {
					changeFlag = true;
				}
			}

			return changeFlag;
		}
		void ConfigureProps() override {
			shader->setInt("type", type);
			shader->setFloat("gamma", gamma);
			if (lut != nullptr) shader->setTexture("FilmLut", lut, 1);
		}
	};

	class Exposure : public PostEffect {
	public:
		float highlight;
		float contrast;
		float exposure;

		Exposure() : PostProcessing("Exposure", "Exposure") {
			exposure = 1.0f; contrast = 0.0f; highlight = 1.0f;
		}
		~Exposure() override {}

		bool ListProperties() override {
			bool changeFlag = false;
			ImGui::Text("Exposure"); ImGui::SameLine(80.0f);
			if (ImGui::DragFloat("exp", &exposure, 0.1f, 0, 0, "%.1f", 1.0f, true)) {
				changeFlag = true;
			}

			ImGui::Text("Highlight"); ImGui::SameLine(80.0f);
			if (ImGui::DragFloat("highlight", &highlight, 0.1f, 0, 1, "%.1f", 1.0f, true)) {
				changeFlag = true;
			}

			ImGui::Text("Contrast"); ImGui::SameLine(80.0f);
			if (ImGui::DragFloat("contrast", &contrast, 0.1f, 0, 0, "%.1f", 1.0f, true)) {
				changeFlag = true;
			}

			return changeFlag;
		}
		void ConfigureProps() override {
			shader->setFloat("gamma", 1.0f);
			shader->setFloat("exposure", exposure);
			shader->setFloat("highlight", highlight);
			shader->setFloat("contrast", contrast);
		}
	};

	class ColorCorrection : public PostEffect {
	public:
		Vector3 balance;
		float whiteBalance;
		float saturation;
		float hue;

		ColorCorrection() : PostProcessing("ColorCorrection", "ColorCorrection") {
			balance.setValue(0, 0, 0);
			whiteBalance = 11000 >> 1;
			saturation = 0;
			hue = 0;
		}
		~ColorCorrection() override {}

		bool ListProperties() override {
			// white balance
			ImGui::Text("White Balance (K)");
			ImGui::Indent(30);
				ButtonEx::SliderFloatNoLabel("wb", &whiteBalance, 1000, 10000, "%.0f");
			ImGui::Unindent(30);

			// hue / saturation
			ImGui::Text("Hue");
			ImGui::Indent(30);
				ButtonEx::SliderFloatNoLabel("hue", &hue, -100, 100, "%.1f");
			ImGui::Unindent(30);
			ImGui::Text("Saturation");
			ImGui::Indent(30);
				ButtonEx::SliderFloatNoLabel("sat", &saturation, 0, 100, "%.1f");
			ImGui::Unindent(30);

			// color balance
			ImGui::Text("Color Balance");
			ImGui::Indent(30);
				ImGui::SetWindowFontScale(0.8f);
				ImGui::Text("Cyan|Red");
				ImGui::SetWindowFontScale(1.0f);
				ButtonEx::SliderFloatNoLabel("cb_r", &balance.x, -100, 100, "%.1f");
				ImGui::SetWindowFontScale(0.8f);
				ImGui::Text("Magenta|Green");
				ImGui::SetWindowFontScale(1.0f);
				ButtonEx::SliderFloatNoLabel("cb_g", &balance.y, -100, 100, "%.1f");
				ImGui::SetWindowFontScale(0.8f);
				ImGui::Text("Yellow|Blue");
				ImGui::SetWindowFontScale(1.0f);
				ButtonEx::SliderFloatNoLabel("cb_b", &balance.z, -100, 100, "%.1f");
			ImGui::Unindent(30);

			return false;
		}
		void ConfigureProps() override {

		}
	};

	class Bloom : public PostEffect {
	public:

		Bloom() : PostProcessing("Bloom", "Bloom") {

		}
		~Bloom() override {}

		bool ListProperties() override {

			return false;
		}
		void ConfigureProps() override {

		}
	};

	class Flare : public PostEffect {
	public:

		Flare() : PostProcessing("Flare", "Flare") {

		}
		~Flare() override {}

		bool ListProperties() override {

			return false;
		}
		void ConfigureProps() override {

		}
	};

	class Levels : public PostEffect {
	public:

		Levels() : PostProcessing("Levels", "Levels") {

		}
		~Levels() override {}

		bool ListProperties() override {

			return false;
		}
		void ConfigureProps() override {

		}
	};

	class Curve : public PostEffect {
	public:

		Curve() : PostProcessing("Curve", "Curve") {

		}
		~Curve() override {}

		bool ListProperties() override {

			return false;
		}
		void ConfigureProps() override {

		}
	};

	class SSAO : public PostEffect {
	public:

		SSAO() : PostProcessing("SSAO", "SSAO") {

		}
		~SSAO() override {}

		bool ListProperties() override {

			return false;
		}
		void ConfigureProps() override {

		}
	};

	class ScreenSpaceReflection : public PostEffect {
	public:

		ScreenSpaceReflection() : PostProcessing("ScreenSpaceReflection", "SSR") {

		}
		~ScreenSpaceReflection() override {}

		bool ListProperties() override {

			return false;
		}
		void ConfigureProps() override {

		}
	};

	class FXAA : public PostEffect {
	public:
		int FXAA_SPAN_MAX;

		FXAA() : PostProcessing("FXAA", "FXAA") {
			FXAA_SPAN_MAX = 8;
		}
		~FXAA() override {}

		bool ListProperties() override {
			bool changeFlag = false;
			ImGui::Text("span"); ImGui::SameLine(80.0f);
			if (ImGui::DragInt("span", &FXAA_SPAN_MAX, 1, 0, 0, "%d", true)) {
				changeFlag = true;
			}

			return changeFlag;
		}
		void ConfigureProps() override {
			shader->setInt("FXAA_SPAN_MAX", FXAA_SPAN_MAX);
		}
	};

}