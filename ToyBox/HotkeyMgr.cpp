#include <imgui.h>
#include <imgui_internal.h>

#include "Vector2.h"
#include "MoonEnums.h"
#include "HotkeyMgr.h"
#include "SceneMgr.h"
#include "UIController.h"

namespace MOON {
	void HotKeyManager::MOON_InputProcessor(GLFWwindow *window) {
		if (MOON_InputManager::isHoverUI) {

		} else {
			// Create mode --------------------------------------------------------------------
			if (state == CREATE) {
				// do create

				// exit create mode
				if (MOON_InputManager::IsMouseRelease(1)) {
					state = FREE;
				}
			} else if (state == PICK) {

			} else { // state == FREE
				// Right click menu -----------------------------------------------------------
				if (MOON_InputManager::IsMouseDown(1)) MainUI::show_right_click_menu = true;
			}

			// Camera control -----------------------------------------------------------------
			if (MOON_InputManager::mouseScrollOffset.magnitude() > 0) {
				MOON_ActiveCamera->PushCamera(MOON_InputManager::mouseScrollOffset);
			}
			if (MOON_InputManager::mouse_middle_hold) {
				if (glfwGetKey(window, GLFW_KEY_LEFT_ALT) == GLFW_PRESS) {
					if (glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS) {
						MOON_ActiveCamera->ZoomCamera(MOON_InputManager::mouseOffset);
					} else MOON_ActiveCamera->RotateCamera(MOON_InputManager::mouseOffset);
				} else if (MOON_InputManager::mouseOffset.magnitude() > 0) {
					MOON_ActiveCamera->PanCamera(MOON_InputManager::mouseOffset);
				}
			}
			if (MOON_KeyDown(KEY_Z)) {
				if (MOON_InputManager::selection.size() > 0) {
					if (ClassOf(MOON_Selection(0))._Equal("Model"))
						MOON_ActiveCamera->CatchTarget(dynamic_cast<Model*>(MOON_Selection(0)));
					else MOON_ActiveCamera->CatchTarget(NULL);
				} else MOON_ActiveCamera->CatchTarget(NULL);
			}

			// Open Material Editor -----------------------------------------------------------
			if (MOON_KeyDown(KEY_M)) MainUI::show_material_editor = !MainUI::show_material_editor;

			// Open MOON NN Editor ------------------------------------------------------------
			if (MOON_KeyDown(KEY_N)) MainUI::show_nn_manager = !MainUI::show_nn_manager;

			// Gizmo --------------------------------------------------------------------------
			if (MOON_KeyDown(KEY_Q)) Gizmo::gizmoMode = GizmoMode::none;
			if (MOON_KeyDown(KEY_W)) Gizmo::gizmoMode = GizmoMode::translate;
			if (MOON_KeyDown(KEY_E)) Gizmo::gizmoMode = GizmoMode::rotate;
			if (MOON_KeyDown(KEY_R)) Gizmo::gizmoMode = GizmoMode::scale;

			// Delete -------------------------------------------------------------------------
			if (MOON_SelectionID.size()) {
				if (MOON_KeyDown(KEY_DELETE)) {
					auto size = MOON_SelectionID.size();
					for (int i = 0; i < size; i++) SceneManager::DeleteObject(MOON_Selection(i));
					MOON_InputManager::ClearSelection();
				}
			}
		}

		// Functions : F(N) ---------------------------------------------------------------
		auto& shading = SceneManager::splitShading[MOON_ActiveView];
		if (MOON_KeyDown(KEY_F2)) shading = shading == FACET ? DEFAULT : FACET;
		if (MOON_KeyDown(KEY_F3)) shading = shading == WIRE ? DEFAULT : WIRE;
		if (MOON_KeyDown(KEY_F4)) shading = shading == DEFWIRE ? DEFAULT : DEFWIRE;

		// Escape -------------------------------------------------------------------------
		if (MOON_KeyDown(KEY_ESCAPE)) glfwSetWindowShouldClose(window, true);

		// Get Control Keys ---------------------------------------------------------------
		MOON_InputManager::left_ctrl_hold = glfwGetKey(window, GLFW_KEY_LEFT_CONTROL) == GLFW_PRESS;
		MOON_InputManager::left_shift_hold = glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS;
		MOON_InputManager::right_ctrl_hold = glfwGetKey(window, GLFW_KEY_RIGHT_CONTROL) == GLFW_PRESS;
		MOON_InputManager::right_shift_hold = glfwGetKey(window, GLFW_KEY_RIGHT_SHIFT) == GLFW_PRESS;

		// reset offset -------------------------------------------------------------------
		MOON_InputManager::mouseOffset.setValue(0.0f, 0.0f);
		MOON_InputManager::mouseScrollOffset.setValue(0.0f, 0.0f);

		// exit ---------------------------------------------------------------------------
		if (SceneManager::exitFlag) glfwSetWindowShouldClose(window, true);
	}

