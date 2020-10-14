/*
*  _  _  _  _ ______    _  __    _  __ _  __ _
* |_)|_||_)|_) |  |    |_)|_ |\|| \|_ |_)|_ |_)
* | \| ||_)|_)_|_ |    | \|__| ||_/|__| \|__| \
*
* @author	HZT
* @date		2020-10-05
* @version	0.1.5
*/

#pragma once
#include <map>
#include <ctime>
#include <omp.h>
#include <conio.h>

#include "BVH.h"
#include "VFB.h"
#include "Vector2.h"
#include "Utility.h"
#include "Hitable.h"
#include "Texture.h"
#include "MatSphere.h"
#include "PostEffects.h"

namespace MOON {
	extern class Camera;
	class Renderer {
	public:
		enum AccStruct {
			acc_NONE,
			acc_AABB,
			acc_BVH,
			acc_BSP,
			acc_KDTree
		};

		// global settings
		static AccStruct acc;
		static TexFilter filter;
		static ToneMappingMethod tone;

		static bool depth;
		static bool motion;
		static float aspect;
		static Vector2 OUTPUT_SIZE;
		static GLubyte *matPrevImage;
		static GLfloat *outputRAW;
		static FrameBuffer* output;
		//static GLuint outputTexID;

		static Camera  matCamera;
		static Camera* targetCamera;

		static std::string timeStamp;

		static unsigned int samplingRate;
		static unsigned int maxReflectionDepth;

		// local params
		static clock_t start, end;
		static float progress;
		static bool isAbort;
		static bool prevInQueue;

		// functions
		static void StartRendering();

		static void* renderingMatPreview(void* args);
		static void* renderingTestImage(void* args);
		static void* rendering(void* args);

		static bool PrepareVFB();
		static bool PrepareRendering();
		static bool PrepareMatPrevRendering(Texture* target);
		static void SetOutputSize(unsigned int width, unsigned int height);

		static void Clear() {
			VFB::Clear();
			if (output != nullptr) delete output;
		}

	private:
		static void UpdateTimeStamp();
		static Vector3 SamplingColor(const Ray &r, int depth);
		static Vector3 SamplingColor_Simple(const Ray &r, int depth, const MSphere* ball, const MSphere* ground);
		
		// environment sampling
		static Vector3 SimpleSky(const Ray &r);
		static Vector3 SampleSphericalMap(const Ray &r);
		static Vector3 PureBackground(const Ray &r, const Vector4 &c);
	};
}