#pragma once
#include <vector>
#include <string>

#include "BVH.h"
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

	extern class Mesh;
	class Triangle : public Hitable {
	public:
		Mesh* parent;
		unsigned int pa, pb, pc;

		Triangle(Mesh* parent, unsigned int& a, unsigned int& b, unsigned int& c);

		void UpdateBBox();

		bool Hit(const Ray &r, HitRecord &rec) const override;
	};

	extern class Model;
	class Mesh : public ObjectBase, public Hitable {
	public:
		//Model* parent;
		std::vector<Vertex> vertices;
		std::vector<unsigned int> triangles;
		unsigned int VAO;
		bool selected;

		Material* material;
		BVH* localBVH;
		std::vector<Hitable*> triList;

		Mesh() {}
		Mesh(const Mesh &mesh) : vertices(mesh.vertices), triangles(mesh.triangles), 
			material(mesh.material), VAO(mesh.VAO), selected(false), Hitable(mesh.bbox) {}
		Mesh(std::vector<Vertex> vertices, std::vector<unsigned int> triangles) : ObjectBase("Mesh", MOON_UNSPECIFIEDID), VAO(0) {
			this->vertices = vertices;
			this->triangles = triangles;
			this->selected = false;
			this->material = nullptr;
			this->localBVH = nullptr;

			// set the vertex buffers and its attribute pointers
			//setupMesh();
			UpdateBBox();
		}
		Mesh(const std::string &name, std::vector<Vertex> vertices, std::vector<unsigned int> triangles) : ObjectBase(name, MOON_UNSPECIFIEDID), VAO(0) {
			this->vertices = vertices;
			this->triangles = triangles;
			this->selected = false;
			this->material = nullptr;
			this->localBVH = nullptr;

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
			if (localBVH != nullptr) delete localBVH;
			Utility::ReleaseVector(triList);
		}

		virtual void BuildBVH() {
			if (localBVH != nullptr) delete localBVH;
			Utility::ReleaseVector(triList);

			// prepare triangles list
			for (int i = 0; i < triangles.size(); i += 3) {
				triList.push_back(
					new Triangle(this, 
						triangles[i], 
						triangles[i + 1], 
						triangles[i + 2]
					)
				);
			}

			// build BVH
			localBVH = new BVH(triList, Vector2(0, triList.size() - 1));
		}

		virtual void UpdateBBox() {
			for (auto &vert : vertices) {
				bbox.join(vert.Position);
			}
		}

		virtual void Draw(Shader* shader, const Matrix4x4 &model, const bool &hovered, const bool &selected);

		// * ray-mesh intersect calculated in local space
		bool Hit(const Ray &r, HitRecord &rec) const override {
			uint32_t triIndex; Vector2 uv; Vector3 hitNormal;

			if (Intersect(r, rec.t, triIndex, uv)) {
				GetSurfaceProperties(triIndex, uv, rec.normal, rec.uv);
				rec.p = r.PointAtParameter(rec.t);
				rec.mat = material;
				return true;
			}

			return false;
		}

		void GetSurfaceProperties(const uint32_t &triIndex, const Vector2 &uv, Vector3 &hitNormal, Vector2 &hitUV) const;
		void GetSurfaceProperties(const int& ta, const int& tb, const int& tc, const Vector2 &uv, Vector3 &hitNormal, Vector2 &hitUV) const;

	protected:
		unsigned int VBO, EBO;

	private:
		// initializes all the buffer objects/arrays
		virtual void SetupMesh();
		// Test if the ray interesests this triangle mesh
		bool Intersect(const Ray &ray, float &tNear, uint32_t &triIndex, Vector2 &uv) const;
	};
}