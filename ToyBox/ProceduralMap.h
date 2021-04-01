#pragma once
#include <string>
//#include <cstdarg>

#include "Texture.h"
#include "Vector2.h"
#include "Vector3.h"
#include "Vector4.h"
#include "Color.h"

namespace MOON {
	#define ProceduralTexture ProceduralMapGenerator::ProceduralMapType

	/* CPU & GPU based procedural map generator */
	class ProceduralMapGenerator {
	public:
		enum ProceduralMapType {
			// noise
			WhiteNoise, FirstNoise = WhiteNoise,
			ValueNoise,
			PerlinNoise,
			OctavePerlin,
			SimplexNoise,
			WorleyNoise,
			VoronoiNoise,
			TysonPolygon,
			CloudNoise,

			// pattern
			Checker,
			Disco,
			WoodPattern,
			BubblePattern, FirstPattern = BubblePattern,
			DigitalStorm,
			TruchetPattern,
			ColorfulTruchet
		};

	#pragma region GPU_based
		static Shader* FireShader;
		static Shader* NoiseShader;
		static Shader* PatternShader;

		static void LoadShaders();

		/*static bool GPUNoiseMaker(FrameBuffer* texHolder, const ProceduralMapType& type,
			const unsigned int channelSelection, ...) {
			va_list arg_ptr;
			va_start(arg_ptr, type);
			GPUNoiseMaker(
				type, texHolder, 
				va_arg(arg_ptr, Vector2), 
				va_arg(arg_ptr, float),
				va_arg(arg_ptr, float), 
				va_arg(arg_ptr, float),
				channelSelection
			);
			va_end(arg_ptr);
			return true;
		}*/

		static bool GPUFireTexMaker(FrameBuffer* texHolder, const Vector2& offset, 
			const float& scale, const float& power, const float& time, const Vector3& tint);

		static bool GPUNoiseMaker(const ProceduralMapType& type, 
			FrameBuffer* texHolder, const Vector2& offset = Vector2::ZERO(),
			const float& scale = 1.0f, const float& dim = 0.0f, const float& time = 0.0f,
			const unsigned int channelSelection = 0);

		static bool GPUPatternMaker(const ProceduralMapType& type,
			FrameBuffer* texHolder, const Vector2& offset = Vector2::ZERO(),
			const float& scale = 1.0f, const float& dim = 0.0f, const float& time = 0.0f,
			const unsigned int channelSelection = 0);

		static bool GPUChecker(FrameBuffer* texHolder, const Vector3& offset = Vector2::ZERO(),
			const float& scale = 1.0f);

		static bool GPUWhiteNoise(FrameBuffer* texHolder, const Vector3& offset = Vector2::ZERO(),
			const float& scale = 1.0f);

		static bool GPUPerlinNoise(FrameBuffer* texHolder, const Vector2& offset = Vector2::ZERO(),
			const float& scale = 1.0f, const float& dim = 0.0f, const float& time = 0.0f);
		static bool GPUPerlinNoise(FrameBuffer* texHolder, const Vector4& offset = Vector4::ZERO(),
			const float& scale = 1.0f) {
			GPUPerlinNoise(texHolder, Vector2(offset.x, offset.y), scale, offset.z, offset.w);
		}

		static bool GPUPerlinNoise_fbm(FrameBuffer* texHolder, const Vector2& offset = Vector2::ZERO(),
			const float& scale = 1.0f, const float& dim = 0.0f, const float& time = 0.0f);

		static bool GPUSimplexNoise(FrameBuffer* texHolder, const Vector2& offset = Vector2::ZERO(),
			const float& scale = 1.0f, const float& dim = 0.0f, const float& time = 0.0f);
		static bool GPUSimplexNoise(FrameBuffer* texHolder, const Vector4& offset = Vector4::ZERO(),
			const float& scale = 1.0f) {
			GPUSimplexNoise(texHolder, Vector2(offset.x, offset.y), scale, offset.z, offset.w);
		}

		static bool GPUWorleyNoise(FrameBuffer* texHolder, const Vector2& offset = Vector2::ZERO(),
			const float& scale = 1.0f, const float& dim = 0.0f, const float& time = 0.0f);

		static bool GPUVoronoiNoise(FrameBuffer* texHolder, const Vector2& offset = Vector2::ZERO(),
			const float& scale = 1.0f, const float& dim = 0.0f, const float& time = 0.0f);
	#pragma endregion

	#pragma region CPU_based
		static bool CPUNoiseMaker(GLubyte *targetImage, const Vector2 &mapSize, const ProceduralMapType& type) {
			if (type == Checker) {
				return CPUCheckImage(targetImage, mapSize);
			} else if (type == WhiteNoise) {

			} else if (type == ValueNoise) {

			}
			return false;
		}

		static bool CPUWhiteNoise(GLubyte *checkImage, const Vector2 &mapSize);
		static bool CPUValueNoise(GLubyte *checkImage, const Vector2 &mapSize);
		static bool CPUPerlinNoise(GLubyte *checkImage, const Vector2 &mapSize);
		static bool CPUWorleyNoise(GLubyte *checkImage, const Vector2 &mapSize);
		static bool CPUVoronoiNoise(GLubyte *checkImage, const Vector2 &mapSize);
		static bool CPUCheckImage(GLubyte *checkImage, const Vector2 &mapSize);
		static bool CPUBlankImage(GLubyte *targetImage);
	#pragma endregion
	};
}

/*
*NOTE:
（1）va_list
定义了一个指针arg_ptr, 用于指示可选的参数.

（2）va_start(arg_ptr, argN)
使参数列表指针arg_ptr指向函数参数列表中的第一个可选参数，
argN是位于第一个可选参数之前的固定参数, 或者说最后一个固定参数（至少要有一个固定参数）.
如:
有一va函数的声明是void va_test(char a, char b, char c, ...), 
则它的固定参数依次是a,b,c, 最后一个固定参数argN为c, 因此就是va_start(arg_ptr, c).

（3）va_arg(arg_ptr, type)
返回参数列表中指针arg_ptr所指的参数, 返回类型为type. 
并使指针arg_ptr指向参数列表中下一个参数.
*返回的是可选参数, 不包括固定参数.

（4）va_end(arg_ptr)
清空参数列表, 并置参数指针arg_ptr无效.

* 取值超出传入时不会出错，但取得随机值
*/
