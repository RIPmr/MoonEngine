#include "StackWindow.h"
#include "MShader.h"

namespace MOON {
	void Shader::ListProperties() {
		// list name ----------------------------------------------------------------------
		ListName();
		ImGui::Separator();

		// list shaders -------------------------------------------------------------------			ListTransform();
		// vs
		if (ImGui::TreeNode("vert source", ID)) {
			ImGui::Indent(-10.0f);
			ImGui::InputTextMultiline(UniquePropName("vssource"), vsbuf, IM_ARRAYSIZE(vsbuf),
				ImVec2(-FLT_MIN, ImGui::GetTextLineHeight() * 16));
			ImGui::Unindent(-10.0f);
			ImGui::TreePop();
		}
		// fs
		if (ImGui::TreeNode("frag source", ID)) {
			ImGui::Indent(-10.0f);
			ImGui::InputTextMultiline(UniquePropName("fssource"), fsbuf, IM_ARRAYSIZE(fsbuf),
				ImVec2(-FLT_MIN, ImGui::GetTextLineHeight() * 16));
			ImGui::Unindent(-10.0f);
			ImGui::TreePop();
		}

		ImGui::Indent(10.0f);
		if (ImGui::Button("Recompile")) {}
		ImGui::SameLine();
		if (ImGui::Button("Save")) {
			RegistStackWnd("QueryWnd");
		}
		StackWnd("QueryWnd", "Overwrite existing shader files?", this, &Shader::SaveShaders);
		ImGui::SameLine();
		if (ImGui::Button("Revert")) { RevertCodes(); }
		ImGui::Unindent(10.0f);

		ImGui::Spacing();
	}

	void Shader::RevertCodes() {
		std::string vertexCode;
		std::string fragmentCode;

		ReadShaderSource(vertexCode, fragmentCode);

		strcpy(vsbuf, vertexCode.c_str());
		strcpy(fsbuf, fragmentCode.c_str());
	}
}