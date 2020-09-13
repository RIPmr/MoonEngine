#include "Model.h"
#include "SceneMgr.h"
#include "MoonEnums.h"

namespace MOON {

	void Model::LoadModel(const std::string& path) {
		OBJLoader::LoadFile(path, meshList, gammaCorrection);
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
		for (int i = 0; i < meshList.size(); i++) {
			meshList[i]->Draw(overrideShader == NULL ? meshList[i]->material->shader :
				overrideShader, transform.localToWorldMat, ID == MOON_InputManager::hoverID, selected);
		}
		if (transform.changeFlag) {
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
		if (path._Equal("[PROCEDURAL]")) {
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
				if (DragAndDropButton(iter->material, SceneManager::GetType(iter->material).c_str(), 
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

}