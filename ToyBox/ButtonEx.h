#pragma once
#include <imgui.h>
#include <string>
#include <iostream>

#include "Icons.h"
#include "imgui_internal.h"

namespace MOON {
	extern class Texture;
	class ButtonEx {
	public:
		static bool FileButton(std::string& label, const ImVec2& size_arg, const int ID = -1);

		static void* FileButtonEx(void** container, const char* label, const ImVec2& size_arg, const int ID = -1);
	
		static bool TexFileBtnWithPrev(Texture*& ref, const int& type, const ImVec2& size_arg, const int ID = -1);

		static bool ComboNoLabel(const char* id, int* current_item, const char* const items[], int items_count, int popup_max_height_in_items = -1);

		static bool DragFloatNoLabel(const char* id, float* v, float v_speed = 1.0f, float v_min = 0.0f, float v_max = 0.0f, const char* format = "%.3f", float power = 1.0f);

		static bool DragIntNoLabel(const char* id, int* v, float v_speed = 1.0f, int v_min = 0, int v_max = 0, const char* format = "%d");

		static bool DragVec2NoLabel(const char* id, float v[2], float v_speed = 1.0f, float v_min = 0.0f, float v_max = 0.0f, const char* format = "%.3f", float power = 1.0f);

		static bool DragVec3NoLabel(const char* id, float v[3], float v_speed = 1.0f, float v_min = 0.0f, float v_max = 0.0f, const char* format = "%.3f", float power = 1.0f);

		static bool DragVec4NoLabel(const char* id, float v[4], float v_speed = 1.0f, float v_min = 0.0f, float v_max = 0.0f, const char* format = "%.3f", float power = 1.0f);

		static bool InputIntNoLabel(const char* id, int* v, int step = 1, int step_fast = 100, ImGuiInputTextFlags flags = 0);
		
		static bool InputFloatNoLabel(const char* label, float* v, float step = 0.0f, float step_fast = 0.0f, const char* format = "%.3f", ImGuiInputTextFlags flags = 0);

		static bool CheckboxNoLabel(const char* id, bool* v);

		static bool ColorEdit3NoLabel(const char* id, float col[3], ImGuiColorEditFlags flags = 0);
		
		static bool ColorEdit4NoLabel(const char* id, float col[4], ImGuiColorEditFlags flags = 0);

		static bool SliderFloatNoLabel(const char* id, float* v, float v_min, float v_max, const char* format = "%.3f", float power = 1.0f);

		static void ClampedImage(Texture* tex, const float& clampSize, const bool& clampWidth = true, const ImVec2& uv0 = ImVec2(0, 0), const ImVec2& uv1 = ImVec2(1, 1), const ImVec4& tint_col = ImVec4(1, 1, 1, 1), const ImVec4& border_col = ImVec4(0, 0, 0, 0));
		static void ClampedImage(unsigned int texID, const Vector2& texSize, const float& clampSize, const bool& clampWidth = true, const ImVec2& uv0 = ImVec2(0, 0), const ImVec2& uv1 = ImVec2(1, 1), const ImVec4& tint_col = ImVec4(1, 1, 1, 1), const ImVec4& border_col = ImVec4(0, 0, 0, 0));
	
		template<class T>
		static void MakeDragAndDropWidget(T* &payload_in, const char* type, const char* label, void(*dropable)(T*&, T*&) = nullptr) {
			if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceNoPreviewTooltip)) {
				ImGui::SetDragDropPayload(type, &payload_in, sizeof(T*));
				ImGui::SetTooltip(label);
				ImGui::EndDragDropSource();
			}
			if (dropable != nullptr && ImGui::BeginDragDropTarget()) {
				if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(type)) {
					IM_ASSERT(payload->DataSize == sizeof(T*));
					T* payload_m = *(T**)payload->Data;
					dropable(payload_in, payload_m);
				}
				ImGui::EndDragDropTarget();
			}
		}

		template<class T>
		static bool DragAndDropButton(T* &payload_in, const char* type, const char* label, const ImVec2& size_arg = ImVec2(0, 0), const int ID = -1) {
			bool flag = ImGui::Button(label, size_arg, ID);

			if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceNoPreviewTooltip)) {
				// Set payload to carry the index of our item (could be anything)
				ImGui::SetDragDropPayload(type, &payload_in, sizeof(T*));
				ImGui::SetTooltip(label);
				ImGui::EndDragDropSource();
			}

			if (ImGui::BeginDragDropTarget()) {
				if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload(type)) {
					IM_ASSERT(payload->DataSize == sizeof(T*));
					T* payload_m = *(T**)payload->Data;
					payload_in = payload_m;
				}
				ImGui::EndDragDropTarget();
			}

			return flag;
		}

		// style event
		static void SetButtonClicked();

		static void ResetButtonColor();

		static void ToggleButton(const char* str_id, bool* v);

		static bool SwitchButton(const char* ON_Label, const char* OFF_Label, bool& switcher, const ImVec2 btnSize = ImVec2(22, 22));

		static void RatioButton(const char* label, const bool& switcher, void(*executer)(), const ImVec2 btnSize = ImVec2(22, 22));

		template <typename F1, typename F2>
		static void SwitchButtonEx(const char* ON_Label, const char* OFF_Label, const bool switcher,
			F1 ON_Execute, F2 OFF_Execute, const ImVec2 btnSize = ImVec2(22, 22)) {
			if (switcher) {
				ButtonEx::SetButtonClicked();
				if (ImGui::Button(ON_Label, btnSize)) ON_Execute();
				ButtonEx::ResetButtonColor();
			} else {
				if (ImGui::Button(OFF_Label, btnSize)) OFF_Execute();
			}
		}
	};

}