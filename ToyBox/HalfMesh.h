#pragma once
#include "Mesh.h"
#include "Vector4.h"
#include "Utility.h"

#include <vector>
#include <hash_map>
#include <iostream>

namespace MOON {

	struct HE_Vertex {
		Vector3 x;
		HE_Edge* v_edge; // 以该顶点为首顶点的出边(任意一条)
		HE_Face* v_face; // 包含该顶点的面(任意一面)

		HE_Vertex(const Vector3& x) {
			this->x.setValue(x);
			v_edge = nullptr;
			v_face = nullptr;
		}
	};

	struct HE_Edge {
		HE_Edge* e_pair;		// 对偶边
		HE_Edge* e_succ;		// 后继边
		HE_Face* e_face;		// 右侧面
		HE_Vertex* e_vert;	    // 首顶点

		HE_Edge() {
			e_pair = nullptr;
			e_succ = nullptr;
			e_face = nullptr;
			e_vert = nullptr;
		}
	};

	struct HE_Face {
		std::vector<HE_Vertex*> f_verts;
		HE_Edge* f_edge;

		HE_Face() {
			f_edge = nullptr;
		}
	};

	// half-edge data structure
	class HalfMesh {
	public:
		std::vector<HE_Vertex*> vertices;
		std::vector<HE_Edge*> edges;
		std::vector<HE_Face*> faces;

		std::hash_map<std::pair<HE_Vertex*, HE_Vertex*>, HE_Edge*> m_emap;

		std::pair<HE_Vertex*, HE_Vertex*> vert_pair(HE_Vertex* v1, HE_Vertex* v2) {
			return std::make_pair(v1, v2);
		}

		HalfMesh(Mesh& mesh) {
			ConvertFromMesh(mesh);
		}

		~HalfMesh() {
			ReleaseVector(vertices);
			ReleaseVector(edges);
			ReleaseVector(faces);
		}

		void ConvertFromMesh(Mesh& mesh) {

		}

		void InsertVertex(const Vector3& x) {
			HE_Vertex* vert = new HE_Vertex(x);
			vertices.push_back(vert);
		}

		void InsertFace(std::vector<HE_Vertex*>& face_v) {
			if (face_v.empty()) return;

			HE_Edge* e1 = InsertEdge(face_v[0], face_v[1]);
			HE_Edge* e2 = InsertEdge(face_v[1], face_v[2]);
			HE_Edge* e3 = InsertEdge(face_v[2], face_v[0]);

			if (!e1 || !e2 || !e3) return;

			HE_Face* face = new HE_Face;
			e1->e_succ = e2;
			e2->e_succ = e3;
			e3->e_succ = e1;
			e1->e_face = e2->e_face = e3->e_face = face;
			face_v[0]->v_face = face_v[1]->v_face = face_v[2]->v_face = face;
			face->f_edge = e1;
			face->f_verts = face_v;
			this->faces.push_back(face);
		}

		HE_Edge* InsertEdge(HE_Vertex* v1, HE_Vertex* v2) {
			if (v1 == NULL || v2 == NULL) return NULL;

			auto pair = vert_pair(v1, v2);
			if (m_emap[pair] != NULL) return m_emap[pair];

			// 提前建立对偶边, 并建立好点边关系
			HE_Edge* edge = new HE_Edge();
			edge->e_vert = v1;
			v1->v_edge = edge;
			HE_Edge* p_edge = new HE_Edge();
			p_edge->e_vert = v2;
			v2->v_edge = p_edge;

			// 建立对偶关系
			edge->e_pair = p_edge;
			p_edge->e_pair = edge;
			m_emap[pair] = edge;
			m_emap[vert_pair(v2, v1)] = p_edge;

			edges.push_back(edge);

			return edge;
		}

		bool IsBoundary(HE_Edge* edge) {
			if (edge->e_face == NULL) return false;
			else return true;
		}

		std::vector<HE_Edge*> GetEdgesFromVertex(HE_Vertex* vert) {
			std::vector<HE_Edge*> edges;
			HE_Edge * base_edge = vert->v_edge;
			HE_Edge * tmp = base_edge;

			do {
				tmp = tmp->e_pair;
				// 边缘边，没有对偶面
				if (IsBoundary(tmp)) break;

				tmp = tmp->e_succ;
				edges.push_back(tmp);
			} while (tmp != base_edge);

			edges.push_back(base_edge);
			return edges;
		}

		std::vector<HE_Vertex*> GetVertsFromVertex(HE_Vertex* vert) {
			std::vector<HE_Edge*> edge = GetEdgesFromVertex(vert);
			std::vector<HE_Vertex*> verts;
			for (int i = 0; i < edge.size(); i++) {
				verts.push_back(edge[i]->e_succ->e_vert);
			}
			return verts;
		}

		std::vector<HE_Face*> GetFacesFromVertex(HE_Vertex* vert) {
			std::vector<HE_Edge*> edge = GetEdgesFromVertex(vert);
			std::vector<HE_Vertex*> face;
			for (int i = 0; i < edge.size(); i++) {
				faces.push_back(edge[i]->e_face);
			}
			return faces;
		}

		std::vector<HE_Edge*> GetEdgesFromEdge(HE_Edge* edge) {
			std::vector<HE_Edge*> e_edge = GetEdgesFromVertex(edge->e_succ->e_vert);
			std::vector<HE_Edge*> p_edge = GetEdgesFromVertex(edge->e_vert);
			for (int i = 0; i < p_edge.size(); i++) {
				e_edge.push_back(p_edge[i]->e_pair);
			}
			return e_edge;
		}

	};

}