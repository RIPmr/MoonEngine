#pragma once
#include <imgui.h>
#include <string>
#include <iostream>

#include "imgui_internal.h"
#include "IconsFontAwesome4.h"

namespace MOON {
	// style event
	static void SetButtonClicked() {
		ImGui::GetStyle().Colors[ImGuiCol_Button] = ImVec4(1.000f, 1.000f, 1.000f, 0.250f);
	}

	static void ResetButtonColor() {
		ImGui::GetStyle().Colors[ImGuiCol_Button] = ImVec4(1.000f, 1.000f, 1.000f, 0.000f);
	}

	static void ToggleButton(const char* str_id, bool* v) {
		ImVec2 p = ImGui::GetCursorScreenPos();
		ImDrawList* draw_list = ImGui::GetWindowDrawList();

		float height = ImGui::GetFrameHeight();
		float width = height * 1.55f;
		float radius = height * 0.50f;

		ImGui::InvisibleButton(str_id, ImVec2(width, height));
		if (ImGui::IsItemClicked())
			*v = !*v;

		float t = *v ? 1.0f : 0.0f;

		ImGuiContext& g = *GImGui;
		float ANIM_SPEED = 0.08f;
		if (g.LastActiveId == g.CurrentWindow->GetID(str_id))// && g.LastActiveIdTimer < ANIM_SPEED)
		{
			float t_anim = ImSaturate(g.LastActiveIdTimer / ANIM_SPEED);
			t = *v ? (t_anim) : (1.0f - t_anim);
		}

		ImU32 col_bg;
		if (ImGui::IsItemHovered())
			col_bg = ImGui::GetColorU32(ImLerp(ImVec4(0.78f, 0.78f, 0.78f, 1.0f), ImVec4(0.64f, 0.83f, 0.34f, 1.0f), t));
		else
			col_bg = ImGui::GetColorU32(ImLerp(ImVec4(0.85f, 0.85f, 0.85f, 1.0f), ImVec4(0.56f, 0.83f, 0.26f, 1.0f), t));

		draw_list->AddRectFilled(p, ImVec2(p.x + width, p.y + height), col_bg, height * 0.5f);
		draw_list->AddCircleFilled(ImVec2(p.x + radius + t * (width - radius * 2.0f), p.y + radius), radius - 1.5f, IM_COL32(255, 255, 255, 255));
	}

	static bool SwitchButton(const char* ON_Label, const char* OFF_Label, bool& switcher, const ImVec2 btnSize = ImVec2(22, 22)) {
		if (switcher) {
			SetButtonClicked();
			if (ImGui::Button(ON_Label, btnSize)) switcher = false;
			ResetButtonColor();
		} else {
			if (ImGui::Button(OFF_Label, btnSize)) switcher = true;
		}
		return switcher;
	}

	static void SwitchButtonEx(const char* ON_Label, const char* OFF_Label, const bool& switcher, 
		void(*ON_Execute)(), void(*OFF_Execute)(), const ImVec2 btnSize = ImVec2(22, 22)) {
		if (switcher) {
			if (ImGui::Button(ON_Label, btnSize) && *ON_Execute != nullptr) (*ON_Execute)();
		} else {
			SetButtonClicked();
			if (ImGui::Button(OFF_Label, btnSize) && *OFF_Execute != nullptr) (*OFF_Execute)();
			ResetButtonColor();
		}
	}

	static void RatioButton(const char* label, const bool& switcher, 
		void(*executer)(), const ImVec2 btnSize = ImVec2(22, 22)) {
		if (switcher) SetButtonClicked();
		if (ImGui::Button(label, btnSize)) (*executer)();
		if (switcher) ResetButtonColor();
	}
}