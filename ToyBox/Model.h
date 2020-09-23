#pragma once
#include <vector>
#include <string>

#include "BoundingBox.h"
#include "ObjectBase.h"
#include "Transform.h"
#include "Material.h"
#include "Mesh.h"
#include "HalfMesh.h"
#include "OBJMgr.h"
#include "Utility.h"
#include "Hitable.h"
#include "ThreadPool.h"
#include "ButtonEx.h"

namespace MOON {
	class Model : public MObject, public Hitable {
	public:
		std::vector<Mesh*> meshList;
		std::vector<unsigned int> selected_meshes;
		std::string path;
		BoundingBox bbox;
		BoundingBox bbox_world;
		bool gammaCorrection;

		#pragma region constructor
		// for procedural mesh
		Model(const std::string &name, const int id = MOON_AUTOID) : gammaCorrection(false), MObject(name, id), path("[PROCEDURAL]") {}

		Model(const Model& other) {
			this->meshList.resize(other.meshList.size());
			for (int i = 0; i < meshList.size(); i++) {
				meshList[i] = new HalfMesh(other.meshList[i]);
			}
			this->transform = other.transform;
			this->path = other.path;
			this->bbox = other.bbox;
			this->bbox_world = other.bbox_world;
			this->gammaCorrection = other.gammaCorrection;
		}

		// for mesh in OBJ file
		Model(const std::string &path, const std::string &name = "FILENAME", const int id = MOON_AUTOID, const bool gamma = false) :
			path(path), gammaCorrection(gamma), MObject(id) {
			if (!name._Equal("FILENAME")) this->name = name;
			else this->name = Utility::GetPathOrURLShortName(path);

			OBJLoader::progress = 0;
			ThreadPool::CreateThread(&Model::LoadModel, this, path);
			//LoadModel(path);
		}

		virtual ~Model() override {
			Utility::ReleaseVector(meshList);
		}

		void LoadModel(const std::string& path);
		#pragma endregion

		#pragma region operations
		void Draw(Shader* overrideShader = NULL) override;

		void UpdateBBox() {
			for (auto &iter : meshList) {
				bbox.join(iter->bbox);
			}
		}

		// Rough but fast
		void UpdateWorldBBox() {
			std::vector<Vector3> corner;
			bbox.GetCorners(&corner);
			bbox_world.Reset();
			for (auto &iter : corner) {
				bbox_world.join(transform.localToWorldMat.multVec(iter));
			}
		}

		bool Hit(const Ray &r, HitRecord &rec) const {
			HitRecord tempRec;
			bool hitAnything = false;
			tempRec.t = rec.t;

			if (bbox_world.intersect(r)) {
				for (auto &iter : meshList) {
					if (iter->Hit(transform.localToWorldMat, r, tempRec)) {
						hitAnything = true;
						rec = tempRec;
					}
				}
			}

			return hitAnything;
		}

		unsigned int CountVerts() {
			unsigned int vertNum = 0;
			for (auto &iter : meshList) {
				vertNum += iter->vertices.size();
			}
			return vertNum;
		}

		unsigned int CountFaces() {
			unsigned int faceNum = 0;
			for (auto &iter : meshList) {
				faceNum += iter->triangles.size() / 3;
			}
			return faceNum;
		}
		#pragma endregion

		void ListProperties() override;

		// select mesh
		#pragma region select
		void ClearSelection();
		void Select(const unsigned int ID);
		void Select_Append(unsigned int ID, const bool& autoInvertSelect = true);
		#pragma endregion

		// procedural mesh
		#pragma region procedural_mesh
		virtual void CreateProceduralMesh(const bool& interactive) {}
		virtual void ListProceduralProperties() {}
		#pragma endregion
	};
}