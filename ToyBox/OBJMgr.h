#pragma once
#include <iostream>
#include <vector>
#include <string>
#include <fstream>

#include "Vector2.h"
#include "Vector3.h"
#include "Mesh.h"

namespace MOON {
	extern class Model;

	// Get element at given index position
	template <class T>
	inline const T & getElement(const std::vector<T> &elements, std::string &index) {
		int idx = std::stoi(index);
		if (idx < 0) idx = int(elements.size()) + idx;
		else idx--;
		return elements[idx];
	}

	// TODO
	class OBJExporter {
	public:
		OBJExporter() = default;
		~OBJExporter() = default;

		bool ExportTo(const std::vector<Mesh*> &meshList, const std::string &path);
	};

	class OBJLoader {
	public:
		static bool gammaCorrection;
		static float progress;
		static std::string info;
		static std::vector<Vertex> LoadedVertices;
		static std::vector<unsigned int> LoadedIndices;

		//OBJLoader() : info("Loading... ..."), gammaCorrection(false), progress(0) {}
		//~OBJLoader() = default;

		static void GetInfo(std::string& info, float& progress);
		static void LoadFile(Model* container);
		static bool LoadFile(const std::string &Path, std::vector<Mesh*> &LoadedMeshes, bool gammaCorrection = false);

	private:
		// Generate vertices from a list of positions, tcoords, normals and a face line
		static void GenVerticesFromRawOBJ(std::vector<Vertex>& oVerts,
								   const std::vector<Vector3>& iPositions,
								   const std::vector<Vector2>& iTCoords,
								   const std::vector<Vector3>& iNormals,
								   std::string icurline);

		// Triangulate a list of vertices into a face by printing
		// inducies corresponding with triangles within it
		static void VertexTriangluation(std::vector<unsigned int>& oIndices,
										const std::vector<Vertex>& iVerts);

		// Load Materials from .mtl file
		static bool LoadMaterials(const std::string &path);

		static Texture* LoadTexture(const TexType &type, const std::string &path);
	};
}
