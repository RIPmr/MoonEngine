#pragma once
#include <vector>
#include <string>

#include "MShader.h"
#include "Texture.h"
#include "Vector2.h"
#include "Vector3.h"
#include "Material.h"
#include "ObjectBase.h"
#include "MathUtils.h"

namespace moon {
	struct Vertex {
		Vector3 Position;
		Vector3 Normal;
		Vector2 TexCoords;
		Vector3 Tangent;
		Vector3 Bitangent;
	};

	class Mesh : public ObjectBase, public Hitable {
	public:
		std::vector<Vertex> vertices;
		std::vector<unsigned int> indices;
		Material* material;
		unsigned int VAO;

		Mesh() {}
		Mesh(const Mesh &mesh) : vertices(mesh.vertices), indices(mesh.indices),
			material(mesh.material), VAO(mesh.VAO) {
			//std::cout << "copy mat: " << mesh.material->name << std::endl;
		}
		Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> indices) : ObjectBase("Mesh", MOON_UNSPECIFIEDID) {
			this->vertices = vertices;
			this->indices = indices;

			// set the vertex buffers and its attribute pointers
			setupMesh();
		}
		Mesh(const std::string &name, std::vector<Vertex> vertices, std::vector<unsigned int> indices) : ObjectBase(name, MOON_UNSPECIFIEDID) {
			this->vertices = vertices;
			this->indices = indices;

			// set the vertex buffers and its attribute pointers
			setupMesh();
		}
		//~Mesh() { delete material; }
		~Mesh() override {}

		void Draw(Shader* shader, const Matrix4x4 & model);

		bool Hit(const Ray &r, float tmin, float tmax, HitRecord &rec) const {
			uint32_t triIndex;
			Vector2 uv;
			Vector3 hitNormal;
			Vector2 hitTextureCoordinates;
			float tNear = rec.t;

			if (Intersect(r, tNear, triIndex, uv)) {
				GetSurfaceProperties(triIndex, uv, hitNormal, hitTextureCoordinates);
				rec.t = tNear;
				rec.p = r.PointAtParameter(tNear);
				//rec.normal = Vector3::Normalize(rec.p);
				rec.normal = hitNormal;
				//rec.uv = hitTextureCoordinates;
				rec.mat = material;
				return true;
			}

			return false;
		}

	private:
		unsigned int VBO, EBO;

		// initializes all the buffer objects/arrays
		void setupMesh() {
			glGenVertexArrays(1, &VAO);
			glGenBuffers(1, &VBO);
			glGenBuffers(1, &EBO);

			glBindVertexArray(VAO);
			// load data into vertex buffers
			glBindBuffer(GL_ARRAY_BUFFER, VBO);
			// A great thing about structs is that their memory layout is sequential for all its items.
			// The effect is that we can simply pass a pointer to the struct and it translates perfectly to a Vector3/2 array which
			// again translates to 3/2 floats which translates to a byte array.
			glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);

			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), &indices[0], GL_STATIC_DRAW);

			// set the vertex attribute pointers
			// vertex Positions
			glEnableVertexAttribArray(0);
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)0);
			// vertex normals
			glEnableVertexAttribArray(1);
			glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Normal));
			// vertex texture coords
			glEnableVertexAttribArray(2);
			glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, TexCoords));
			// vertex tangent
			glEnableVertexAttribArray(3);
			glVertexAttribPointer(3, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Tangent));
			// vertex bitangent
			glEnableVertexAttribArray(4);
			glVertexAttribPointer(4, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, Bitangent));

			glBindVertexArray(0);
		}

		void GetSurfaceProperties(const uint32_t &triIndex, const Vector2 &uv, Vector3 &hitNormal, Vector2 &hitTextureCoordinates) const {
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
			Vector3 const &n0 = vertices[indices[triIndex * 3]].Normal;
			Vector3 const &n1 = vertices[indices[triIndex * 3 + 1]].Normal;
			Vector3 const &n2 = vertices[indices[triIndex * 3 + 2]].Normal;
			/*Vector3 &n0 = N[triIndex * 3];
			Vector3 &n1 = N[triIndex * 3 + 1];
			Vector3 &n2 = N[triIndex * 3 + 2];*/

			/*std::cout << "---------------------------------------" << std::endl;
			std::cout << n0.x << ", " << n0.y << ", " << n0.z << std::endl;
			std::cout << n1.x << ", " << n1.y << ", " << n1.z << std::endl;
			std::cout << n2.x << ", " << n2.y << ", " << n2.z << std::endl;*/

			hitNormal = (1 - uv.x - uv.y) * n0 + uv.x * n1 + uv.y * n2;
			hitNormal.normalize();
			//std::cout << hitNormal.x << ", " << hitNormal.y << ", " << hitNormal.z  << std::endl;
		}

		// Test if the ray interesests this triangle mesh
		bool Intersect(const Ray &ray, float &tNear, uint32_t &triIndex, Vector2 &uv) const {
			uint32_t j = 0;
			bool isect = false;
			for (uint32_t i = 0; i < indices.size() / 3; ++i) {
				Vector3 const &v0 = vertices[indices[j]].Position;
				Vector3 const &v1 = vertices[indices[j + 1]].Position;
				Vector3 const &v2 = vertices[indices[j + 2]].Position;

				float t = tNear, u, v;
				if (MoonMath::RayTriangleIntersect(ray, v0, v1, v2, t, u, v) && t < tNear) {
					if (t < INFINITY && t > EPSILON) {
						tNear = t;
						uv.x = u;
						uv.y = v;
						triIndex = i;
						isect = true;
					}
				}
				j += 3;
			}

			return isect;
		}
	};
}