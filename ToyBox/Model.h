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

namespace moon {
	class Model : public MObject, public Hitable {
	public:
		std::vector<Mesh*> meshList;
		std::string path;
		BoundingBox bbox;
		BoundingBox bbox_world;
		bool gammaCorrection;

		// for procedural mesh
		Model(const std::string &name, const int id = MOON_AUTOID) : gammaCorrection(false), MObject(name, id) {}

		// for mesh in OBJ file
		Model(const std::string &path, const std::string &name = "FILENAME", const int id = MOON_AUTOID, const bool gamma = false) :
			path(path), gammaCorrection(gamma), MObject(id) {
			LoadModel(path, gamma);

			if (!name._Equal("FILENAME")) this->name = name;
			else this->name = GetPathOrURLShortName(path);
		}
		~Model() override {
			for (auto &iter : meshList) delete iter;
			meshList.clear();
		}

		void Draw() {
			for (int i = 0; i < meshList.size(); i++) {
				meshList[i]->Draw(meshList[i]->material->shader, transform.modelMat);
			}
			if (transform.changeFlag) {
				UpdateWorldBBox();
				transform.changeFlag = false;
			}
		}

		void UpdateBBox() {
			for (auto &iter : meshList) {
				bbox.join(iter->bbox);
			}
		}

		void UpdateWorldBBox() {
			std::vector<Vector3> corner;
			bbox.GetCorners(&corner);
			bbox_world.Reset();
			for (auto &iter : corner) {
				bbox_world.join(transform.modelMat.multVec(iter));
			}
		}

		void LoadModel(const std::string &path, const bool &gammaCorrection) {
			OBJLoader loader;
			loader.LoadFile(path, gammaCorrection);

			std::cout << "- OBJ file loaded, copying mesh list... ..." << std::endl;

			// transfer data in loader to model & building b-box
			meshList = loader.LoadedMeshes;
			for (auto &iter : meshList) {
				iter->parent = this;
				bbox.join(iter->bbox);
			}
		}

		bool Hit(const Ray &r, float tmin, float tmax, HitRecord &rec) const {
			HitRecord tempRec;
			bool hitAnything = false;
			double closestSoFar = tmax;

			if (bbox_world.intersect(r)) {
				for (auto &iter : meshList) {
					if (iter->Hit(r, tmin, closestSoFar, tempRec)) {
						hitAnything = true;
						closestSoFar = tempRec.t;
						rec = tempRec;
					}
				}
			}

			return hitAnything;
		}

	};
}