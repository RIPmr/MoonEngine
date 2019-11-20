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
#include "Matrix4x4.h"

namespace moon {
	struct Vertex {
		Vector3 Position;
		Vector3 Normal;
		Vector2 TexCoords;
		Vector3 Tangent;
		Vector3 Bitangent;
	};

	extern class Model;
	class Mesh : public ObjectBase, public Hitable {
	public:
		std::vector<Vertex> vertices;
		std::vector<unsigned int> indices;
		Model* parent;
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
		void setupMesh();
		void GetSurfaceProperties(const uint32_t &triIndex, const Vector2 &uv, Vector3 &hitNormal, Vector2 &hitTextureCoordinates) const;
		// Test if the ray interesests this triangle mesh
		bool Intersect(const Ray &ray, float &tNear, uint32_t &triIndex, Vector2 &uv) const;
	};
}