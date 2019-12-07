#pragma once
#include "MoonEnums.h"

namespace MOON {

	class PipelineManager {
	public:
		static ShadingMode mode;

		static void DrawDeferredShading();

	private:
		static unsigned int quadVAO, quadVBO;

		// vertex attributes for a quad that fills the 
		// entire screen in Normalized Device Coordinates.
		static float quadVertices[24];

		// config screen quad VAO
		static void ConfigureScreenQuad();
	};

}