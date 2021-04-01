#include "Utility.h"
#include "WinDiagHandler.h"
#include "SceneMgr.h"
#include "ButtonEx.h"
#include "ObjectBase.h"
#include "Texture.h"
#include "Model.h"
#include "MoonEnums.h"

namespace MOON {

#pragma region file_button
	bool ButtonEx::FileButton(std::string& label, const ImVec2& size_arg, const int ID) {
		if (ImGui::Button(label.c_str(), size_arg, ID)) {
			label = WinDiagMgr::FileDialog();
			return true;
		} else return false;
	}

	void* ButtonEx::FileButtonEx(void** container, const char* label, const ImVec2& size_arg, const int ID) {
		if (ImGui::Button(label, size_arg, ID)) {
			std::string path = WinDiagMgr::FileDialog();
			if (!path._Equal("")) {
				ObjectBase* base = (ObjectBase*)*container;
				//void* result = nullptr;

				if (base != nullptr) {
					if (CheckType(base, "Texture")) {
						dynamic_cast<Texture*>(base)->Replace(path);
					} else {
						// TODO
					}
				}

				//*container = result;
				return base;
			}
		}
		return nullptr;
	}

	bool ButtonEx::TexFileBtnWithPrev(Texture*& tex, const ImVec2& size_arg, const int ID) {
		if (ImGui::Button((tex == nullptr ? "[Texture]" : tex->name).c_str(), 
			ImVec2(size_arg.x - (tex == nullptr ? 0 : 32), size_arg.y), ID)) {
			std::string path = WinDiagMgr::FileDialog();
			if (!path._Equal("")) {
				if (tex != nullptr) MOON_TextureManager::DeleteItem(tex);
				tex = MOON_TextureManager::LoadTexture(path);
				return true;
			}
		}

		// drag & drop
		MakeDragAndDropWidget<Texture>(tex, "Texture", tex ? tex->name.c_str() : "[Texture]");

		// preview
		if (tex != nullptr) {
			if (ImGui::IsItemHovered() && tex->localID) {
				ImGui::BeginTooltip();
				ClampedImage(tex, 100.0f);
				ImGui::EndTooltip();
			}
			ImGui::SameLine();
			if (ImGui::Button(ICON_FA_TIMES, ImVec2(22, 22))) {
				//MOON_TextureManager::DeleteItem(tex);
				tex = nullptr;
				return true;
			}
		}
		return false;
	}
#pragma endregion

#pragma region extensions
	void ButtonEx::ClampedImage(Texture* tex, const float& clampSize, const bool& clampWidth, const ImVec2& uv0, const ImVec2& uv1, const ImVec4& tint_col, const ImVec4& border_col) {
		if (clampWidth) ImGui::Image((void*)(intptr_t)tex->localID, ImVec2(clampSize, tex->height * clampSize / tex->width), uv0, uv1, tint_col, border_col);
		else ImGui::Image((void*)(intptr_t)tex->localID, ImVec2(tex->width * clampSize / tex->height, clampSize), uv0, uv1, tint_col, border_col);
	}
	void ButtonEx::ClampedImage(unsigned int texID, const Vector2& texSize, const float& clampSize, const bool& clampWidth, const ImVec2& uv0, const ImVec2& uv1, const ImVec4& tint_col, const ImVec4& border_col) {
		if (clampWidth) ImGui::Image((void*)(intptr_t)texID, ImVec2(clampSize, texSize.y * clampSize / texSize.x), uv0, uv1, tint_col, border_col);
		else ImGui::Image((void*)(intptr_t)texID, ImVec2(texSize.x * clampSize / texSize.y, clampSize), uv0, uv1, tint_col, border_col);
	}

	void ButtonEx::SetButtonClicked() {
		ImGui::GetStyle().Colors[ImGuiCol_Button] = ImVec4(1.000f, 1.000f, 1.000f, 0.250f);
	}

	void ButtonEx::ResetButtonColor() {
		ImGui::GetStyle().Colors[ImGuiCol_Button] = ImVec4(1.000f, 1.000f, 1.000f, 0.000f);
	}

