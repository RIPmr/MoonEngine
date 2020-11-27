#include "ProceduralMap.h"
#include "MShader.h"
#include "Graphics.h"
#include "SceneMgr.h"

namespace MOON {

#pragma region init_function
	void ProceduralMapGenerator::LoadShaders() {
		if (NoiseShader == nullptr) {
			NoiseShader = new Shader("NoiseShader", "ScreenBuffer.vs", "NoiseGenerator.fs");
			MOON_ShaderManager::AddItem(NoiseShader);
		}
	}
#pragma endregion

#pragma region GPU_Texture_Generator
	bool ProceduralMapGenerator::GPUMapMaker(const ProceduralMapType& type,
		FrameBuffer* texHolder, const Vector2& offset, const float& scale, 
		const float& dim, const float& time, const unsigned int channelSelection) {
		LoadShaders();

		NoiseShader->use();
		NoiseShader->setInt("noiseType", type);
		NoiseShader->setFloat("noiseScale", scale);
		NoiseShader->setVec2("offset", offset);
		NoiseShader->setFloat("dim", dim);
		NoiseShader->setFloat("time", time);
		NoiseShader->setInt("channelSelection", channelSelection);

		Graphics::Blit(texHolder, texHolder, NoiseShader);
		return true;
	}

	bool ProceduralMapGenerator::GPUWhiteNoise(FrameBuffer* texHolder, 
		const Vector3& offset, const float& scale) {

		return true;
	}

	bool ProceduralMapGenerator::GPUPerlinNoise(FrameBuffer* texHolder, 
		const Vector2& offset, const float& scale, const float& dim, const float& time) {

		return true;
	}

	bool ProceduralMapGenerator::GPUPerlinNoise_fbm(FrameBuffer* texHolder, 
		const Vector2& offset, const float& scale, const float& dim, const float& time) {

		return true;
	}

	bool ProceduralMapGenerator::GPUSimplexNoise(FrameBuffer* texHolder, 
		const Vector2& offset, const float& scale, const float& dim, const float& time) {

		return true;
	}

	bool ProceduralMapGenerator::GPUWorleyNoise(FrameBuffer* texHolder, 
		const Vector2& offset, const float& scale, const float& dim, const float& time) {

		return true;
	}

	bool ProceduralMapGenerator::GPUVoronoiNoise(FrameBuffer* texHolder, 
		const Vector2& offset, const float& scale, const float& dim, const float& time) {

		return true;
	}
#pragma endregion

#pragma region CPU_Texture_Generator
	bool ProceduralMapGenerator::CPUWhiteNoise(GLubyte *checkImage, const Vector2 &mapSize) {

		return true;
	}

	bool ProceduralMapGenerator::CPUValueNoise(GLubyte *checkImage, const Vector2 &mapSize) {

		return true;
	}

	bool ProceduralMapGenerator::CPUPerlinNoise(GLubyte *checkImage, const Vector2 &mapSize) {

		return true;
	}

	bool ProceduralMapGenerator::CPUWorleyNoise(GLubyte *checkImage, const Vector2 &mapSize) {

		return true;
	}

	bool ProceduralMapGenerator::CPUCheckImage(GLubyte *checkImage, const Vector2 &mapSize) {
		for (int i = 0, c, cnt = 0; i < mapSize.x; i++) {
			for (int j = 0; j < mapSize.y; j++) {
				c = ((((i & 0x8) == 0) ^ ((j & 0x8)) == 0)) * 255;
				checkImage[cnt++] = (GLubyte)c;
				checkImage[cnt++] = (GLubyte)c;
				checkImage[cnt++] = (GLubyte)c;
			}
		}
		return true;
	}

	bool ProceduralMapGenerator::CPUBlankImage(GLubyte *targetImage) {
		memset(targetImage, (GLubyte)0, sizeof(targetImage));
		return true;
	}
#pragma endregion

}