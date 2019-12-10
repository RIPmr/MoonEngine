/*
*  _  _  _  _ ______    _  __    _  __ _  __ _
* |_)|_||_)|_) |  |    |_)|_ |\|| \|_ |_)|_ |_)
* | \| ||_)|_)_|_ |    | \|__| ||_/|__| \|__| \
*
* @author	HZT
* @date		2019-12-06
* @version	0.1.0
*/

#pragma once
#include <map>
#include <ctime>
#include <omp.h>
#include <conio.h>

#include "Vector2.h"
#include "Utility.h"
#include "Hitable.h"
#include "Texture.h"
#include "MatSphere.h"

namespace MOON {
	extern class Camera;
	class Renderer {
	public:
		// global settings
		static Vector2 OUTPUT_SIZE;
		static float aspect;
		static GLubyte *matPrevImage;
		static GLubyte *outputImage;
		static GLuint outputTexID;

		static Camera  matCamera;
		static Camera* targetCamera;

		static unsigned int samplingRate;
		static unsigned int maxReflectionDepth;

		// local params
		static clock_t start, end;
		static float progress;
		static bool isAbort;
		static bool prevInQueue;

		// functions
		static void* renderingMatPreview(void* args);
		static void* renderingTestImage(void* args);
		static void* rendering(void* args);
		static bool PrepareVFB();
		static bool PrepareRendering();
		static bool PrepareMatPrevRendering(Texture* target);
		static void SetOutputSize(unsigned int width, unsigned int height);

	private:
		static Vector3 SamplingColor(const Ray &r, int depth);
		static Vector3 SamplingColor_Simple(const Ray &r, int depth, const Sphere* ball, const Sphere* ground);
		static Vector3 SimpleSky(const Ray &r);
	};
}