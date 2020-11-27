#include "SceneMgr.h"
#include "PostEffects.h"

namespace MOON {

	bool DepthOfField::PostBehaviour(FrameBuffer* src, FrameBuffer* dst) {
		if (fastMode) return false;

		Shader* passShader = MOON_ShaderManager::CreateShader("DOF_Pass", "ScreenBuffer.vs", "DOF_MultiPass.fs");

		// do three pass
		passShader->use();

		passShader->setBool("debug", debug);

		passShader->setFloat("_distance", distance);
		passShader->setFloat("_multiply", multiply);
		passShader->setFloat("_tolerance", tolerance);
		passShader->setFloat("_cutoff", cutoff);

		passShader->setFloat("_time", radius);
		passShader->setFloat("_expo", exposure);
		passShader->setFloat("_angle", angle);

		passShader->setFloat("_iter", iter);
		passShader->setFloat("_falloff", falloff);

		passShader->setTexture("depthBuffer", src->attachment, 2);
		passShader->setVec2("dir", 0.02, 0.02);
		Graphics::Blit(src, dst, passShader);
		passShader->setVec2("dir", 0.02, -0.02);
		Graphics::Blit(dst, passShader);
		passShader->setVec2("dir", 0.00, 0.04);
		Graphics::Blit(dst, passShader);

		//ConfigureProps();
		//Graphics::Blit(dst, shader);
		return true;
	}

}