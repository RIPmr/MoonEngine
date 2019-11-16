#pragma once
#include <iostream>
#include <vector>
#include <string>
#include <fstream>
#include <cmath>

#include "Vector2.h"
#include "Vector3.h"
#include "Mesh.h"
#include "Material.h"
#include "MathUtils.h"
#include "Texture.h"
#include "Utility.h"

namespace moon {
	// Split a String into a string array at a given token
	inline void split(const std::string &in, std::vector<std::string> &out, std::string token) {
		out.clear();
		std::string temp;
		for (int i = 0; i < int(in.size()); i++) {
			std::string test = in.substr(i, token.size());
			if (test == token) {
				if (!temp.empty()) {
					out.push_back(temp);
					temp.clear();
					i += (int)token.size() - 1;
				} else {
					out.push_back("");
				}
			} else if (i + token.size() >= in.size()) {
				temp += in.substr(i, token.size());
				out.push_back(temp);
				break;
			} else {
				temp += in[i];
			}
		}
	}

	// Get tail of string after first token and possibly following spaces
	inline std::string tail(const std::string &in) {
		size_t token_start = in.find_first_not_of(" \t");
		size_t space_start = in.find_first_of(" \t", token_start);
		size_t tail_start = in.find_first_not_of(" \t", space_start);
		size_t tail_end = in.find_last_not_of(" \t");
		if (tail_start != std::string::npos && tail_end != std::string::npos) {
			return in.substr(tail_start, tail_end - tail_start + 1);
		} else if (tail_start != std::string::npos) {
			return in.substr(tail_start);
		}
		return "";
	}

	// Get first token of string
	inline std::string firstToken(const std::string &in) {
		if (!in.empty()) {
			size_t token_start = in.find_first_not_of(" \t");
			size_t token_end = in.find_first_of(" \t", token_start);
			if (token_start != std::string::npos && token_end != std::string::npos) {
				return in.substr(token_start, token_end - token_start);
			} else if (token_start != std::string::npos) {
				return in.substr(token_start);
			}
		}
		return "";
	}

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
		std::vector<Mesh> LoadedMeshes;
		std::vector<Vertex> LoadedVertices;
		std::vector<unsigned int> LoadedIndices;
		std::vector<MoonMtl> LoadedMaterials;
		std::vector<Texture> LoadedTextures;

		Loader() {}
		~Loader() { LoadedMeshes.clear(); }

