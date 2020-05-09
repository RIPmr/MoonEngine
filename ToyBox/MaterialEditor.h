#pragma once
#include "NodeEditorBase.h"

namespace MOON {

	class MaterialEditor : public NodeEditor {
	public:
		MaterialEditor();
		~MaterialEditor() = default;

		// TODO : clear canvas
		void ClearEditor() {
			std::cout << "node editor cleared." << std::endl;
		}

		void PopMenu() override {
			// Numerics ----------------------------------
			if (ImGui::BeginMenu("Numerics")) {
				ListSlots("Numeric");
				ImGui::EndMenu();
			}

			// Operators ---------------------------------
			if (ImGui::BeginMenu("Operators")) {
				ListSlots("Operator");
				ImGui::EndMenu();
			}

			// Procedural Texture ------------------------
			if (ImGui::BeginMenu("Procedural")) {
				ListSlots("Procedural");
				ImGui::EndMenu();
			}

			ImGui::Separator();
			// Mtls --------------------------------------
			ListSlots("Material");

			// Other funcs -------------------------------
			if (anythingSelected) {
				ImGui::Separator();
				if (ImGui::MenuItem("Delete")) {}
				if (ImGui::MenuItem("Copy")) {}
				if (ImGui::MenuItem("Paste")) {}
			}

			//if (ImGui::MenuItem("Reset Zoom")) canvas.zoom = 1;
			//if (ImGui::IsAnyMouseDown() && !ImGui::IsWindowHovered()) ImGui::CloseCurrentPopup();
		}
	};

}