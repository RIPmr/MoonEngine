#include "Mesh.h"
#include "SceneMgr.h"
#include "Model.h"

//#define modelToWorld parent->transform.localToWorldMat.multVec

namespace MOON {
#pragma region Triangle_implementation
	Triangle::Triangle(Mesh* parent, unsigned int& a, unsigned int& b, unsigned int& c) {
		this->parent = parent;
		pa = a; pb = b; pc = c;
		UpdateBBox();
	}
	
	void Triangle::UpdateBBox() {
		bbox.Reset();
		bbox.join(parent->vertices[pa].Position);
		bbox.join(parent->vertices[pb].Position);
		bbox.join(parent->vertices[pc].Position);
	}

	// * ray-tri intersect calculated test in local space
	bool Triangle::Hit(const Ray &r, HitRecord &rec) const {
		Vector2 uv; float t;
		if (MoonMath::RayTriangleIntersect(r, parent->vertices[pa].Position,
			parent->vertices[pb].Position, parent->vertices[pc].Position, t, uv.x, uv.y)) {
			if (t > EPSILON && t < rec.t) {
				parent->GetSurfaceProperties(pa, pb, pc, uv, rec.normal, rec.uv);
				rec.t = t;
				rec.p = r.PointAtParameter(rec.t);
				rec.mat = parent->material;
				return true;
			}
		}

		return false;
	}
#pragma endregion

#pragma region Mesh_implementation
	void Mesh::Draw(Shader* shader, const Matrix4x4 & model, const bool &hovered, const bool &selected) {
		if (shader == NULL) shader = material->shader;
		
		// shader configuration
		shader->use();

		// basic pros ----------------------------------------------------
		shader->setMat4("model", model);
		shader->setMat4("view", MOON_ActiveCamera->view);
		shader->setMat4("projection", MOON_ActiveCamera->projection);

		shader->setVec3("viewPos", MOON_ActiveCamera->transform.position);
		shader->setBool("isHovered", hovered);
		shader->setBool("isSelected", selected);

		// lights --------------------------------------------------------
		shader->setInt("lightNum", MOON_LightManager::CountItem());
		auto end = MOON_LightManager::itemMap.end(); unsigned int i = 0;
		for (auto it = MOON_LightManager::itemMap.begin(); it != end; it++) {
			shader->setVec3("lightPositions[" + std::to_string(i) + "]", it->second->transform.position);
			shader->setVec3("lightColors[" + std::to_string(i++) + "]", it->second->color * it->second->power);
		}

		// legacy viewport light
		shader->setVec3("lightColor", Vector3(1.0, 1.0, 1.0));
		shader->setVec3("lightPos", MOON_ActiveCamera->transform.position);

		// material: local prop configuration
		if (material != nullptr) material->SetShaderProps(shader);
		//else std::cout << "[Warning]: no material: " << this->name << std::endl;

		// draw mesh
		if (!VAO) SetupMesh();
		glBindVertexArray(VAO);
		glDrawElements(GL_TRIANGLES, triangles.size(), GL_UNSIGNED_INT, 0);

		// set everything back to defaults once configured
		glBindVertexArray(0);
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

	// triIndex: start id of the triangle
	void Mesh::GetSurfaceProperties(const uint32_t &triIndex, const Vector2 &uv, 
		Vector3 &hitNormal, Vector2 &hitUV) const {
		GetSurfaceProperties(triangles[triIndex * 3], triangles[triIndex * 3 + 1],
			triangles[triIndex * 3 + 2], uv, hitNormal, hitUV);
	}
	void Mesh::GetSurfaceProperties(const int& ta, const int& tb, const int& tc, 
		const Vector2 &uv, Vector3 &hitNormal, Vector2 &hitUV) const {
		// texture coordinates
		/*const Vector2 &st0 = texCoordinates[triIndex * 3];
		const Vector2 &st1 = texCoordinates[triIndex * 3 + 1];
		const Vector2 &st2 = texCoordinates[triIndex * 3 + 2];
		hitTextureCoordinates = (1 - uv.x - uv.y) * st0 + uv.x * st1 + uv.y * st2;*/

		// flat normal
		/*Vector3 &v0 = P[trisIndex[triIndex * 3]];
		Vector3 &v1 = P[trisIndex[triIndex * 3 + 1]];
		Vector3 &v2 = P[trisIndex[triIndex * 3 + 2]];
		hitNormal = (v1 - v0).cross(v2 - v0);
		hitNormal.normalize();*/

		// smooth normal
		Vector3 const &n0 = vertices[ta].Normal;
		Vector3 const &n1 = vertices[tb].Normal;
		Vector3 const &n2 = vertices[tc].Normal;

		hitNormal = (1 - uv.x - uv.y) * n0 + uv.x * n1 + uv.y * n2;
	}

	// Test if the ray interesests this triangle mesh
	bool Mesh::Intersect(const Ray &ray, float &tNear, uint32_t &triIndex, Vector2 &uv) const {
		bool isect = false;
		for (uint32_t i = 0, j = 0; i < triangles.size() / 3; ++i) {
			/*Vector3 const &v0 = modelMat.multVec(vertices[triangles[j]].Position);
			Vector3 const &v1 = modelMat.multVec(vertices[triangles[j + 1]].Position);
			Vector3 const &v2 = modelMat.multVec(vertices[triangles[j + 2]].Position);*/

			float t, u, v;
			if (MoonMath::RayTriangleIntersect(ray, vertices[triangles[j]].Position, 
				vertices[triangles[j + 1]].Position, vertices[triangles[j + 2]].Position, t, u, v)) {
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
#pragma endregion

}

/*
*NOTE:
绑定多个纹理到对应的纹理单元后（最大32个纹理单元，0-31），需要定义哪个uniform采样器对应哪个纹理单元：
    注意，我们使用glform1i设置uniform采样器的位置值，或者说纹理单元。
    通过glUniform1i的设置，我们保证每个uniform采样器对应着正确的纹理单元。
*/