	void HotKeyManager::SelectRegion(const SelectType& type) {
		static ImVec2 start{ -1, -1 }, end{ -1, -1 };

		if (MOON_InputManager::isHoverUI || Gizmo::hoverGizmo || HotKeyManager::state == CREATE) {
			start = end = ImVec2{ -1, -1 };
			return;
		}

		auto draw_list = ImGui::GetWindowDrawList();

		if (MOON_MouseDown(0)) {
			start.x = ImGui::GetWindowPos().x + MOON_MousePos.x + 7;
			start.y = ImGui::GetWindowPos().y + MOON_MousePos.y + 7;
		} else if (MOON_MouseRepeat(0)) {
			if (start.x < 0) return;
			end.x = ImGui::GetWindowPos().x + MOON_MousePos.x + 7;
			end.y = ImGui::GetWindowPos().y + MOON_MousePos.y + 7;
			if (start.x != end.x && start.y != end.y)
				ConvertRegionToSelection(
					Vector2(
						start.x - ImGui::GetWindowPos().x - 7,
						start.y - ImGui::GetWindowPos().y - 7
					), MOON_MousePos
				);
		} else if (MOON_MouseRelease(0)) {
			start = end = ImVec2{ -1, -1 };
		}

		// draw select region (rectangle)
		if (start.x > 0 && end.x > 0) {
			draw_list->AddRectFilled(
				start, end, ImColor(255, 255, 255, 50)
			);
		}
	}

	void HotKeyManager::ConvertRegionToSelection(const Vector2& start, const Vector2& end) {
		Vector2 size(std::fabs(end.x - start.x), std::fabs(end.y - start.y));
		GLfloat* col = new GLfloat[size.x * size.y * 4];
		auto minX = std::min(start.x, end.x);
		auto minY = std::max(start.y, end.y);

		glBindFramebuffer(GL_READ_FRAMEBUFFER, MOON_TextureManager::IDLUT->fbo);
		glReadBuffer(GL_COLOR_ATTACHMENT0);
		glReadPixels(
			minX, MOON_ScrSize.y - minY - 1,
			size.x, size.y,
			GL_RGBA, GL_FLOAT, col
		);
		glReadBuffer(GL_NONE);
		glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);

		if (!MOON_InputManager::left_ctrl_hold) {
			if (state == EDIT) {
				Model* md = dynamic_cast<Model*>(MOON_EditTarget);
				for (int i = 0; i < md->meshList.size(); i++) {
					dynamic_cast<HalfMesh*>(md->meshList[i])->ClearSelection();
				}
			} else MOON_InputManager::ClearSelection();
		}

		for (int x = 0; x < size.x * size.y; x++) {
			auto x4 = x * 4;
			if (x > 0 && (col[x4] == col[x4 - 4] &&
				col[x4 + 1] == col[x4 - 3] &&
				col[x4 + 2] == col[x4 - 2] &&
				col[x4 + 3] == col[x4 - 1])) continue;

			if (state == EDIT) {
				Model* md = dynamic_cast<Model*>(MOON_EditTarget);
				auto id = Color::IDDecoder(
					col[x4], col[x4 + 1],
					col[x4 + 2], col[x4 + 3]
				) - 1;

				for (int i = 0, base = 0; i < md->meshList.size(); i++) {
					if (id >= base + md->meshList[i]->vertices.size()) {
						base += md->meshList[i]->vertices.size();
						continue;
					} else {
						dynamic_cast<HalfMesh*>(md->meshList[i])->Select_Append(
							MOON_EditElem, id - base, false);
						break;
					}
				}
			} else {
				MOON_InputManager::Select_Append(
					Color::IDDecoder(
						col[x4], col[x4 + 1],
						col[x4 + 2], col[x4 + 3]
					), false
				);
			}
		}
		delete[] col;
	}
}