		bool LoadFile(const std::string &Path, bool gammaCorrection = false) {
			// If the file is not an .obj file return false
			if (Path.substr(Path.size() - 4, 4) != ".obj")
				return false;

			this->gammaCorrection = gammaCorrection;

			std::ifstream file(Path);
			if (!file.is_open()) return false;

			LoadedMeshes.clear();
			LoadedVertices.clear();
			LoadedIndices.clear();

			std::vector<Vector3> Positions;
			std::vector<Vector2> TCoords;
			std::vector<Vector3> Normals;

			std::vector<Vertex> Vertices;
			std::vector<unsigned int> Indices;

			std::vector<std::string> MeshMatNames;

			bool listening = false;
			std::string meshname;

			Mesh tempMesh;

#ifdef OBJL_CONSOLE_OUTPUT
			const unsigned int outputEveryNth = 1000;
			unsigned int outputIndicator = outputEveryNth;
#endif

			std::string curline;
			while (std::getline(file, curline)) {
#ifdef OBJL_CONSOLE_OUTPUT
				if ((outputIndicator = ((outputIndicator + 1) % outputEveryNth)) == 1) {
					if (!meshname.empty())
						std::cout << "\r- " << meshname << "\t| vertices > " << Positions.size()
								  << "\t| texcoords > " << TCoords.size()
								  << "\t| normals > " << Normals.size()
								  << "\t| triangles > " << (Vertices.size() / 3)
								  << (!MeshMatNames.empty() ? "\t| material: " + MeshMatNames.back() : "");
				}
#endif

				// Generate a Mesh Object or Prepare for an object to be created
				if (firstToken(curline) == "o" || firstToken(curline) == "g" || curline[0] == 'g') {
					if (!listening) {
						listening = true;
						if (firstToken(curline) == "o" || firstToken(curline) == "g")
							meshname = tail(curline);
						else meshname = "unnamed";
					} else {
						// Generate the mesh to put into the array
						if (!Indices.empty() && !Vertices.empty()) {
							// Create Mesh
							tempMesh = Mesh(Vertices, Indices);
							tempMesh.name = meshname;

							// Insert Mesh
							LoadedMeshes.push_back(tempMesh);

							// Cleanup
							Vertices.clear();
							Indices.clear();
							meshname.clear();

							meshname = tail(curline);
						} else {
							if (firstToken(curline) == "o" || firstToken(curline) == "g")
								meshname = tail(curline);
							else meshname = "unnamed";
						}
					}
#ifdef OBJL_CONSOLE_OUTPUT
					std::cout << std::endl;
					outputIndicator = 0;
#endif
				}
				// Generate a Vertex Position
				if (firstToken(curline) == "v") {
					std::vector<std::string> spos;
					Vector3 vpos;
					split(tail(curline), spos, " ");

					vpos.x = std::stof(spos[0]);
					vpos.y = std::stof(spos[1]);
					vpos.z = std::stof(spos[2]);

					Positions.push_back(vpos);
				}
				// Generate a Vertex Texture Coordinate
				if (firstToken(curline) == "vt") {
					std::vector<std::string> stex;
					Vector2 vtex;
					split(tail(curline), stex, " ");

					vtex.x = std::stof(stex[0]);
					vtex.y = std::stof(stex[1]);

					TCoords.push_back(vtex);
				}
				// Generate a Vertex Normal;
				if (firstToken(curline) == "vn") {
					std::vector<std::string> snor;
					Vector3 vnor;
					split(tail(curline), snor, " ");

					vnor.x = std::stof(snor[0]);
					vnor.y = std::stof(snor[1]);
					vnor.z = std::stof(snor[2]);

					Normals.push_back(vnor);
				}
				// Generate a Face (vertices & indices)
				if (firstToken(curline) == "f") {
					// Generate the vertices
					std::vector<Vertex> vVerts;
					GenVerticesFromRawOBJ(vVerts, Positions, TCoords, Normals, curline);

					// Add Vertices
					for (int i = 0; i < int(vVerts.size()); i++) {
						Vertices.push_back(vVerts[i]);
						LoadedVertices.push_back(vVerts[i]);
					}

					std::vector<unsigned int> iIndices;
					VertexTriangluation(iIndices, vVerts);
					// Add Indices
					for (int i = 0; i < int(iIndices.size()); i++) {
						unsigned int indnum = (unsigned int)((Vertices.size()) - vVerts.size()) + iIndices[i];
						Indices.push_back(indnum);

						indnum = (unsigned int)((LoadedVertices.size()) - vVerts.size()) + iIndices[i];
						LoadedIndices.push_back(indnum);
					}
				}
				// Get Mesh Material Name
				if (firstToken(curline) == "usemtl") {
					MeshMatNames.push_back(tail(curline));

					// Create new Mesh, if Material changes within a group
					if (!Indices.empty() && !Vertices.empty()) {
						// Create Mesh
						tempMesh = Mesh(Vertices, Indices);
						tempMesh.name = meshname;
						int i = 2;
						while (1) {
							tempMesh.name = meshname + "_" + std::to_string(i);
							for (auto &m : LoadedMeshes)
								if (m.name == tempMesh.name)
									continue;
							break;
						}

						// Insert Mesh
						LoadedMeshes.push_back(tempMesh);

						// Cleanup
						Vertices.clear();
						Indices.clear();
					}

#ifdef OBJL_CONSOLE_OUTPUT
					outputIndicator = 0;
#endif
				}
				// Load Materials
				if (firstToken(curline) == "mtllib") {
					// Generate LoadedMaterial
					// Generate a path to the material file
					std::vector<std::string> temp;
					split(Path, temp, "/");

					std::string pathtomat = "";
					if (temp.size() != 1) {
						for (int i = 0; i < temp.size() - 1; i++) {
							pathtomat += temp[i] + "/";
						}
					}
					pathtomat += tail(curline);

#ifdef OBJL_CONSOLE_OUTPUT
					std::cout << std::endl << "- find materials in: " << pathtomat << std::endl;
#endif

					// Load Materials
					LoadMaterials(pathtomat);
				}
			}

#ifdef OBJL_CONSOLE_OUTPUT
			std::cout << std::endl;
#endif

			// Deal with last mesh
			if (!Indices.empty() && !Vertices.empty()) {
				// Create Mesh
				tempMesh = Mesh(Vertices, Indices);
				tempMesh.name = meshname;

				// Insert Mesh
				LoadedMeshes.push_back(tempMesh);
			}

			file.close();

			// Set Materials for each Mesh
			for (int i = 0; i < MeshMatNames.size(); i++) {
				std::string matname = MeshMatNames[i];

				// Find corresponding material name in loaded materials
				// when found copy material variables into mesh material
				for (int j = 0; j < LoadedMaterials.size(); j++) {
					if (LoadedMaterials[j].name == matname) {
						LoadedMeshes[i].material = &LoadedMaterials[j];
						break;
					}
				}
			}

			if (LoadedMeshes.empty() && LoadedVertices.empty() && LoadedIndices.empty()) {
				return false;
			} else {
				return true;
			}
		}

