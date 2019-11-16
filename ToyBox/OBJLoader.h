#pragma once
#include <iostream>
#include <vector>
#include <string>
#include <fstream>

#include "Vector2.h"
#include "Vector3.h"
#include "Mesh.h"

namespace moon {
	// Get element at given index position
	template <class T>
	inline const T & getElement(const std::vector<T> &elements, std::string &index) {
		int idx = std::stoi(index);
		if (idx < 0) idx = int(elements.size()) + idx;
		else idx--;
		return elements[idx];
	}

	class Loader {
	public:
		bool gammaCorrection;
		std::vector<Mesh*> LoadedMeshes;
		std::vector<Vertex> LoadedVertices;
		std::vector<unsigned int> LoadedIndices;

		Loader() {}
		~Loader() { LoadedMeshes.clear(); }

		bool LoadFile(const std::string &Path, bool gammaCorrection = false);

	private:
		// Generate vertices from a list of positions, tcoords, normals and a face line
		void GenVerticesFromRawOBJ(std::vector<Vertex>& oVerts,
			const std::vector<Vector3>& iPositions,
			const std::vector<Vector2>& iTCoords,
			const std::vector<Vector3>& iNormals,
			std::string icurline);

		// Triangulate a list of vertices into a face by printing
		// inducies corresponding with triangles within it
		void VertexTriangluation(std::vector<unsigned int>& oIndices,
			const std::vector<Vertex>& iVerts);

		// Load Materials from .mtl file
		bool LoadMaterials(const std::string &path);

		Texture* LoadTexture(const TexType &type, const std::string &path);
	};
}
