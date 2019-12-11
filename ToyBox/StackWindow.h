#pragma once
#include <imgui.h>
#include <string>

#include "IconsFontAwesome4.h"
#define Icon_Name_To_ID(x, y) (std::string(x) + y).c_str()

namespace MOON {
	// stack window(delegation)
	static void RegistStackWnd(const std::string& wndName) {
		ImGui::OpenPopup(Icon_Name_To_ID(ICON_FA_EXCLAMATION_TRIANGLE, " " + wndName));
	}

	template<class T>
	static void QueryWnd(const std::string& wndName, const char* content, T* tar, void (T::*deleMethod)()) {
		bool dummy_open = true;
		if (ImGui::BeginPopupModal(Icon_Name_To_ID(ICON_FA_EXCLAMATION_TRIANGLE, " " + wndName), &dummy_open, ImGuiWindowFlags_NoResize)) {
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

	template<class T>
	static void ProgressWnd(const std::string& wndName, const char* content, const float* progress) {
		bool dummy_open = true;
		if (ImGui::BeginPopupModal(Icon_Name_To_ID(ICON_FA_HOURGLASS_HALF, " " + wndName), &dummy_open, ImGuiWindowFlags_NoResize)) {
			ImGui::Text(content);
			ImGui::ProgressBar(*progress);
			if (*progress >= 100.0f) ImGui::CloseCurrentPopup();
			ImGui::EndPopup();
		}
	}

}