	private:
		// Generate vertices from a list of positions, tcoords, normals and a face line
		void GenVerticesFromRawOBJ(std::vector<Vertex>& oVerts,
			const std::vector<Vector3>& iPositions,
			const std::vector<Vector2>& iTCoords,
			const std::vector<Vector3>& iNormals,
			std::string icurline) {
			std::vector<std::string> sface, svert;
			Vertex vVert;
			split(tail(icurline), sface, " ");

			bool noNormal = false;

			// For every given vertex do this
			for (int i = 0; i < int(sface.size()); i++) {
				// See What type the vertex is.
				int vtype;
				split(sface[i], svert, "/");

				// Check for just position - v1
				if (svert.size() == 1) {
					// Only position
					vtype = 1;
				}

				// Check for position & texture - v1/vt1
				if (svert.size() == 2) {
					// Position & Texture
					vtype = 2;
				}

				// Check for Position, Texture and Normal - v1/vt1/vn1
				// or if Position and Normal - v1//vn1
				if (svert.size() == 3) {
					if (svert[1] != "") {
						// Position, Texture, and Normal
						vtype = 4;
					} else {
						// Position & Normal
						vtype = 3;
					}
				}

				// Calculate and store the vertex
				switch (vtype) {
					case 1: { // P
						vVert.Position = getElement(iPositions, svert[0]);
						vVert.TexCoords = Vector2(0, 0);
						noNormal = true;
						oVerts.push_back(vVert);
						break;
					}
					case 2: { // P/T
						vVert.Position = getElement(iPositions, svert[0]);
						vVert.TexCoords = getElement(iTCoords, svert[1]);
						noNormal = true;
						oVerts.push_back(vVert);
						break;
					}
					case 3: { // P//N
						vVert.Position = getElement(iPositions, svert[0]);
						vVert.TexCoords = Vector2(0, 0);
						vVert.Normal = getElement(iNormals, svert[2]);
						oVerts.push_back(vVert);
						break;
					}
					case 4: { // P/T/N
						vVert.Position = getElement(iPositions, svert[0]);
						vVert.TexCoords = getElement(iTCoords, svert[1]);
						vVert.Normal = getElement(iNormals, svert[2]);
						oVerts.push_back(vVert);
						break;
					}
					default: {
						break;
					}
				}
			}

			// take care of missing normals
			// these may not be truly acurate but it is the 
			// best they get for not compiling a mesh with normals	
			if (noNormal) {
				Vector3 A = oVerts[0].Position - oVerts[1].Position;
				Vector3 B = oVerts[2].Position - oVerts[1].Position;

				Vector3 normal = Vector3::Cross(A, B);
				for (int i = 0; i < int(oVerts.size()); i++) {
					oVerts[i].Normal = normal;
				}
			}
		}

