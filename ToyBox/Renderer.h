#pragma once
#include <map>
#include <ctime>
#include <omp.h>
#include <conio.h>

#include "Vector2.h"
#include "Utility.h"
#include "Hitable.h"

namespace MOON {
	extern class Camera;
	class Renderer {
	public:
		// global settings
		static Vector2 OUTPUT_SIZE;
		static float aspect;
		static GLubyte *outputImage;
		static GLuint outputTexID;

		static Camera* targetCamera;

		static unsigned int samplingRate;
		static unsigned int maxReflectionDepth;

		// local params
		static clock_t start, end;
		static float progress;
		static bool isAbort;

		// functions
		static void* renderingTestImage(void* args);
		static void* rendering(void* args);
		static bool PrepareVFB();
		static bool PrepareRendering();
		static void SetOutputSize(unsigned int width, unsigned int height);

	private:
		static Vector3 SamplingColor(const Ray &r, int depth);
		static Vector3 SimpleSky(const Ray &r);
	};
}