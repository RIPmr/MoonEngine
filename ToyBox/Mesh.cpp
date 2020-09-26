#include "Mesh.h"
#include "SceneMgr.h"
#include "Model.h"

//#define modelToWorld parent->transform.localToWorldMat.multVec

namespace MOON {
	void Mesh::Draw(Shader* shader, const Matrix4x4 & model, const bool &hovered = false, const bool &selected = false) {
		// bind appropriate textures
		unsigned int diffuseNr = 1;
		unsigned int specularNr = 1;
		unsigned int normalNr = 1;
		unsigned int heightNr = 1;
		unsigned int i;

		// shader configuration
		shader->use();

		shader->setMat4("model", model);
		shader->setMat4("view", MOON_ActiveCamera->view);
		shader->setMat4("projection", MOON_ActiveCamera->projection);

		shader->setVec3("lightColor", Vector3(1.0, 1.0, 1.0));
		shader->setVec3("lightPos", MOON_ActiveCamera->transform.position);
		shader->setVec3("viewPos", MOON_ActiveCamera->transform.position);
		shader->setBool("isHovered", hovered);
		shader->setBool("isSelected", selected);

		material->SetShaderProps(shader);

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

		// draw mesh
		if (!VAO) SetupMesh();
		//std::cout << VAO << std::endl;
		glBindVertexArray(VAO);
		glDrawElements(GL_TRIANGLES, triangles.size(), GL_UNSIGNED_INT, 0);
		glBindVertexArray(0);

		// always good practice to set everything back to defaults once configured.
		glActiveTexture(GL_TEXTURE0);
	}

	void Mesh::SetupMesh() {
		glGenVertexArrays(1, &VAO);
		glGenBuffers(1, &VBO);
		glGenBuffers(1, &EBO);

		UpdateMesh();

		// set the vertex attribute pointers
		// vertex Positions
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
		// vertex normals
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Normal));
		// vertex texture coords
		glEnableVertexAttribArray(2);
		glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, UV));
		// vertex tangent
		glEnableVertexAttribArray(3);
		glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Tangent));
		// vertex bitangent
		glEnableVertexAttribArray(4);
		glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Bitangent));

		glBindVertexArray(0);
	}

	void Mesh::GetSurfaceProperties(const Matrix4x4 modelMat, const uint32_t &triIndex, 
		const Vector2 &uv, Vector3 &hitNormal, Vector2 &hitTextureCoordinates) const {
		// face normal
		/*Vector3 &v0 = P[trisIndex[triIndex * 3]];
		Vector3 &v1 = P[trisIndex[triIndex * 3 + 1]];
		Vector3 &v2 = P[trisIndex[triIndex * 3 + 2]];
		hitNormal = (v1 - v0).cross(v2 - v0);
		hitNormal.normalize();*/

		// texture coordinates
		/*const Vector2 &st0 = texCoordinates[triIndex * 3];
		const Vector2 &st1 = texCoordinates[triIndex * 3 + 1];
		const Vector2 &st2 = texCoordinates[triIndex * 3 + 2];
		hitTextureCoordinates = (1 - uv.x - uv.y) * st0 + uv.x * st1 + uv.y * st2;*/

		// vertex normal
		Vector3 const &n0 = vertices[triangles[triIndex * 3]].Normal;
		Vector3 const &n1 = vertices[triangles[triIndex * 3 + 1]].Normal;
		Vector3 const &n2 = vertices[triangles[triIndex * 3 + 2]].Normal;

		hitNormal = (1 - uv.x - uv.y) * n0 + uv.x * n1 + uv.y * n2;
		hitNormal = modelMat.multDir(hitNormal);
		hitNormal.normalize();
		//std::cout << hitNormal << std::endl;
	}

	// Test if the ray interesests this triangle mesh
	bool Mesh::Intersect(const Matrix4x4 modelMat, const Ray &ray, float &tNear, uint32_t &triIndex, Vector2 &uv) const {
		bool isect = false;
		for (uint32_t i = 0, j = 0; i < triangles.size() / 3; ++i) {
			Vector3 const &v0 = modelMat.multVec(vertices[triangles[j]].Position);
			Vector3 const &v1 = modelMat.multVec(vertices[triangles[j + 1]].Position);
			Vector3 const &v2 = modelMat.multVec(vertices[triangles[j + 2]].Position);

			float t = tNear, u, v;
			if (MoonMath::RayTriangleIntersect(ray, v0, v1, v2, t, u, v)) {
				if (t > EPSILON && t < tNear) {
					tNear = t;
					uv.setValue(u, v);
					triIndex = i;
					isect = true;
				}
			}
			j += 3;
		}

		return isect;
	}
}

/*
*NOTE:
绑定多个纹理到对应的纹理单元后（最大32个纹理单元，0-31），需要定义哪个uniform采样器对应哪个纹理单元：
    注意，我们使用glform1i设置uniform采样器的位置值，或者说纹理单元。
    通过glUniform1i的设置，我们保证每个uniform采样器对应着正确的纹理单元。
*/