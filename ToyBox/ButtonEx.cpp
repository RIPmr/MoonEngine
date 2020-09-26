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
				void* result = nullptr;

				if (base != nullptr) {
					if (CheckType(base, "Texture")) {
						MOON_TextureManager::DeleteItem((Texture*)base);
						result = MOON_TextureManager::LoadTexture(path);
					} else if (CheckType(base, "Model")) {
						MOON_ModelManager::DeleteItem((Model*)base);
						result = MOON_ModelManager::LoadModel(path);
					}
				}

				*container = result;
				return result;
			}
		}
		return nullptr;
	}

}