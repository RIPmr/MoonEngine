#pragma once
#include <imgui.h>
#include "Icons.h"
#include "Texture.h"
#include "PostEffects.h"
#include "StackWindow.h"

namespace MOON {

	class VFB {
	public:
		static bool show;
		static bool show_history;
		static bool show_postfx;

		static std::vector<PostEffect*> postStack;

		static void Draw();
		static void VFB_Main();
		static void VFB_History();
		static void VFB_PostFX();

		static void UpdateOutput();

		static void Clear() {
			if (postStack.size()) Utility::ReleaseVector(postStack);
		}

		static void InitPostStack() {
			postStack = std::vector<PostEffect*>{
				new ColorSpaceConverter(),
				new Exposure(),
				new ColorCorrection(),
				new Levels(),
				new Curve(),
				new Bloom(),
				new Flare(),
				new ToneMapping()
			};
		}
	};

}