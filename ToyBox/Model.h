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
		BoundingBox bbox_local;

		#pragma region constructor
		// for procedural mesh
		Model(const std::string &name, const int id = MOON_AUTOID) : MObject(name, id), path(PROCEDURAL) {}

		Model(const Model& other) {
			this->meshList.resize(other.meshList.size());
			for (int i = 0; i < meshList.size(); i++) {
				meshList[i] = new HalfMesh(other.meshList[i]);
			}
			this->transform = other.transform;
			this->path = other.path;
			this->bbox = other.bbox;
			this->bbox_local = other.bbox_local;
		}

		// for mesh in OBJ file
		Model(const std::string &path, const std::string &name = UseFileName, const int id = MOON_AUTOID) :
			path(path), MObject(id) {
			if (!name._Equal(UseFileName)) this->name = name;
			else this->name = Utility::GetPathOrURLShortName(path);

			OBJLoader::progress = 0;
			ThreadPool::CreateThread(&Model::LoadModel, this, path);
			//LoadModel(path);
		}

		Model(const std::string &path, const bool asynchronous = true, const std::string &name = UseFileName, const int id = MOON_AUTOID) :
			path(path), MObject(id) {
			if (!name._Equal(UseFileName)) this->name = name;
			else this->name = Utility::GetPathOrURLShortName(path);

			if (asynchronous) {
				OBJLoader::progress = 0;
				ThreadPool::CreateThread(&Model::LoadModel, this, path);
			} else LoadModel(path);
		}

		virtual ~Model() override {
			Utility::ReleaseVector(meshList);
		}

		void LoadModel(const std::string& path);
		#pragma endregion

		#pragma region operations
		virtual void BuildBVH() {
			for (auto &iter : meshList) {
				iter->BuildBVH();
			}
		}

		void Draw(Shader* overrideShader = NULL) override;

		void UpdateBBox() {
			bbox_local.Reset();
			for (auto &iter : meshList) {
				bbox_local.join(iter->bbox);
			}
		}

		// Rough but fast
		void UpdateWorldBBox() {
			std::vector<Vector3> corner;
			bbox_local.GetCorners(&corner);
			bbox.Reset();
			for (auto &iter : corner) {
				bbox.join(transform.localToWorldMat.multVec(iter));
			}
		}

		bool Hit(const Ray &r, HitRecord &rec) const;

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

		void DebugBVH(const Vector4& color = Color::GREEN()) {
			for (auto &iter : meshList) {
				if (iter->localBVH != nullptr) iter->localBVH->Draw(color);
			}
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
		virtual void CreateProcedural(const bool& interactive) override {}
		virtual void ListProceduralProperties() {}
		#pragma endregion
	};
}