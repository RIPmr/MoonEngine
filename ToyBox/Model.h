#pragma once
#include <vector>
#include <string>

#include "BoundingBox.h"
#include "ObjectBase.h"
#include "Transform.h"
#include "Material.h"
#include "Mesh.h"
#include "OBJMgr.h"
#include "Utility.h"
#include "Hitable.h"
#include "ThreadPool.h"
#include "ButtonEx.h"

namespace MOON {
	class Model : public MObject, public Hitable {
	public:
		std::vector<Mesh*> meshList;
		std::string path;
		BoundingBox bbox;
		BoundingBox bbox_world;
		bool gammaCorrection;

		// for procedural mesh
		Model(const std::string &name, const int id = MOON_AUTOID) : gammaCorrection(false), MObject(name, id), path("[PROCEDURAL]") {}

		// for mesh in OBJ file
		Model(const std::string &path, const std::string &name = "FILENAME", const int id = MOON_AUTOID, const bool gamma = false) :
			path(path), gammaCorrection(gamma), MObject(id) {
			if (!name._Equal("FILENAME")) this->name = name;
			else this->name = GetPathOrURLShortName(path);

			OBJLoader::progress = 0;
			ThreadPool::CreateThread(&Model::LoadModel, this, path);
			//LoadModel(path);
		}
		~Model() override {
			for (auto &iter : meshList) delete iter;
			meshList.clear();
		}

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

		void LoadModel(const std::string& path);

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

		void ListProperties() override;

		// procedural mesh
		virtual void CreateProceduralMesh(const bool& interactive) {}
		virtual void ListProceduralProperties() {}
	};
}