		// Triangulate a list of vertices into a face by printing
		// inducies corresponding with triangles within it
		void VertexTriangluation(std::vector<unsigned int>& oIndices,
			const std::vector<Vertex>& iVerts) {
			// If there are 2 or less verts,
			// no triangle can be created, so exit
			if (iVerts.size() < 3) return;

			// If it is a triangle no need to calculate it
			if (iVerts.size() == 3) {
				oIndices.push_back(0);
				oIndices.push_back(1);
				oIndices.push_back(2);
				return;
			}

			// Create a list of vertices
			std::vector<Vertex> tVerts = iVerts;

			while (true) {
				// For every vertex
				for (int i = 0; i < int(tVerts.size()); i++) {
					// pPrev = the previous vertex in the list
					Vertex pPrev;
					if (i == 0) pPrev = tVerts[tVerts.size() - 1];
					else pPrev = tVerts[i - 1];

					// pCur = the current vertex;
					Vertex pCur = tVerts[i];

					// pNext = the next vertex in the list
					Vertex pNext;
					if (i == tVerts.size() - 1) pNext = tVerts[0];
					else pNext = tVerts[i + 1];
					

					// Check to see if there are only 3 verts left
					// if so this is the last triangle
					if (tVerts.size() == 3) {
						// Create a triangle from pCur, pPrev, pNext
						for (int j = 0; j < int(tVerts.size()); j++) {
							if (iVerts[j].Position == pCur.Position)
								oIndices.push_back(j);
							if (iVerts[j].Position == pPrev.Position)
								oIndices.push_back(j);
							if (iVerts[j].Position == pNext.Position)
								oIndices.push_back(j);
						}

						tVerts.clear();
						break;
					}
					if (tVerts.size() == 4) {
						// Create a triangle from pCur, pPrev, pNext
						for (int j = 0; j < int(iVerts.size()); j++) {
							if (iVerts[j].Position == pCur.Position)
								oIndices.push_back(j);
							if (iVerts[j].Position == pPrev.Position)
								oIndices.push_back(j);
							if (iVerts[j].Position == pNext.Position)
								oIndices.push_back(j);
						}

						Vector3 tempVec;
						for (int j = 0; j < int(tVerts.size()); j++) {
							if (tVerts[j].Position != pCur.Position
								&& tVerts[j].Position != pPrev.Position
								&& tVerts[j].Position != pNext.Position) {
								tempVec = tVerts[j].Position;
								break;
							}
						}

						// Create a triangle from pCur, pPrev, pNext
						for (int j = 0; j < int(iVerts.size()); j++) {
							if (iVerts[j].Position == pPrev.Position)
								oIndices.push_back(j);
							if (iVerts[j].Position == pNext.Position)
								oIndices.push_back(j);
							if (iVerts[j].Position == tempVec)
								oIndices.push_back(j);
						}

						tVerts.clear();
						break;
					}

					// If Vertex is not an interior vertex
					float angle = Vector3::Angle(pPrev.Position - pCur.Position, pNext.Position - pCur.Position) * (180 / 3.14159265359);
					if (angle <= 0 && angle >= 180) continue;

					// If any vertices are within this triangle
					bool inTri = false;
					for (int j = 0; j < int(iVerts.size()); j++) {
						if (MoonMath::IsInTriangle(iVerts[j].Position, pPrev.Position, pCur.Position, pNext.Position)
							&& iVerts[j].Position != pPrev.Position
							&& iVerts[j].Position != pCur.Position
							&& iVerts[j].Position != pNext.Position) {
							inTri = true;
							break;
						}
					}
					if (inTri) continue;

					// Create a triangle from pCur, pPrev, pNext
					for (int j = 0; j < int(iVerts.size()); j++) {
						if (iVerts[j].Position == pCur.Position)
							oIndices.push_back(j);
						if (iVerts[j].Position == pPrev.Position)
							oIndices.push_back(j);
						if (iVerts[j].Position == pNext.Position)
							oIndices.push_back(j);
					}

					// Delete pCur from the list
					for (int j = 0; j < int(tVerts.size()); j++) {
						if (tVerts[j].Position == pCur.Position) {
							tVerts.erase(tVerts.begin() + j);
							break;
						}
					}

					// reset i to the start -1 since loop will add 1 to it
					i = -1;
				}

				// if no triangles were created
				if (oIndices.size() == 0) break;

				// if no more vertices
				if (tVerts.size() == 0) break;
			}
		}

