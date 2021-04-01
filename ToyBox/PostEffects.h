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
			const static char* items[] = { "Reinhard", "Cineon", "ACES", "Filmic", "LUT" };
			ImGui::AlignTextToFramePadding();
			ImGui::Text("Method"); ImGui::SameLine(80.0f);
			if (ButtonEx::ComboNoLabel("tonemapType", (int*)&type, items, IM_ARRAYSIZE(items))) {
				changeFlag = true;
			}

			ImGui::Text("Gamma"); ImGui::SameLine(80.0f);
			if (type != Tone_LUT) {
				if (ButtonEx::DragFloatNoLabel("gamma", &gamma, 0.1f, 0, 0, "%.1f", 1.0f)) {
					changeFlag = true;
				}
			}

			if (type == Tone_Cineon) {
				ImGui::Text("LUT"); ImGui::SameLine(80.0f);
				ImVec2 size{ ImGui::GetContentRegionAvailWidth(), 22.0f };
				if (ButtonEx::TexFileBtnWithPrev(lut, size)) {
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
			exposure = 1.0f; contrast = 1.0f; highlight = 1.0f;
		}
		~Exposure() override {}

		bool ListProperties() override {
			bool changeFlag = false;
			ImGui::Text("Exposure"); ImGui::SameLine(80.0f);
			if (ButtonEx::DragFloatNoLabel("exp", &exposure, 0.01f, 0, INFINITY, "%.3f", 1.0f)) {
				changeFlag = true;
			}

			ImGui::Text("Highlight"); ImGui::SameLine(80.0f);
			if (ButtonEx::DragFloatNoLabel("highlight", &highlight, 0.01f, 0, 1, "%.3f", 1.0f)) {
				changeFlag = true;
			}

			ImGui::Text("Contrast"); ImGui::SameLine(80.0f);
			if (ButtonEx::DragFloatNoLabel("contrast", &contrast, 0.01f, 0, 1, "%.3f", 1.0f)) {
				changeFlag = true;
			}

			return changeFlag;
		}

		void ConfigureProps() override {
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
		bool debug;

		float threshold;
		float exposure;
		float weight;

		Bloom() : PostProcessing("Bloom", "Bloom") {
			debug = false;
			threshold = 2.00f;
			exposure = 0.50f;
			weight = 0.10f;
		}
		~Bloom() override {}

		bool ListProperties() override {
			bool changeFlag = false;

			ImGui::Text("Debug"); ImGui::SameLine(80.0f);
			if (ButtonEx::CheckboxNoLabel("debug", &debug)) {
				changeFlag = true;
			}

			ImGui::Text("Threshold"); ImGui::SameLine(80.0f);
			if (ButtonEx::DragFloatNoLabel("threshold", &threshold, 0.1f, 0, 0, "%.2f", 1.0f)) {
				changeFlag = true;
			}

			ImGui::Text("Weight"); ImGui::SameLine(80.0f);
			if (ButtonEx::DragFloatNoLabel("weight", &weight, 0.1f, 0.0f, 100.0f, "%.2f")) {
				changeFlag = true;
			}

			ImGui::Text("Soft Knee"); ImGui::SameLine(80.0f);
			if (ButtonEx::DragFloatNoLabel("exposure", &exposure, 0.1f, 0.1f, 2.0f, "%.2f")) {
				changeFlag = true;
			}
			return changeFlag;
		}

		void ConfigureProps() override {
			shader->setFloat("_threshold", threshold);
			shader->setFloat("_exposure", exposure);
			shader->setFloat("_weight", weight);
			shader->setFloat("debug", debug);
		}
	};

	class Flare : public PostEffect {
	public:
		Vector3 tint;

		float threshold;
		float intensity;
		float stretch;
		float brightness;

		Flare() : PostProcessing("Flare", "Flare") {
			threshold = 0.5f;
			intensity = 500.0f;
			stretch = 0.5f;
			brightness = 1.0f;
			tint.setValue(0.5, 0.4, 1.0);
		}
		~Flare() override {}

		bool ListProperties() override {
			bool changeFlag = false;

			ImGui::Text("Tint Color"); ImGui::SameLine(80.0f);
			if (ButtonEx::ColorEdit3NoLabel("tint", (float*)&tint[0])) {
				changeFlag = true;
			}

			ImGui::Text("Threshold"); ImGui::SameLine(80.0f);
			if (ButtonEx::DragFloatNoLabel("threshold", &threshold, 0.001f, 0.001f, 1.0f, "%.3f", 1.0f)) {
				changeFlag = true;
			}

			ImGui::Text("Iteration"); ImGui::SameLine(80.0f);
			if (ButtonEx::DragFloatNoLabel("intensity", &intensity, 0.1f, 0, 0, "%.3f", 1.0f)) {
				changeFlag = true;
			}

			ImGui::Text("Stretch"); ImGui::SameLine(80.0f);
			if (ButtonEx::DragFloatNoLabel("stretch", &stretch, 0.01f, 0, 0, "%.3f", 1.0f)) {
				changeFlag = true;
			}

			ImGui::Text("Brightness"); ImGui::SameLine(80.0f);
			if (ButtonEx::DragFloatNoLabel("brightness", &brightness, 0.01f, 0, 0, "%.3f", 1.0f)) {
				changeFlag = true;
			}

			return false;
		}

		void ConfigureProps() override {
			shader->setFloat("threshold", threshold);
			shader->setFloat("intensity", intensity);
			shader->setFloat("stretch", stretch);
			shader->setFloat("brightness", brightness);
			shader->setVec3("tint", tint);
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

	class DepthOfField : public PostEffect {
	public:
		bool debug;
		bool bokeh;
		bool fastMode;

		// bokeh params
		float exposure;
		float threshold;
		float radius;
		float angle;

		// DOF params
		float iter;
		float multiply;
		float distance;
		float tolerance;
		float falloff;
		float cutoff;

		float accurate;

		DepthOfField() : PostProcessing("Depth-Of-Field", "DepthOfField") {
			debug = false;
			bokeh = false;
			fastMode = false;
			exposure = 1.8f;
			radius = 5.0f;
			angle = 2.39996f;

			distance = 0.9902f;
			multiply = 5.0f;
			tolerance = 0.004f;
			cutoff = 0.50f;

			iter = 5.0f;
			falloff = 10.0f;

			accurate = 20.0f;
		}
		~DepthOfField() override {}

		bool PostBehaviour(FrameBuffer* src, FrameBuffer* dst) override;

		bool ListProperties() override {
			bool changeFlag = false;

			ImGui::Text("Debug"); ImGui::SameLine(100.0f);
			if (ButtonEx::CheckboxNoLabel("debug", &debug)) {
				changeFlag = true;
			}

			ImGui::Text("Fast Mode"); ImGui::SameLine(100.0f);
			if (ButtonEx::CheckboxNoLabel("fast", &fastMode)) {
				changeFlag = true;
			}

			ImGui::Text("Bokeh"); ImGui::SameLine(100.0f);
			if (ButtonEx::CheckboxNoLabel("bokeh", &bokeh)) {
				changeFlag = true;
			}

			ImGui::Text("Distance"); ImGui::SameLine(80.0f);
			if (ButtonEx::DragFloatNoLabel("distance", &distance, 0.0001f, 0.0f, 0.0f, "%.4f")) {
				changeFlag = true;
			}

			ImGui::Text("Iteration"); ImGui::SameLine(80.0f);
			if (ButtonEx::DragFloatNoLabel("iter", &iter, 0.1f, 0.0f, 1024.0f, "%.4f")) {
				changeFlag = true;
			}

			ImGui::Text("Multiply"); ImGui::SameLine(80.0f);
			if (ButtonEx::DragFloatNoLabel("multiply", &multiply, 0.1f, 0.0f, 0.0f, "%.4f")) {
				changeFlag = true;
			}

			ImGui::Text("Tolerance"); ImGui::SameLine(80.0f);
			if (ButtonEx::DragFloatNoLabel("tolerance", &tolerance, 0.001f, 0.0f, 0.0f, "%.4f")) {
				changeFlag = true;
			}

			ImGui::Text("Falloff"); ImGui::SameLine(80.0f);
			if (ButtonEx::DragFloatNoLabel("falloff", &falloff, 0.01f, 0.0f, 0.0f, "%.4f")) {
				changeFlag = true;
			}

			ImGui::Text("Cutoff"); ImGui::SameLine(80.0f);
			if (ButtonEx::DragFloatNoLabel("cutoff", &cutoff, 0.1f, 0.0f, 0.0f, "%.4f")) {
				changeFlag = true;
			}

			ImGui::Text("Soft Edge"); ImGui::SameLine(80.0f);
			if (ButtonEx::DragFloatNoLabel("accurate", &accurate, 1.0f, 2.0f, 256.0f, "%.1f")) {
				changeFlag = true;
			}

			if (bokeh) {
				ImGui::Text("[Bokeh]");
				ImGui::Indent(10.0f);
				ImGui::Text("Radius"); ImGui::SameLine(80.0f);
				if (ButtonEx::DragFloatNoLabel("rad", &radius, 0.1f, 0.0f, 0.0f, "%.1f")) {
					changeFlag = true;
				}

				ImGui::Text("Exposure"); ImGui::SameLine(80.0f);
				if (ButtonEx::DragFloatNoLabel("expo", &exposure, 0.1f, 0.0f, 0.0f, "%.1f")) {
					changeFlag = true;
				}

				ImGui::Text("Angle"); ImGui::SameLine(80.0f);
				if (ButtonEx::DragFloatNoLabel("ang", &angle, 0.01f, 0.0f, 0.0f, "%.3f")) {
					changeFlag = true;
				}
				ImGui::Unindent(10.0f);
			}

			return changeFlag;
		}

		void ConfigureProps() override {
			shader->setBool("debug", debug);

			shader->setFloat("_distance", distance);
			shader->setFloat("_multiply", multiply);
			shader->setFloat("_tolerance", tolerance);
			shader->setFloat("_cutoff", cutoff);

			shader->setFloat("_time", radius);
			shader->setFloat("_expo", exposure);
			shader->setFloat("GOLDEN_ANGLE", angle);

			shader->setInt("_iter", iter);
			shader->setFloat("_falloff", falloff);

			shader->setFloat("_accurate", accurate);
		}
	};

	class Chromatic : public PostEffect {
	public:
		float fishEye;
		float chromatic;
		float scale;

		Chromatic() : PostProcessing("Chromatic", "Chromatic") {
			fishEye = 0.2f;
			chromatic = 1.0f;
			scale = 0.97f;
		}
		~Chromatic() override {}

		bool ListProperties() override {
			bool changeFlag = false;
			
			ImGui::Text("Fish Eye"); ImGui::SameLine(80.0f);
			if (ButtonEx::DragFloatNoLabel("fishEye", &fishEye, 0.1f, 0, 0, "%.3f", 1.0f)) {
				changeFlag = true;
			}

			ImGui::Text("Chromatic"); ImGui::SameLine(80.0f);
			if (ButtonEx::DragFloatNoLabel("chromatic", &chromatic, 0.1f, 0, 0, "%.3f", 1.0f)) {
				changeFlag = true;
			}

			ImGui::Text("Scale"); ImGui::SameLine(80.0f);
			if (ButtonEx::DragFloatNoLabel("scale", &scale, 0.1f, 0, 0, "%.3f", 1.0f)) {
				changeFlag = true;
			}

			return changeFlag;
		}

		void ConfigureProps() override {
			shader->setFloat("_fishEye", fishEye);
			shader->setFloat("_chromatic", chromatic);
			shader->setFloat("_scale", scale);
		}
	};

	class Vignette : public PostEffect {
	public:
		float weight;
		float shape;
		float smooth;

		Vignette() : PostProcessing("Vignette", "Vignette") {
			weight = 0.2f;
			shape = 2.0f;
			smooth = 0.8f;
		}
		~Vignette() override {}

		bool ListProperties() override {
			bool changeFlag = false;

			ImGui::Text("Weight"); ImGui::SameLine(80.0f);
			if (ButtonEx::DragFloatNoLabel("weight", &weight, 0.1f, 0.0f, 100.0f, "%.3f", 1.0f)) {
				changeFlag = true;
			}

			ImGui::Text("Shape"); ImGui::SameLine(80.0f);
			if (ButtonEx::DragFloatNoLabel("shape", &shape, 0.1f, 0.0f, 100.0f, "%.3f", 1.0f)) {
				changeFlag = true;
			}

			ImGui::Text("Smooth"); ImGui::SameLine(80.0f);
			if (ButtonEx::DragFloatNoLabel("smooth", &smooth, 0.1f, 0.0f, 100.0f, "%.3f", 1.0f)) {
				changeFlag = true;
			}

			return changeFlag;
		}

		void ConfigureProps() override {
			shader->setFloat("_weight", weight);
			shader->setFloat("_shape", shape);
			shader->setFloat("_smooth", smooth);
		}
	};

	class Blur : public PostEffect {
	public:
		unsigned int type;
		unsigned int iteration;
		float sigma;

		float exposure;
		float radius;
		float angle;

		Blur() : PostProcessing("Blur", "BlurEffects") {
			type = 2;
			iteration = 5;
			sigma = 7.0f;
			exposure = 1.8f;
			radius = 1.0f;
			angle = 2.39996f;
		}
		~Blur() override {}

		bool ListProperties() override {
			bool changeFlag = false;

			const static char* items[] = { "Box", "Gaussian", "Bokeh" };
			ImGui::AlignTextToFramePadding();
			ImGui::Text("Method"); ImGui::SameLine(80.0f);
			if (ButtonEx::ComboNoLabel("blurType", (int*)&type, items, IM_ARRAYSIZE(items))) {
				changeFlag = true;
			}

			ImGui::Text("Iteration"); ImGui::SameLine(80.0f);
			if (ButtonEx::DragIntNoLabel("iter", (int*)&iteration, 1.0f, 0, 64)) {
				changeFlag = true;
			}

			if (type == 1) {
				ImGui::Text("Sigma"); ImGui::SameLine(80.0f);
				if (ButtonEx::DragFloatNoLabel("sigma", &sigma, 0.1f, 0.0f, 256.0f, "%.1f")) {
					changeFlag = true;
				}
			}

			if (type == 2) {
				ImGui::Text("Radius"); ImGui::SameLine(80.0f);
				if (ButtonEx::DragFloatNoLabel("rad", &radius, 0.1f, 0.0f, 0.0f, "%.1f")) {
					changeFlag = true;
				}

				ImGui::Text("Exposure"); ImGui::SameLine(80.0f);
				if (ButtonEx::DragFloatNoLabel("expo", &exposure, 0.1f, 0.0f, 0.0f, "%.1f")) {
					changeFlag = true;
				}

				ImGui::Text("Angle"); ImGui::SameLine(80.0f);
				if (ButtonEx::DragFloatNoLabel("ang", &angle, 0.01f, 0.0f, 0.0f, "%.3f")) {
					changeFlag = true;
				}
			}

			return changeFlag;
		}

		void ConfigureProps() override {
			shader->setInt("_type", type);
			shader->setInt("_iter", iteration);
			shader->setFloat("_sigma", sigma);
			shader->setFloat("_time", radius);
			shader->setFloat("_expo", exposure);
			shader->setFloat("GOLDEN_ANGLE", angle);
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

	class Glitch : public PostEffect {
	public:

		Glitch() : PostProcessing("Glitch", "Glitch") {

		}
		~Glitch() override {}

		bool ListProperties() override {

			return false;
		}
		void ConfigureProps() override {

		}
	};

	class Halftone : public PostEffect {
	public:

		Halftone() : PostProcessing("Halftone", "Halftone") {

		}
		~Halftone() override {}

		bool ListProperties() override {

			return false;
		}
		void ConfigureProps() override {

		}
	};

	class RaindropFX : public PostEffect {
	public:

		RaindropFX() : PostProcessing("RaindropFX", "RaindropFX") {
			
		}
		~RaindropFX() override {}

		bool ListProperties() override {
			
			return false;
		}

		void ConfigureProps() override {

		}
	};

}