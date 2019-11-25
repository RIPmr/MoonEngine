#pragma once
#include <imgui.h>
#include <string>

namespace MOON {
	// stack window(delegation)
	static void RegistStackWnd(const std::string& wndName) {
		ImGui::OpenPopup(wndName.c_str());
	}

	template<class T>
	static void StackWnd(const std::string& wndName, const char* content, T* tar, void (T::*deleMethod)()) {
		bool dummy_open = true;
		if (ImGui::BeginPopupModal("QueryWnd", &dummy_open)) {
			ImGui::Text(content);
			if (ImGui::Button("Yes")) {
				(tar->*deleMethod)();
				ImGui::CloseCurrentPopup();
			}
			ImGui::SameLine();
			if (ImGui::Button("No")) ImGui::CloseCurrentPopup();
			ImGui::EndPopup();
		}
	}

}