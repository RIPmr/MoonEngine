#pragma once
#include <imgui.h>
#include <imgui_internal.h>

namespace ImGui {
	bool BeginPiePopup(const char* pName, int iMouseButton = 0);
	void EndPiePopup();

	bool PieMenuItem(const char* pName, bool bEnabled = true);
	bool BeginPieMenu(const char* pName, bool bEnabled = true);
	void EndPieMenu();
}