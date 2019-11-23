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
			loader.LoadFile(path, meshList, gammaCorrection);

			std::cout << "- OBJ file loaded, copying mesh list... ..." << std::endl;

			// transfer data in loader to model & building b-box
			for (auto &iter : meshList) {
				iter->parent = this;
				bbox.join(iter->bbox);
			}
		}

		bool Hit(const Ray &r, HitRecord &rec) const {
			HitRecord tempRec;
			bool hitAnything = false;
			tempRec.t = rec.t;

			if (bbox_world.intersect(r)) {
				for (auto &iter : meshList) {
					if (iter->Hit(r, tempRec)) {
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

		void ListProperties() override {
			// list name ----------------------------------------------------------------------
			ListName();
			ImGui::Separator();

			// list transform -----------------------------------------------------------------			ListTransform();
			ListTransform();
			ImGui::Separator();

			// list Mesh ----------------------------------------------------------------------
			ImGui::Text("Mesh:");
			if (ImGui::TreeNode((std::to_string(meshList.size()) + " meshes, " + std::to_string(CountVerts()) + " verts").c_str())) {
				for (auto &iter : meshList) {
					ImGui::Columns(2, "mycolumns", false);
					ImGui::Text((std::string(ICON_FA_PUZZLE_PIECE) + " " + iter->name).c_str()); ImGui::NextColumn();
					ImGui::Text((std::string(ICON_FA_GLOBE) + " " + iter->material->name).c_str());
					ImGui::Columns(1);
				}
				ImGui::TreePop();
			}
			ImGui::Separator();

			// list operators -----------------------------------------------------------------
			opstack.ListStacks();
			ImGui::Spacing();
		}

	};
}