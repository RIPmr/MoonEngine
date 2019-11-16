#pragma once
#include <ctime>

#include <omp.h>
#include <conio.h>
#include "Vector2.h"
#include "Utility.h"

namespace moon {
	class Renderer {
	public:
		// global settings
		static Vector2 OUTPUT_SIZE;
		static GLubyte *outputImage;
		static GLuint outputTexID;

		// local params
		static clock_t start, end;
		static float progress;
		static bool isAbort;

		// functions
		static void* rendering(void* args);
		static bool PrepareVFB();
		static bool PrepareRendering();
	};
}