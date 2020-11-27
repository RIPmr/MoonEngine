#include "Model.h"
#include "SceneMgr.h"
#include "MoonEnums.h"
#include "Renderer.h"

namespace MOON {

	void Model::LoadModel(const std::string& path) {
		OBJLoader::LoadFile(path, meshList);
		std::cout << "- OBJ file loaded, copying mesh list... ..." << std::endl;

		// transfer data in loader to model & building b-box
		for (auto &iter : meshList) {
			//iter->parent = this;
			bbox.join(iter->bbox);
		}

		// add model to manager
		MOON_ModelManager::AddItem(this);
	}

	void Model::Draw(Shader* overrideShader) {
		bool updateFlag = false;
		for (int i = 0; i < meshList.size(); i++) {
			meshList[i]->Draw(
				overrideShader == NULL ? meshList[i]->material->shader : overrideShader, 
				transform.localToWorldMat, ID == MOON_InputManager::hoverID, selected
			);
			if (meshList[i]->changed) {
				meshList[i]->changed = false;
				updateFlag = true;
			}
		}
		if (updateFlag) UpdateBBox();
		if (updateFlag || transform.changeFlag) {
			UpdateWorldBBox();
			transform.changeFlag = false;
		}
	}

	void Model::ListProperties() {
		// list name ----------------------------------------------------------------------
		ListName();
		ImGui::Separator();

		// list transform -----------------------------------------------------------------
		ListTransform();
		ImGui::Separator();

		// list procedural property -------------------------------------------------------
		if (path._Equal(PROCEDURAL)) {
			ListProceduralProperties();
			ImGui::Separator();
		}

		// list mesh ----------------------------------------------------------------------
		ImGui::Text("Mesh:");
		if (ImGui::TreeNodeEx((std::to_string(meshList.size()) + " meshe(s), " +
			std::to_string(CountVerts()) + " vert(s)").c_str(), ImGuiTreeNodeFlags_DefaultOpen)) {
			for (auto &iter : meshList) {
				ImGui::Columns(2, "mycolumns", false);
				ImGui::AlignTextToFramePadding();
				ImGui::Text(Icon_Name_To_ID(ICON_FA_PUZZLE_PIECE, " " + iter->name)); ImGui::NextColumn();
				//ImGui::Text(Icon_Name_To_ID(ICON_FA_GLOBE, " " + iter->material->name));
				auto width = ImGui::GetContentRegionAvailWidth();
				if (ButtonEx::DragAndDropButton(iter->material, SceneManager::GetType(iter->material).c_str(), 
					Icon_Name_To_ID(ICON_FA_GLOBE, " " + iter->material->name), ImVec2(width, 22), iter->material->ID)) {
					MOON_InputManager::Select_Append(iter->material->ID);
				}
				ImGui::Columns(1);
			}
			ImGui::TreePop();
		}
		ImGui::Separator();

		// list operators -----------------------------------------------------------------
		opstack.ListStacks();
		ImGui::Spacing();
	}

	void Model::ClearSelection() {
		MOON_InputManager::Selector::ClearSelectionPrototype(meshList, selected_meshes);
	}

	void Model::Select(const unsigned int ID) {
		MOON_InputManager::Selector::SelectPrototype(meshList, selected_meshes, ID);
	}

	void Model::Select_Append(unsigned int ID, const bool& autoInvertSelect) {
		MOON_InputManager::Selector::Select_AppendPrototype(meshList, selected_meshes, ID, autoInvertSelect);
	}

	bool Model::Hit(const Ray &r, HitRecord &rec) const {
		HitRecord tempRec; tempRec.t = rec.t;
		bool hitAnything = false;

		if (Renderer::acc == Renderer::acc_NONE) {
			Ray tray(transform.worldToLocalMat.multVec(r.pos), transform.worldToLocalMat.multDir(r.dir));
			auto normalMat = transform.worldToLocalMat.transposed();
			for (auto &iter : meshList) {
				if (iter->Hit(tray, tempRec)) {
					hitAnything = true;
					// surface properties are returned in local space, 
					// so we need to transform them back to world space
					tempRec.p = transform.localToWorldMat.multVec(tempRec.p);
					tempRec.normal = Vector3::Normalize(normalMat.multDir(tempRec.normal));
					rec = tempRec;
				}
			}
		} else if (Renderer::acc == Renderer::acc_AABB) {
			if (bbox.intersect(r)) {
				Ray tray(transform.worldToLocalMat.multVec(r.pos), transform.worldToLocalMat.multDir(r.dir));
				auto normalMat = transform.worldToLocalMat.transposed();
				for (auto &iter : meshList) {
					if (iter->bbox.intersect(tray)) {
						if (iter->Hit(tray, tempRec)) {
							hitAnything = true;
							tempRec.p = transform.localToWorldMat.multVec(tempRec.p);
							tempRec.normal = Vector3::Normalize(normalMat.multDir(tempRec.normal));
							rec = tempRec;
						}
					}
				}
			}
		} else { // BVH
			Ray tray(transform.worldToLocalMat.multVec(r.pos), transform.worldToLocalMat.multDir(r.dir));
			auto normalMat = transform.worldToLocalMat.transposed();
			for (auto &iter : meshList) {
				if (iter->bbox.intersect(tray)) {
					if (iter->localBVH->Hit(tray, tempRec)) {
						hitAnything = true;
						tempRec.p = transform.localToWorldMat.multVec(tempRec.p);
						tempRec.normal = Vector3::Normalize(normalMat.multDir(tempRec.normal));
						rec = tempRec;
					}
				}
			}
		}

		return hitAnything;
	}

}