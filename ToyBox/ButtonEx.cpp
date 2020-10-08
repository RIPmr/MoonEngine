#include "Utility.h"
#include "WinDiagHandler.h"
#include "SceneMgr.h"
#include "ButtonEx.h"
#include "ObjectBase.h"
#include "Texture.h"
#include "Model.h"
#include "MoonEnums.h"

namespace MOON {
	std::string ButtonEx::FileButton(const char* label, const ImVec2& size_arg, const int ID) {
		if (ImGui::Button(label, size_arg, ID)) {
			return WinDiagMgr::FileDialog();
		}
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

	bool ButtonEx::TexFileBtnWithPrev(Texture*& tex, const int& type, const ImVec2& size_arg, const int ID) {
		if (ImGui::Button((tex == nullptr ? "[Texture]" : tex->name).c_str(), 
			ImVec2(size_arg.x - (tex == nullptr ? 0 : 32), size_arg.y), ID)) {
			std::string path = WinDiagMgr::FileDialog();
			if (!path._Equal("")) {
				if (tex != nullptr) MOON_TextureManager::DeleteItem(tex);
				tex = MOON_TextureManager::LoadTexture(path);
				tex->type = (TexType)type;
				return true;
			}
		}
		if (tex != nullptr) {
			if (ImGui::IsItemHovered()) {
				ImGui::BeginTooltip();
				ClampedImage(tex, 100.0f);
				ImGui::EndTooltip();
			}
			ImGui::SameLine();
			if (ImGui::Button(ICON_FA_TIMES, ImVec2(22, 22))) {
				MOON_TextureManager::DeleteItem(tex);
				tex = nullptr;
				return true;
			}
		}
		return false;
	}

	void ButtonEx::ClampedImage(Texture* tex, const float& clampSize, const bool& clampWidth, const ImVec2& uv0, const ImVec2& uv1, const ImVec4& tint_col, const ImVec4& border_col) {
		if (clampWidth) ImGui::Image((void*)(intptr_t)tex->localID, ImVec2(clampSize, tex->height * clampSize / tex->width), uv0, uv1, tint_col, border_col);
		else ImGui::Image((void*)(intptr_t)tex->localID, ImVec2(tex->width * clampSize / tex->height, clampSize), uv0, uv1, tint_col, border_col);
	}

}