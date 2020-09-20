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
#include "BoundingBox.h"

namespace MOON {
	struct Edge; struct Face;

	struct Vertex {
		Vector3 Position;
		Vector3 Normal;
		Vector3 Tangent;
		Vector3 Bitangent;
		Vector2 UV;

		unsigned int ID;
		Vector4  Color;
		Edge* v_edge; // 以该顶点为首顶点的出边(任意一条)
		Face* v_face; // 包含该顶点的面(任意一面)

		bool selected;

		Vertex() {
			v_edge = nullptr;
			v_face = nullptr;
		}

		bool operator==(Vertex& o) {
			if (ID == o.ID) return true;
			else return false;
		}
	};

	extern class Model;
	class Mesh : public ObjectBase, public Hitable {
	public:
		std::vector<Vertex> vertices;
		std::vector<unsigned int> triangles;
		//Model* parent;
		Material* material;
		BoundingBox bbox;
		unsigned int VAO;

		Mesh() {}
		Mesh(const Mesh &mesh) : vertices(mesh.vertices), triangles(mesh.triangles), material(mesh.material), VAO(mesh.VAO), bbox(mesh.bbox) {}
		Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> triangles) : ObjectBase("Mesh", MOON_UNSPECIFIEDID), VAO(0) {
			this->vertices = vertices;
			this->triangles = triangles;

			// set the vertex buffers and its attribute pointers
			//setupMesh();
			UpdateBBox();
		}
		Mesh(const std::string &name, std::vector<Vertex> vertices, std::vector<unsigned int> triangles) : ObjectBase(name, MOON_UNSPECIFIEDID), VAO(0) {
			this->vertices = vertices;
			this->triangles = triangles;

			// set the vertex buffers and its attribute pointers
			//setupMesh();
			UpdateBBox();
		}

		virtual void UpdateMesh() {
			glBindVertexArray(VAO);
			// load data into vertex buffers
			glBindBuffer(GL_ARRAY_BUFFER, VBO);
			// A great thing about structs is that their memory layout is sequential for all its items.
			// The effect is that we can simply pass a pointer to the struct and it translates perfectly to a Vector3/2 array which
			// again translates to 3/2 floats which translates to a byte array.
			glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_STATIC_DRAW);

			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, triangles.size() * sizeof(unsigned int), &triangles[0], GL_STATIC_DRAW);
		}

		virtual ~Mesh() override {
			if (VAO) {
				glDeleteBuffers(1, &EBO);
				glDeleteBuffers(1, &VBO);
				glDeleteVertexArrays(1, &VAO);
			}
		}

		virtual void UpdateBBox() {
			for (auto &vert : vertices) {
				bbox.join(vert.Position);
			}
		}

		virtual void Draw(Shader* shader, const Matrix4x4 &model, const bool &hovered, const bool &selected);

		bool Hit(const Ray &r, HitRecord &rec) const {
			return Hit(Matrix4x4::identity(), r, rec);
		}

		bool Hit(const Matrix4x4 modelMat, const Ray &r, HitRecord &rec) const {
			uint32_t triIndex;
			Vector2 uv;
			Vector3 hitNormal;
			Vector2 hitTextureCoordinates;

			if (Intersect(modelMat, r, rec.t, triIndex, uv)) {
				GetSurfaceProperties(modelMat, triIndex, uv, hitNormal, hitTextureCoordinates);
				rec.p = r.PointAtParameter(rec.t);
				//rec.normal = Vector3::Normalize(rec.p);
				rec.normal = hitNormal;
				//rec.uv = hitTextureCoordinates;
				rec.mat = material;
				return true;
			}

			return false;
		}

	protected:
		unsigned int VBO, EBO;

	private:
		// initializes all the buffer objects/arrays
		virtual void SetupMesh();
		void GetSurfaceProperties(const Matrix4x4 modelMat, const uint32_t &triIndex, const Vector2 &uv, Vector3 &hitNormal, Vector2 &hitTextureCoordinates) const;
		// Test if the ray interesests this triangle mesh
		bool Intersect(const Matrix4x4 modelMat, const Ray &ray, float &tNear, uint32_t &triIndex, Vector2 &uv) const;
	};
}