	void ButtonEx::ToggleButton(const char* str_id, bool* v) {
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

	bool ButtonEx::SwitchButton(const char* ON_Label, const char* OFF_Label, bool& switcher, const ImVec2 btnSize) {
		if (switcher) {
			ButtonEx::SetButtonClicked();
			if (ImGui::Button(ON_Label, btnSize)) switcher = false;
			ButtonEx::ResetButtonColor();
		} else {
			if (ImGui::Button(OFF_Label, btnSize)) switcher = true;
		}
		return switcher;
	}

	void ButtonEx::RatioButton(const char* label, const bool& switcher,
		void(*executer)(), const ImVec2 btnSize) {
		if (switcher) ButtonEx::SetButtonClicked();
		if (ImGui::Button(label, btnSize)) (*executer)();
		if (switcher) ButtonEx::ResetButtonColor();
	}
#pragma endregion

#pragma region imgui_input_encapsulate
	bool ButtonEx::ComboNoLabel(const char* id, int* current_item, const char* const items[], int items_count, int popup_max_height_in_items) {
		ImGui::PushID(id);
		auto ret = ImGui::Combo("", current_item, items, items_count, popup_max_height_in_items);
		ImGui::PopID();
		return ret;
	}
	
	bool ButtonEx::SliderFloatNoLabel(const char* id, float* v, float v_min, float v_max, const char* format, float power) {
		ImGui::PushID(id);
		auto ret = ImGui::SliderFloat("", v, v_min, v_max, format, power);
		ImGui::PopID();
		return ret;
	}

	bool ButtonEx::SliderFloat2NoLabel(const char* id, float v[2], float v_min, float v_max, const char* format, float power) {
		ImGui::PushID(id);
		auto ret = ImGui::SliderFloat2("", v, v_min, v_max, format, power);
		ImGui::PopID();
		return ret;
	}

	bool ButtonEx::SliderFloat3NoLabel(const char* id, float v[3], float v_min, float v_max, const char* format, float power) {
		ImGui::PushID(id);
		auto ret = ImGui::SliderFloat3("", v, v_min, v_max, format, power);
		ImGui::PopID();
		return ret;
	}

	bool ButtonEx::SliderFloat4NoLabel(const char* id, float v[4], float v_min, float v_max, const char* format, float power) {
		ImGui::PushID(id);
		auto ret = ImGui::SliderFloat4("", v, v_min, v_max, format, power);
		ImGui::PopID();
		return ret;
	}

	bool ButtonEx::DragFloatNoLabel(const char* id, float* v, float v_speed, float v_min, float v_max, const char* format, float power) {
		ImGui::PushID(id);
		auto ret = ImGui::DragFloat("", v, v_speed, v_min, v_max, format, power);
		ImGui::PopID();
		return ret;
	}

	bool ButtonEx::DragIntNoLabel(const char* id, int* v, float v_speed, int v_min, int v_max, const char* format) {
		ImGui::PushID(id);
		auto ret = ImGui::DragInt("", v, v_speed, v_min, v_max, format);
		ImGui::PopID();
		return ret;
	}

	bool ButtonEx::DragVec2NoLabel(const char* id, float v[2], float v_speed, float v_min, float v_max, const char* format, float power) {
		ImGui::PushID(id);
		auto ret = ImGui::DragFloat2("", v, v_speed, v_min, v_max, format, power);
		ImGui::PopID();
		return ret;
	}

	bool ButtonEx::DragVec3NoLabel(const char* id, float v[3], float v_speed, float v_min, float v_max, const char* format, float power) {
		ImGui::PushID(id);
		auto ret = ImGui::DragFloat3("", v, v_speed, v_min, v_max, format, power);
		ImGui::PopID();
		return ret;
	}

	bool ButtonEx::DragVec4NoLabel(const char* id, float v[4], float v_speed, float v_min, float v_max, const char* format, float power) {
		ImGui::PushID(id);
		auto ret = ImGui::DragFloat4("", v, v_speed, v_min, v_max, format, power);
		ImGui::PopID();
		return ret;
	}

	bool ButtonEx::InputIntNoLabel(const char* id, int* v, int step, int step_fast, ImGuiInputTextFlags flags) {
		ImGui::PushID(id);
		auto ret = ImGui::InputInt("", v, step, step_fast, flags);
		ImGui::PopID();
		return ret;
	}

	bool ButtonEx::InputFloatNoLabel(const char* id, float* v, float step, float step_fast, const char* format, ImGuiInputTextFlags flags) {
		ImGui::PushID(id);
		auto ret = ImGui::InputFloat("", v, step, step_fast, format, flags);
		ImGui::PopID();
		return ret;
	}

	bool ButtonEx::CheckboxNoLabel(const char* id, bool* v) {
		ImGui::PushID(id);
		auto ret = ImGui::Checkbox("", v);
		ImGui::PopID();
		return ret;
	}

	bool ButtonEx::ColorEdit3NoLabel(const char* id, float col[3], ImGuiColorEditFlags flags) {
		ImGui::PushID(id);
		auto ret = ImGui::ColorEdit3("", col, flags);
		ImGui::PopID();
		return ret;
	}

	bool ButtonEx::ColorEdit4NoLabel(const char* id, float col[4], ImGuiColorEditFlags flags) {
		ImGui::PushID(id);
		auto ret = ImGui::ColorEdit4("", col, flags);
		ImGui::PopID();
		return ret;
	}

#pragma endregion

}