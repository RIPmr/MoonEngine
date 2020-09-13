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
}