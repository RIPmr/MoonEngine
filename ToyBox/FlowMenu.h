#pragma once
#include <iostream>

#include "ImPie.h"
#include "Gizmo.h"

namespace MOON {
	// Legacy right-click menu
	static void QuadMenu() {
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
				if (ImGui::MenuItem("Center"))  Gizmo::gizmoPos = GizmoPos::center;
				ImGui::EndMenu();
			}

			ImGui::Separator();
			if (ImGui::MenuItem("Delete")) {}
			if (ImGui::MenuItem("Copy")) {}
			if (ImGui::MenuItem("Paste")) {}

			ImGui::EndPopup();
		}
	}

	static void FlowMenu() {
		if (ImGui::BeginPiePopup("FlowMenu", 1)) {
			if (ImGui::PieMenuItem("Delete")) std::cout << "Delete" << std::endl;
			if (ImGui::PieMenuItem("Copy")) std::cout << "Copy" << std::endl;
			if (ImGui::PieMenuItem("Paste", false)) std::cout << "Paste" << std::endl;

			if (ImGui::BeginPieMenu("Gizmo")) {
				if (ImGui::PieMenuItem("Pivot")) Gizmo::gizmoPos = GizmoPos::pivot;
				if (ImGui::PieMenuItem("Center")) Gizmo::gizmoPos = GizmoPos::center;
				if (ImGui::BeginPieMenu("Other")) {
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
				}
				ImGui::EndPieMenu();
			}

			if (ImGui::BeginPieMenu("Coord")) {
				if (ImGui::PieMenuItem("World")) Gizmo::manipCoord = CoordSys::WORLD;
				if (ImGui::PieMenuItem("Local")) Gizmo::manipCoord = CoordSys::LOCAL;
				if (ImGui::PieMenuItem("Parent")) Gizmo::manipCoord = CoordSys::PARENT;
				if (ImGui::PieMenuItem("Screen")) Gizmo::manipCoord = CoordSys::SCREEN;
				ImGui::EndPieMenu();
			}

			ImGui::EndPiePopup();
		}
	}
}