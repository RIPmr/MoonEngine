#include "StackWindow.h"
#include "CodeEditor.h"
#include "MShader.h"
#include "UIController.h"

namespace MOON {
	void Shader::ListProperties() {
		// list name ----------------------------------------------------------------------
		ListName();
		ImGui::Separator();

		// list shaders -------------------------------------------------------------------
		// vs
		ImGui::PushID("vs");
		if (ImGui::TreeNode("vert source", ID)) {
			ImGui::SameLine(); 
			if (ImGui::SmallButton(ICON_FA_PENCIL_SQUARE_O)) {
				MainUI::show_code_editor = true;
				MainUI::CEditor.LoadFile(vertexPath);
			}
			ImGui::Indent(-10.0f);
			ImGui::InputTextMultiline(UniquePropName("vssource"), vsbuf,
				IM_ARRAYSIZE(vsbuf), ImVec2(-FLT_MIN, ImGui::GetTextLineHeight() * 16));
			ImGui::Unindent(-10.0f);
			ImGui::TreePop();
		} else {
			ImGui::SameLine();
			if (ImGui::SmallButton(ICON_FA_PENCIL_SQUARE_O)) {
				MainUI::show_code_editor = true;
				MainUI::CEditor.LoadFile(vertexPath);
			}
		}
		ImGui::PopID();
		// fs
		ImGui::PushID("fs");
		if (ImGui::TreeNode("frag source", ID)) {
			ImGui::SameLine();
			if (ImGui::SmallButton(ICON_FA_PENCIL_SQUARE_O)) {
				MainUI::show_code_editor = true;
				MainUI::CEditor.LoadFile(fragmentPath);
			}
			ImGui::Indent(-10.0f);
			ImGui::InputTextMultiline(UniquePropName("fssource"), vsbuf,
				IM_ARRAYSIZE(fsbuf), ImVec2(-FLT_MIN, ImGui::GetTextLineHeight() * 16));
			ImGui::Unindent(-10.0f);
			ImGui::TreePop();
		} else {
			ImGui::SameLine();
			if (ImGui::SmallButton(ICON_FA_PENCIL_SQUARE_O)) {
				MainUI::show_code_editor = true;
				MainUI::CEditor.LoadFile(fragmentPath);
			}
		}
		ImGui::PopID();

		ImGui::Indent(10.0f);
		if (ImGui::Button("Recompile")) {}
		ImGui::SameLine();
		if (ImGui::Button("Save")) {
			RegistStackWnd("Information");
		}
		QueryWnd("Information", "Overwrite existing shader files?", this, &Shader::SaveShaders);
		ImGui::SameLine();
		if (ImGui::Button("Revert")) { RevertCodes(); }
		ImGui::Unindent(10.0f);

		ImGui::Spacing();
	}

	void Shader::RevertCodes() {
		std::string vs, fs;
		ReadShaderSource(vs, fs);
		strcpy(vsbuf, vs.c_str());
		strcpy(fsbuf, fs.c_str());
	}
}