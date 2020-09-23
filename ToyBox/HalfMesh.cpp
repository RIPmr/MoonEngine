#include "HalfMesh.h"
#include "SceneMgr.h"

namespace MOON {

	void HalfMesh::SetupMesh() {
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

	void HalfMesh::ClearSelection(const int& type) {
		if (type == VERT)
			MOON_InputManager::Selector::ClearSelectionPrototype(vertices, selected_verts);
		else if (type == EDGE)
			MOON_InputManager::Selector::ClearSelectionPrototype(edges, selected_edges);
		else if (type == FACE)
			MOON_InputManager::Selector::ClearSelectionPrototype(faces, selected_faces);
		else {
			MOON_InputManager::Selector::ClearSelectionPrototype(vertices, selected_verts);
			MOON_InputManager::Selector::ClearSelectionPrototype(edges, selected_edges);
			MOON_InputManager::Selector::ClearSelectionPrototype(faces, selected_faces);
		}
	}

	void HalfMesh::Select(const Element& type, const unsigned int ID) {
		if (type == VERT)
			MOON_InputManager::Selector::SelectPrototype(vertices, selected_verts, ID);
		else if (type == EDGE)
			MOON_InputManager::Selector::SelectPrototype(edges, selected_edges, ID);
		else if (type == FACE)
			MOON_InputManager::Selector::SelectPrototype(faces, selected_faces, ID);
	}

	void HalfMesh::Select_Append(const Element& type, unsigned int ID, const bool& autoInvertSelect) {
		if (type == VERT)
			MOON_InputManager::Selector::Select_AppendPrototype(
				vertices, selected_verts, ID, autoInvertSelect
			);
		else if (type == EDGE)
			MOON_InputManager::Selector::Select_AppendPrototype(
				edges, selected_edges, ID, autoInvertSelect
			);
		else if (type == FACE)
			MOON_InputManager::Selector::Select_AppendPrototype(
				faces, selected_faces, ID, autoInvertSelect
			);
	}

}