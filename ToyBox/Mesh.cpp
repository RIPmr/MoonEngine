#include "Mesh.h"
#include "SceneMgr.h"

namespace moon {
	void Mesh::Draw(Shader* shader, const Matrix4x4 & model) {
		// bind appropriate textures
		unsigned int diffuseNr = 1;
		unsigned int specularNr = 1;
		unsigned int normalNr = 1;
		unsigned int heightNr = 1;
		unsigned int i;

		// binding textures
		// TODO
		//for (i = 0; i < textures.size(); i++) {
		//	// active proper texture unit before binding
		//	glActiveTexture(GL_TEXTURE0 + i);
		//	// retrieve texture number (the N in diffuse_textureN)
		//	std::string number;
		//	if (textures[i].type == TexType::diffuse)
		//		number = std::to_string(diffuseNr++);
		//	else if (textures[i].type == TexType::specular)
		//		number = std::to_string(specularNr++);
		//	else if (textures[i].type == TexType::normal)
		//		number = std::to_string(normalNr++);
		//	else if (textures[i].type == TexType::height)
		//		number = std::to_string(heightNr++);

		//	// now set the sampler to the correct texture unit
		//	glUniform1i(glGetUniformLocation(shader->localID, (name + number).c_str()), i);
		//	// and finally bind the texture
		//	glBindTexture(GL_TEXTURE_2D, textures[i].localID);
		//}

		// shader configuration
		shader->use();

		shader->setMat4("model", model);
		shader->setMat4("view", SceneManager::CameraManager::currentCamera->view);
		shader->setMat4("projection", SceneManager::CameraManager::currentCamera->projection);

		shader->setVec3("lightColor", Vector3(1.0, 1.0, 1.0));
		shader->setVec3("objectColor", dynamic_cast<MoonMtl*>(material)->Kd);
		shader->setVec3("lightPos", SceneManager::CameraManager::currentCamera->transform.position);
		shader->setVec3("viewPos", SceneManager::CameraManager::currentCamera->transform.position);

		// draw mesh
		glBindVertexArray(VAO);
		glDrawElements(GL_TRIANGLES, indices.size(), GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);

		// always good practice to set everything back to defaults once configured.
		glActiveTexture(GL_TEXTURE0);
	}
}