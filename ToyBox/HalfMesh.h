#pragma once
#include "Mesh.h"
#include "Gizmo.h"
#include "Vector4.h"
#include "Utility.h"

#include <vector>
#include <unordered_map>
#include <iostream>

namespace MOON {

	struct Edge {
		Edge* e_pair;		// 对偶边
		Edge* e_succ;		// 后继边
		Face* e_face;		// 右侧面
		Vertex* e_vert;	    // 首顶点

		bool selected;

		Edge() {
			e_pair = nullptr;
			e_succ = nullptr;
			e_face = nullptr;
			e_vert = nullptr;
		}
	};

	struct Face {
		unsigned int ID;
		std::vector<Vertex*> f_verts;
		Edge* f_edge;

		bool selected;

		Face() {
			f_edge = nullptr;
		}
	};

	// half-edge data structure
	class HalfMesh : public Mesh {
	public:
	#pragma region variables
		std::vector<Edge> edges;
		std::vector<Face> faces;

		/*#define vpair std::pair<Vertex*, Vertex*>
		vpair vert_pair(Vertex* v1, Vertex* v2) {
			return std::make_pair(v1, v2);
		}*/
		unsigned int Pair(Vertex* v1, Vertex* v2) const {
			return v1->ID * 10 + v2->ID;
		}
		std::unordered_map<unsigned int, Edge*> m_emap;

	#pragma endregion

	#pragma region constructors
		HalfMesh(Mesh* mesh) {
			ConvertFromMesh(mesh);
		}

		void UpdateMesh() override {
			glBindVertexArray(VAO);
			glBindBuffer(GL_ARRAY_BUFFER, VBO);
			glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), &vertices[0], GL_DYNAMIC_DRAW);

			glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
			glBufferData(GL_ELEMENT_ARRAY_BUFFER, triangles.size() * sizeof(unsigned int), &triangles[0], GL_DYNAMIC_DRAW);
		}

		void ConvertFromMesh(Mesh* mesh) {
			// get properties
			this->material = mesh->material;
			this->bbox = mesh->bbox;

			// convert vertex
			this->vertices = mesh->vertices;
			for (int i = 0; i < mesh->vertices.size(); i++) {
				mesh->vertices[i].ID = i;
			}

			// convert faces
			this->triangles = mesh->triangles;
			for (int i = 0; i < mesh->triangles.size(); i += 3) {
				InsertFace({ 
					&vertices[mesh->triangles[i]],
					&vertices[mesh->triangles[i + 1]],
					&vertices[mesh->triangles[i + 2]]
				});
			}
		}
	#pragma endregion

	#pragma region insert_delete_data
		void InsertVertex(const Vector3& pos, const Vector3& norm, const Vector2& uv = Vector2::ZERO(),
			const Vector3& tangent = Vector3::ZERO(), const Vector3& bitangent = Vector3::ZERO()) {
			Vertex vert;
			vert.Position = pos;
			vert.Normal = norm;
			vert.UV = uv;
			vert.Tangent = tangent;
			vert.Bitangent = bitangent;
			vert.ID = vertices.size();
			vertices.push_back(vert);
		}

		void DeleteVertex(Vertex& vert) {
			Utility::RemoveElem(vertices, vert);


		}

		Edge& InsertEdge(Vertex& v1, Vertex& v2) {
			if (m_emap[Pair(&v1, &v2)] != NULL) return *m_emap[Pair(&v1, &v2)];

			Edge edge;
			edge.e_vert = &v1;
			v1.v_edge = &edge;

			Edge p_edge;
			p_edge.e_vert = &v2;
			v2.v_edge = &p_edge;

			// make pair
			edge.e_pair = &p_edge;
			p_edge.e_pair = &edge;
			m_emap[Pair(&v1, &v2)] = &edge;
			m_emap[Pair(&v2, &v1)] = &p_edge;

			edges.push_back(edge);

			return edge;
		}

		void DeleteEdge(Edge& edge) {

		}

		void InsertFace(std::vector<Vertex*> face_v) {
			if (face_v.empty()) return;

			Edge& e1 = InsertEdge(*face_v[0], *face_v[1]);
			Edge& e2 = InsertEdge(*face_v[1], *face_v[2]);
			Edge& e3 = InsertEdge(*face_v[2], *face_v[0]);

			Face face;
			e1.e_succ = &e2;
			e2.e_succ = &e3;
			e3.e_succ = &e1;
			e1.e_face = e2.e_face = e3.e_face = &face;
			face_v[0]->v_face = face_v[1]->v_face = face_v[2]->v_face = &face;
			face.f_edge = &e1;
			face.f_verts = face_v;
			face.ID = faces.size();
			faces.push_back(face);
		}

		void DeleteFace(Face& face) {

		}
	#pragma endregion

	#pragma region operations
		bool IsBoundary(Edge& edge) {
			if (edge.e_face == NULL) return false;
			else return true;
		}

		#pragma region vertex operations
		// get adjacent vertices from given vertex
		std::vector<Vertex*> GetVertsFromVertex(Vertex& vert) {
			std::vector<Edge*> edge = GetEdgesFromVertex(vert);
			std::vector<Vertex*> verts;
			for (int i = 0; i < edge.size(); i++) {
				verts.push_back(edge[i]->e_succ->e_vert);
			}
			return verts;
		}

		// get adjacent edges from given vertex
		std::vector<Edge*> GetEdgesFromVertex(Vertex& vert) {
			std::vector<Edge*> edges;
			Edge* base_edge = vert.v_edge;
			Edge* tmp = base_edge;

			do {
				tmp = tmp->e_pair;
				if (IsBoundary(*tmp)) break;

				tmp = tmp->e_succ;
				edges.push_back(tmp);
			} while (tmp != base_edge);

			edges.push_back(base_edge);
			return edges;
		}

		// get adjacent faces from given vertex
		std::vector<Face*> GetFacesFromVertex(Vertex vert) {
			std::vector<Edge*> edge = GetEdgesFromVertex(vert);
			std::vector<Face*> face;
			for (int i = 0; i < edge.size(); i++) {
				face.push_back(edge[i]->e_face);
			}
			return face;
		}
		#pragma endregion

		#pragma region edge operations
		std::vector<Vertex*> GetVertsFromEdge(Edge& edge) {

		}

		std::vector<Edge*> GetEdgesFromEdge(Edge& edge) {
			std::vector<Edge*> e_edge = GetEdgesFromVertex(*edge.e_succ->e_vert);
			std::vector<Edge*> p_edge = GetEdgesFromVertex(*edge.e_vert);
			for (int i = 0; i < p_edge.size(); i++) {
				e_edge.push_back(p_edge[i]->e_pair);
			}
			return e_edge;
		}

		std::vector<Face*> GetFacesFromEdge(Edge& edge) {

		}
		#pragma endregion

		#pragma region face operations
		std::vector<Vertex*> GetVertsFromFace(Face& face) {

		}

		std::vector<Edge*> GetEdgesFromFace(Face& face) {

		}

		std::vector<Face*> GetFacesFromFace(Face& face) {

		}
		#pragma endregion
	#pragma endregion

	#pragma region editable_mesh
		void SetupMesh() override;

		void Edit_Vertex() {
			// draw vertex
			Gizmo::DrawPointPrototype(VAO, vertices.size(), Color::BLUE(), 1.0f);
		}
	#pragma endregion

	};

}