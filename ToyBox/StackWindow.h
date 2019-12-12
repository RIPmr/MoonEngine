#pragma once
#include <imgui.h>
#include <string>

#include "IconsFontAwesome4.h"
#define Icon_Name_To_ID(x, y) (std::string(x) + y).c_str()

namespace MOON {
	enum StackWndType {
		QUERY,
		PROGRESS
	};

	static bool regQuery = false;
	static bool regProgress = false;

	// stack window(delegation)
	static void RegistStackWnd(const std::string& wndName, const StackWndType type = StackWndType::QUERY) {
		switch (type) {
			case StackWndType::QUERY:
				regQuery = true; break;
			case StackWndType::PROGRESS:
				regProgress = true; break;
			default:
				return;
		}
	}

	template<class T>
	static void QueryWnd(const std::string& wndName, const std::string& content, T* tar, void (T::*deleMethod)()) {
		bool dummy_open = true;
		if (regQuery) {
			ImGui::OpenPopup(Icon_Name_To_ID(ICON_FA_EXCLAMATION_TRIANGLE, " " + wndName));
			regQuery = false;
		}
		if (ImGui::BeginPopupModal(Icon_Name_To_ID(ICON_FA_EXCLAMATION_TRIANGLE, " " + wndName), &dummy_open, ImGuiWindowFlags_NoResize)) {
			ImVec2 size = ImGui::CalcTextSize(content.c_str());
			ImGui::SetWindowSize(ImVec2(size.x + 30.0f, size.y + 65.0f));
			ImGui::Text(content.c_str());
			if (ImGui::Button("Yes")) {
				(tar->*deleMethod)();
				ImGui::CloseCurrentPopup();
			}
			ImGui::SameLine();
			if (ImGui::Button("No")) ImGui::CloseCurrentPopup();
			ImGui::EndPopup();
		}
	}

	static void ProgressWnd(const std::string& wndName, const std::string* content, const float* progress) {
		if (regProgress) {
			ImGui::OpenPopup(Icon_Name_To_ID(ICON_FA_HOURGLASS_HALF, " " + wndName));
			regProgress = false;
		}
		if (ImGui::BeginPopupModal(Icon_Name_To_ID(ICON_FA_HOURGLASS_HALF, " " + wndName), 0, ImGuiWindowFlags_NoResize)) {
			ImGui::SetWindowSize(ImVec2(200.0f, 80.0f));
			ImGui::Text((*content).c_str());
			ImGui::ProgressBar(*progress);
			if (*progress >= 1.0f) ImGui::CloseCurrentPopup();
			ImGui::EndPopup();
		}
	}

}