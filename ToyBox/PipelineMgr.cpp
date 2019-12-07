#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "PipelineMgr.h"
#include "SceneMgr.h"

namespace MOON {

	void PipelineManager::DrawDeferredShading() {
		glDisable(GL_DEPTH_TEST);
		MOON_ShaderManager::screenBufferShader->use();
		glBindVertexArray(quadVAO);
		glBindTexture(GL_TEXTURE_2D, MOON_TextureManager::IDLUT->localID);
		glDrawArrays(GL_TRIANGLES, 0, 6);
	}

	void PipelineManager::ConfigureScreenQuad() {
		glGenVertexArrays(1, &quadVAO);
		glGenBuffers(1, &quadVBO);
		glBindVertexArray(quadVAO);
		glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
	}

}