		// Load Materials from .mtl file
		bool LoadMaterials(const std::string &path) {
			// If the file is not a material file return false
			if (path.substr(path.size() - 4, path.size()) != ".mtl")
				return false;

			std::ifstream file(path);
			// If the file is not found return false
			if (!file.is_open()) return false;

			MoonMtl tempMaterial;
			bool listening = false;
			// Go through each line looking for material variables
			std::string curline;
			while (std::getline(file, curline)) {
				// new material and material name
				if (firstToken(curline) == "newmtl") {
					if (!listening) {
						listening = true;

						if (curline.size() > 7) {
							tempMaterial.name = tail(curline);
						} else {
							tempMaterial.name = "none";
						}
					} else {
						// Generate the material
						LoadedMaterials.push_back(tempMaterial);

						// Clear Loaded Material
						tempMaterial = MoonMtl();
						if (curline.size() > 7) {
							tempMaterial.name = tail(curline);
						} else {
							tempMaterial.name = "none";
						}
					}
				}
				// Ambient Color
				if (firstToken(curline) == "Ka") {
					std::vector<std::string> temp;
					split(tail(curline), temp, " ");

					if (temp.size() != 3) continue;

					tempMaterial.Ka.x = std::stof(temp[0]);
					tempMaterial.Ka.y = std::stof(temp[1]);
					tempMaterial.Ka.z = std::stof(temp[2]);
				}
				// Diffuse Color
				if (firstToken(curline) == "Kd") {
					std::vector<std::string> temp;
					split(tail(curline), temp, " ");

					if (temp.size() != 3) continue;

					tempMaterial.Kd.x = std::stof(temp[0]);
					tempMaterial.Kd.y = std::stof(temp[1]);
					tempMaterial.Kd.z = std::stof(temp[2]);
				}
				// Specular Color
				if (firstToken(curline) == "Ks") {
					std::vector<std::string> temp;
					split(tail(curline), temp, " ");

					if (temp.size() != 3) continue;

					tempMaterial.Ks.x = std::stof(temp[0]);
					tempMaterial.Ks.y = std::stof(temp[1]);
					tempMaterial.Ks.z = std::stof(temp[2]);
				}
				// Specular Exponent
				if (firstToken(curline) == "Ns") {
					tempMaterial.Ns = std::stof(tail(curline));
				}
				// Optical Density
				if (firstToken(curline) == "Ni") {
					tempMaterial.Ni = std::stof(tail(curline));
				}
				// Dissolve
				if (firstToken(curline) == "d") {
					tempMaterial.d = std::stof(tail(curline));
				}
				// Illumination
				if (firstToken(curline) == "illum") {
					tempMaterial.illum = std::stoi(tail(curline));
				}

				// Load textures
				int texID = -1;
				// Ambient Texture
				if (firstToken(curline) == "map_Ka") {
					texID = LoadTexture(tempMaterial, TexType::ambient, tail(curline));
					if (texID != -1) tempMaterial.map_Ka = &LoadedTextures[texID];
				}
				// Diffuse Texture
				if (firstToken(curline) == "map_Kd") {
					texID = LoadTexture(tempMaterial, TexType::diffuse, tail(curline));
					if (texID != -1) tempMaterial.map_Kd = &LoadedTextures[texID];
				}
				// Specular Texture
				if (firstToken(curline) == "map_Ks") {
					texID = LoadTexture(tempMaterial, TexType::specular, tail(curline));
					if (texID != -1) tempMaterial.map_Ks = &LoadedTextures[texID];
				}
				// Specular Hightlight Map
				if (firstToken(curline) == "map_Ns") {
					texID = LoadTexture(tempMaterial, TexType::highlight, tail(curline));
					if (texID != -1) tempMaterial.map_Ns = &LoadedTextures[texID];
				}
				// Alpha Texture
				if (firstToken(curline) == "map_d") {
					texID = LoadTexture(tempMaterial, TexType::alpha, tail(curline));
					if (texID != -1) tempMaterial.map_d = &LoadedTextures[texID];
				}
				// Bump Map
				if (firstToken(curline) == "map_Bump" || firstToken(curline) == "map_bump" || firstToken(curline) == "bump") {
					texID = LoadTexture(tempMaterial, TexType::normal, tail(curline));
					if (texID != -1) tempMaterial.map_bump = &LoadedTextures[texID];
				}
			}

			// Deal with last material
			// Push Back loaded Material
			LoadedMaterials.push_back(tempMaterial);

			// Test to see if anything was loaded
			// If not return false
			if (LoadedMaterials.empty()) return false;
			// If so return true
			else return true;
		}

		int LoadTexture(const Material &mat, const TexType &type, const std::string &path) {
			// if find corresponding texture in loaded textures
			for (int j = 0; j < LoadedTextures.size(); j++) {
				if (LoadedTextures[j].path == path) {
					return j;
				}
			}

			// else load that new texture
			GLuint texID = (GLuint)MOON_UNSPECIFIEDID;
			int texWidth, texHeight;
			if (LoadTextureFromFile(path, texID, texWidth, texHeight, gammaCorrection))
				LoadedTextures.push_back(Texture(type, path, texWidth, texHeight, texID));

			return texID;
		}
	};
}
