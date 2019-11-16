#pragma once
#include "Transform.h"
#include "Mesh.h"
#include "Material.h"
#include "Hitable.h"
#include "ObjectBase.h"
#include "OBJLoader.h"
#include "Utility.h"

#include <vector>
#include <string>

namespace moon {
	class Model : public MObject, public Hitable {
	public:
		std::vector<Mesh*> meshList;
		std::string path;
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
		~Model() {
			for (auto &iter : meshList) delete iter;
			meshList.clear();
		}

		void Draw() {
			for (int i = 0; i < meshList.size(); i++) {
				meshList[i]->Draw(meshList[i]->material->shader);
			}
		}

		void LoadModel(const std::string &path, const bool &gammaCorrection) {
			Loader loader;
			loader.LoadFile(path, gammaCorrection);

			std::cout << "copying mesh list" << std::endl;
			// transfer data in loader to model
			meshList = loader.LoadedMeshes;
		}

		bool Hit(const Ray &r, float tmin, float tmax, HitRecord &rec) const {
			HitRecord tempRec;
			bool hitAnything = false;
			double closestSoFar = tmax;

			for (auto &iter : meshList) {
				if (iter->Hit(r, tmin, closestSoFar, tempRec)) {
					hitAnything = true;
					closestSoFar = tempRec.t;
					rec = tempRec;
				}
			}

			return hitAnything;
		}

	};
}