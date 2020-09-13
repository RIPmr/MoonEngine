#include "FlowMenu.h"
#include "HotkeyMgr.h"
#include "Gizmo.h"

namespace MOON {
	void MOON_FlowMenu::FlowMenu() {
		if (ImGui::BeginPiePopup("FlowMenu", 1)) {
			if (ImGui::PieMenuItem("Freeze")) std::cout << "Freeze" << std::endl;
			if (ImGui::PieMenuItem("Property")) std::cout << "Property" << std::endl;
			if (ImGui::PieMenuItem("Isolate", false)) std::cout << "Isolate" << std::endl;

			if (ImGui::BeginPieMenu("Gizmo")) {
				if (ImGui::PieMenuItem(Gizmo::gizmoPos == GizmoPos::pivot ? "[Pivot]" : "Pivot")) Gizmo::gizmoPos = GizmoPos::pivot;
				if (ImGui::PieMenuItem(Gizmo::gizmoPos == GizmoPos::center ? "[Center]" : "Center")) Gizmo::gizmoPos = GizmoPos::center;
				/*if (ImGui::BeginPieMenu("Other")) {
					if (ImGui::PieMenuItem("Sub")) std::cout << "SubSub" << std::endl;
					if (ImGui::BeginPieMenu("Other")) {
						if (ImGui::PieMenuItem("Sub")) std::cout << "SubSub" << std::endl;
						if (ImGui::PieMenuItem("Sub2")) std::cout << "SubSub2" << std::endl;
						if (ImGui::BeginPieMenu("Other")) {
							if (ImGui::PieMenuItem("Sub")) std::cout << "SubSub" << std::endl;
							if (ImGui::PieMenuItem("Sub2")) std::cout << "SubSub2" << std::endl;
							ImGui::EndPieMenu();
						}
						if (ImGui::PieMenuItem("Sub3")) std::cout << "SubSub3" << std::endl;
						if (ImGui::PieMenuItem("Sub4")) std::cout << "SubSub4" << std::endl;
						ImGui::EndPieMenu();
					}
					if (ImGui::PieMenuItem("Sub2")) std::cout << "SubSub2" << std::endl;
					ImGui::EndPieMenu();
				}*/
				ImGui::EndPieMenu();
			}

			if (ImGui::BeginPieMenu("Coord")) {
				if (ImGui::PieMenuItem(Gizmo::manipCoord == CoordSys::WORLD ? "[World]" : "World")) Gizmo::manipCoord = CoordSys::WORLD;
				if (ImGui::PieMenuItem(Gizmo::manipCoord == CoordSys::LOCAL ? "[Local]" : "Local")) Gizmo::manipCoord = CoordSys::LOCAL;
				if (ImGui::PieMenuItem(Gizmo::manipCoord == CoordSys::PARENT ? "[Parent]" : "Parent")) Gizmo::manipCoord = CoordSys::PARENT;
				if (ImGui::PieMenuItem(Gizmo::manipCoord == CoordSys::SCREEN ? "[Screen]" : "Screen")) Gizmo::manipCoord = CoordSys::SCREEN;
				ImGui::EndPieMenu();
			}

			if (ImGui::BeginPieMenu("Snap")) {
				auto oldType = HotKeyManager::snapType;
				if (ImGui::PieMenuItem(HotKeyManager::snapType == vertex ? "[Vert]" : "Vert")) {
					HotKeyManager::snapType = vertex;
					if (oldType == vertex && HotKeyManager::enableSnap) HotKeyManager::enableSnap = false;
					else HotKeyManager::enableSnap = true;
				}
				if (ImGui::PieMenuItem(HotKeyManager::snapType == edge ? "[Edge]" : "Edge")) {
					HotKeyManager::snapType = edge;
					if (oldType == edge && HotKeyManager::enableSnap) HotKeyManager::enableSnap = false;
					else HotKeyManager::enableSnap = true;
				}
				if (ImGui::PieMenuItem(HotKeyManager::snapType == face ? "[Face]" : "Face")) {
					HotKeyManager::snapType = face;
					if (oldType == face && HotKeyManager::enableSnap) HotKeyManager::enableSnap = false;
					else HotKeyManager::enableSnap = true;
				}
				if (ImGui::PieMenuItem(HotKeyManager::snapType == grid ? "[Grid]" : "Grid")) {
					HotKeyManager::snapType = grid;
					if (oldType == grid && HotKeyManager::enableSnap) HotKeyManager::enableSnap = false;
					else HotKeyManager::enableSnap = true;
				}
				ImGui::EndPieMenu();
			}

			ImGui::EndPiePopup();
		}
	}

	void MOON_FlowMenu::QuadMenu() {
		if (ImGui::BeginPopup("QuadMenu")) {
			if (ImGui::BeginMenu("CoordSys")) {
				if (ImGui::MenuItem("World"))  Gizmo::manipCoord = CoordSys::WORLD;
				if (ImGui::MenuItem("Local"))  Gizmo::manipCoord = CoordSys::LOCAL;
				if (ImGui::MenuItem("Parent")) Gizmo::manipCoord = CoordSys::PARENT;
				if (ImGui::MenuItem("Screen")) Gizmo::manipCoord = CoordSys::SCREEN;
				ImGui::EndMenu();
			}
			if (ImGui::BeginMenu("GizmoPos")) {
				if (ImGui::MenuItem("Pivot"))  Gizmo::gizmoPos = GizmoPos::pivot;
				if (ImGui::MenuItem("Center")) Gizmo::gizmoPos = GizmoPos::center;
				ImGui::EndMenu();
			}

			ImGui::Separator();
			if (ImGui::MenuItem("Freeze")) {}
			if (ImGui::MenuItem("Property")) {}
			if (ImGui::MenuItem("Isolate")) {}

			ImGui::EndPopup();
		}
	}
}