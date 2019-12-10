#pragma once
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <shellapi.h>
#include <cstdlib>
#include <string.h>
#include <algorithm>
#include <string>
#include <shlobj.h>
#include <commDlg.h>

#include "Texture.h"
#include "Gizmo.h"
#include "CodeEditor.h"
#include "StackWindow.h"
#include "AssetLoader.h"
#include "MaterialEditor.h"
#include "IconsFontAwesome4.h"
#pragma warning(disable:4996)

namespace MOON {
	class MainUI {
	public:
		// image resources
		static Texture *icon, *logo, *logoFull;

		// ImGui IO
		static ImGuiIO* io;
		static ImGuiStyle* style;

		// bool for wnds
		static bool show_control_window;
		static bool show_demo_window;
		static bool show_VFB_window;
		static bool show_preference_window;
		static bool show_about_window;

		static bool show_explorer_window;
		static bool show_scene_window;
		static bool show_inspector_window;
		static bool show_project_window;
		static bool show_console_window;

		static bool show_create_window;
		static bool show_material_editor;
		static bool show_enviroment_editor;

		static bool show_codeEditor;
		static bool show_ribbon;
		static bool show_timeline;

		static bool show_curve_editor;
		static bool show_operator_editor;

		static bool show_profiler;

		// class-like wnds
		static MaterialEditor nodeEditor;

		static void CleanUp() {
			nodeEditor.CleanUp();
		}

		// style event
		static void SetButtonClicked() {
			style->Colors[ImGuiCol_Button] = ImVec4(1.000f, 1.000f, 1.000f, 0.250f);
		}

		static void ResetButtonColor() {
			style->Colors[ImGuiCol_Button] = ImVec4(1.000f, 1.000f, 1.000f, 0.000f);
		}

		static void SwitchButton(const char* ON_Label, const char* OFF_Label, const bool& switcher, void(*ON_Execute)(), void(*OFF_Execute)()) {
			if (switcher) {
				if (ImGui::Button(ON_Label, ImVec2(22, 22))) (*ON_Execute)();
			} else {
				SetButtonClicked();
				if (ImGui::Button(OFF_Label, ImVec2(22, 22))) (*OFF_Execute)();
				ResetButtonColor();
			}
		}
		static void RatioButton(const char* label, const bool& switcher, void(*executer)()) {
			if (switcher) SetButtonClicked();
			if (ImGui::Button(label, ImVec2(22, 22))) (*executer)();
			if (switcher) ResetButtonColor();
		}

		// Right click popup wnd
		static void QuadMenu() {
			if (!MOON_InputManager::isHoverUI && ImGui::IsMouseDown(1)) {
				ImGui::OpenPopup("QuadMenu");
			}

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

		// window definition
		static void MainMenu() {
			if (icon == NULL) icon = MOON_TextureManager::GetItem("moon_icon");

			ImGui::Image((void*)(intptr_t)icon->localID, ImVec2(20, 20));

			if (ImGui::BeginMenu("File")) {
				ShowFileMenu();
				ImGui::EndMenu();
			}
			if (ImGui::BeginMenu("Edit")) {
				if (ImGui::MenuItem("Undo", "CTRL+Z")) {}
				if (ImGui::MenuItem("Redo", "CTRL+Y", false, false)) {}  // Disabled item
				ImGui::Separator();
				if (ImGui::MenuItem("Cut", "CTRL+X")) {}
				if (ImGui::MenuItem("Copy", "CTRL+C")) {}
				if (ImGui::MenuItem("Paste", "CTRL+V")) {}
				ImGui::Separator();
				if (ImGui::MenuItem("Preferences", NULL, MainUI::show_preference_window)) {
					MainUI::show_preference_window = !MainUI::show_preference_window;
				}
				ImGui::EndMenu();
			}
			if (ImGui::BeginMenu("Animation")) {
				if (ImGui::BeginMenu("Constraints")) {
					if (ImGui::MenuItem("IK")) {}
					if (ImGui::MenuItem("Look At")) {}
					if (ImGui::MenuItem("Position")) {}
					if (ImGui::MenuItem("Rotation")) {}
					ImGui::EndMenu();
				}
				if (ImGui::MenuItem("Bone Tool")) {}
				if (ImGui::MenuItem("Facial")) {}
				if (ImGui::MenuItem("Retarget")) {}
				ImGui::EndMenu();
			}
			if (ImGui::BeginMenu("Simulation")) {
				if (ImGui::MenuItem("Fire")) {}
				if (ImGui::MenuItem("Water")) {}
				if (ImGui::MenuItem("Cloth")) {}
				if (ImGui::MenuItem("Fracture")) {}
				ImGui::Separator();
				if (ImGui::MenuItem("ParticleSys", "6")) {}
				ImGui::EndMenu();
			}
			if (ImGui::BeginMenu("Rabbit")) {
				if (ImGui::MenuItem("Enviroment", "8", MainUI::show_enviroment_editor)) {
					MainUI::show_enviroment_editor = !MainUI::show_enviroment_editor;
				}
				if (ImGui::MenuItem("Render", "F9")) {}
				if (ImGui::MenuItem("Settings", "F10")) {}
				ImGui::EndMenu();
			}
			if (ImGui::BeginMenu("Utility")) {
				if (ImGui::MenuItem("Baker", "0")) {}
				ImGui::EndMenu();
			}
			if (ImGui::BeginMenu("Window")) {
				if (ImGui::BeginMenu("Layout")) {
					if (ImGui::MenuItem("Default")) {}
					if (ImGui::MenuItem("Modeling")) {}
					if (ImGui::MenuItem("Animation")) {}
					if (ImGui::MenuItem("FX")) {}
					ImGui::EndMenu();
				}
				ImGui::Separator();
				if (ImGui::MenuItem("Control Panel", NULL, MainUI::show_control_window)) {
					MainUI::show_control_window = !MainUI::show_control_window;
				}
				if (ImGui::MenuItem("Explorer Window", NULL, MainUI::show_explorer_window)) {
					MainUI::show_explorer_window = !MainUI::show_explorer_window;
				}
				if (ImGui::MenuItem("Inspector Window", NULL, MainUI::show_inspector_window)) {
					MainUI::show_inspector_window = !MainUI::show_inspector_window;
				}
				if (ImGui::MenuItem("Scene Window", NULL, MainUI::show_scene_window)) {
					MainUI::show_scene_window = !MainUI::show_scene_window;
				}
				if (ImGui::MenuItem("Project Window", NULL, MainUI::show_project_window)) {
					MainUI::show_project_window = !MainUI::show_project_window;
				}
				if (ImGui::MenuItem("Material Editor", NULL, MainUI::show_material_editor)) {
					MainUI::show_material_editor = !MainUI::show_material_editor;
				}
				ImGui::Separator();
				if (ImGui::MenuItem("VFB Window", NULL, MainUI::show_VFB_window)) {
					MainUI::show_VFB_window = !MainUI::show_VFB_window;
				}
				if (ImGui::MenuItem("Code Editor", NULL, MainUI::show_codeEditor)) {
					MainUI::show_codeEditor = !MainUI::show_codeEditor;
					if (MainUI::show_codeEditor) MainUI::show_console_window = true;
				}
				if (ImGui::MenuItem("Console Window", NULL, MainUI::show_console_window)) {
					MainUI::show_console_window = !MainUI::show_console_window;
				}
				ImGui::Separator();
				if (ImGui::MenuItem("Ribbon", NULL, MainUI::show_ribbon)) {
					MainUI::show_ribbon = !MainUI::show_ribbon;
				}
				if (ImGui::MenuItem("Timeline", NULL, MainUI::show_timeline)) {
					MainUI::show_timeline = !MainUI::show_timeline;
				}
				ImGui::Separator();
				if (ImGui::MenuItem("Demo Window", NULL, MainUI::show_demo_window)) {
					MainUI::show_demo_window = !MainUI::show_demo_window;
				}
				ImGui::EndMenu();
			}
			if (ImGui::BeginMenu("Help")) {
				if (ImGui::MenuItem("Document", "F11")) {}
				if (ImGui::MenuItem("About", "F12")) {
					MainUI::show_about_window = !MainUI::show_about_window;
				}
				ImGui::EndMenu();
			}
			ImGui::EndMainMenuBar();
		}

		static void ControlPanel(const ImGuiIO *io, const Vector4 &clear_color) {
			static int width = MOON_OutputSize.x, height = MOON_OutputSize.y;

			ImGui::Begin(Icon_Name_To_ID(ICON_FA_COGS, " ControlPanel"));

			ImGui::Text("[Statistics]");
			ImGui::Text("FPS: %.1f (%.2f ms/frame)", ImGui::GetIO().Framerate, 1000.0f / ImGui::GetIO().Framerate);
			ImGui::Text("Hover UI: %d", io->WantCaptureMouse);

			ImGui::Separator();

			ImGui::Text("[Settings]");
			ImGui::Text("Debug:"); ImGui::SameLine();
			ImGui::Checkbox("BBox", &SceneManager::showbbox, true);

			ImGui::Spacing();

			ImGui::Text("BColor:"); ImGui::SameLine();
			ImGui::ColorEdit3("", (float*)&clear_color);

			ImGui::Text("Output Size:");
			ImGui::SetNextItemWidth(ImGui::GetWindowWidth() / 2.7);
			ImGui::InputInt("output_width", &width, 100, 1000, 0, true); ImGui::SameLine();
			ImGui::Text(u8"×"); ImGui::SameLine();
			ImGui::SetNextItemWidth(ImGui::GetWindowWidth() / 2.7);
			ImGui::InputInt("output_height", &height, 100, 1000, 0, true);
			if (MOON_OutputSize.x != width || MOON_OutputSize.y != height) {
				if (width < 1) width = 1;
				if (height < 1) height = 1;
				Renderer::SetOutputSize(width, height);
			}

			ImGui::Spacing();
			if (ImGui::Button("Rendering")) {
				if (!Renderer::prevInQueue) {
					if (Renderer::PrepareRendering()) {
						MainUI::show_VFB_window = true;

						pthread_t renderThread;
						int ret = pthread_create(&renderThread, NULL, Renderer::rendering, NULL);
						if (!ret) std::cout << "renderer thread created!" << std::endl;
						else std::cout << "renderer thread error! pthread_create error: error_code=" << ret << std::endl;
					}
				}
			}

			ImGui::End();
		}

		static void PreferencesWnd() {
			ImGui::Begin(Icon_Name_To_ID(ICON_FA_COG, " Style Editor"), &MainUI::show_preference_window);
			ImGui::ShowStyleEditor();
			ImGui::End();
		}

		static void ShowVFB() {
			ImGui::SetNextWindowSize(ImVec2(MOON_OutputSize.x < 100 ? 100 : MOON_OutputSize.x + 18,
				MOON_OutputSize.y < 100 ? 100 : MOON_OutputSize.y + 115));

			ImGui::Begin(Icon_Name_To_ID(ICON_FA_FILM, " VFB"), &MainUI::show_VFB_window, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoResize);

			ImGui::Button(ICON_FA_CLOCK_O, ImVec2(22, 22)); ImGui::SameLine();
			ImGui::Text(u8"|"); ImGui::SameLine();
			ImGui::Button(ICON_FA_OBJECT_GROUP, ImVec2(22, 22)); ImGui::SameLine();
			ImGui::Button(ICON_FA_ADJUST, ImVec2(22, 22));
			ImGui::Separator();
			
			ImGui::Text("size = %.0f x %.0f", MOON_OutputSize.x, MOON_OutputSize.y);

			ImVec2 pos = ImGui::GetCursorScreenPos();
			if (MOON_OutputTexID != -1)
				ImGui::Image((void*)(intptr_t)MOON_OutputTexID, ImVec2(MOON_OutputSize.x, MOON_OutputSize.y));

			if (ImGui::IsItemHovered() && ImGui::IsMouseDown(2)) {
				ImGui::BeginTooltip();
				float region_sz = 32.0f;
				float region_x = MOON_MousePos.x - pos.x - region_sz * 0.5f;
				float region_y = MOON_MousePos.y - pos.y - region_sz * 0.5f;

				if (region_x < 0.0f) region_x = 0.0f;
				else if (region_x > MOON_OutputSize.x - region_sz) region_x = MOON_OutputSize.x - region_sz;
				if (region_y < 0.0f) region_y = 0.0f;
				else if (region_y > MOON_OutputSize.y - region_sz) region_y = MOON_OutputSize.y - region_sz;

				float zoom = 4.0f;
				ImGui::Text("Min: (%.2f, %.2f)", region_x, region_y);
				ImGui::Text("Max: (%.2f, %.2f)", region_x + region_sz, region_y + region_sz);
				ImVec2 uv0 = ImVec2((region_x) / MOON_OutputSize.x, (region_y) / MOON_OutputSize.y);
				ImVec2 uv1 = ImVec2((region_x + region_sz) / MOON_OutputSize.x, (region_y + region_sz) / MOON_OutputSize.y);
				ImGui::Image((void*)(intptr_t)MOON_OutputTexID, ImVec2(region_sz * zoom, region_sz * zoom), uv0, uv1, ImVec4(1.0f, 1.0f, 1.0f, 1.0f), ImVec4(1.0f, 1.0f, 1.0f, 0.5f));
				ImGui::EndTooltip();
			}
			if (Renderer::progress > 0 && !Renderer::prevInQueue) {
				if (ImGui::Button("Abort")) {
					Renderer::isAbort = true;
				}
				ImGui::SameLine();
				ImGui::ProgressBar(Renderer::progress);
			}
			if (Renderer::end != -1) {
				float timeInSec = (float)(Renderer::end - Renderer::start) / CLOCKS_PER_SEC;
				int timeInMin = timeInSec / 60.0f;
				if (timeInMin > 0) {
					timeInSec -= timeInMin * 60.0f;
					ImGui::Text("Total time: %d m: %.1f s", timeInMin, timeInSec);
				} else {
					ImGui::Text("Total time: %.3f s", timeInSec);
				}
			}
			ImGui::End();
		}

		static void AboutWnd() {
			if (logo == NULL) logo = MOON_TextureManager::GetItem("moon_logo");

			ImGui::Begin(Icon_Name_To_ID(ICON_FA_USER, " ABOUT ME"), &MainUI::show_about_window, ImGuiWindowFlags_NoResize);
			ImGui::Image((void*)(intptr_t)logo->localID, ImVec2(logo->width / 3, logo->height / 3));

			ImGui::Text(u8"【HU ANIME】");
			ImGui::Text(u8">独立动画/游戏创作者");
			ImGui::Text(">Indie Game/Animation Creator");

			ImGui::Separator();
			ImGui::Text(u8"【CONTACT】");
			ImGui::Text(u8">联系 / Contact me：");
			if (ImGui::Button("Email")) ShellExecute(NULL, "open", "mailto:hztmailbox@gmail.com", NULL, NULL, SW_SHOW);
			ImGui::SameLine(); ImGui::Text("hztmailbox@gmail.com");

			ImGui::Separator();
			ImGui::Text(u8"【LINKS】");
			ImGui::Text(u8">动画作品 / My animation works：");
			if (ImGui::Button("Bilibili Homepage")) ShellExecute(NULL, "open", "https://space.bilibili.com/7868260", NULL, NULL, SW_SHOW);
			ImGui::Text(u8">其他CG作品 / My other CG works：");
			if (ImGui::Button("Pixiv")) ShellExecute(NULL, "open", "https://www.pixiv.net/member.php?id=22929868", NULL, NULL, SW_SHOW); ImGui::SameLine();
			if (ImGui::Button("ArtStation")) ShellExecute(NULL, "open", "https://www.artstation.com/huanime", NULL, NULL, SW_SHOW);
			ImGui::Text(u8">My Unity publisher page：");
			if (ImGui::Button("Publisher Page")) ShellExecute(NULL, "open", "https://assetstore.unity.com/publishers/34649", NULL, NULL, SW_SHOW);
			ImGui::Text(u8">投食 / Support me：");
			if (ImGui::Button("Aifadian")) ShellExecute(NULL, "open", "https://afdian.net/@MrMaoMao", NULL, NULL, SW_SHOW);

			ImGui::End();
		}

		static void ExplorerWnd() {
			ImGui::Begin(Icon_Name_To_ID(ICON_FA_SHOPPING_BASKET, " Explorer"), 
				&MainUI::show_explorer_window,ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);

			static bool showAll = true;
			static bool showModel = true;
			static bool showMat = true;
			static bool showTex = true;
			static bool showLight = true;
			static bool showCam = true;
			static bool showShader = true;

			static bool allFlag = true;
			static char pattern[128] = "";
			bool notEmpty = strcmp(pattern, "");

			ImGui::Checkbox("All", &showAll); ImGui::SameLine(80);
			ImGui::SetNextItemWidth(ImGui::GetWindowWidth() / 1.65 - (notEmpty ? 20 : 0));
			ImGui::InputTextWithHint("search", "type to search", pattern, IM_ARRAYSIZE(pattern));
			if (ImGui::IsItemDeactivated() && ImGui::IsKeyPressed(ImGui::GetKeyIndex(ImGuiKey_Enter))) {

			}
			if (notEmpty) {
				ImGui::SameLine();
				if (ImGui::Button(ICON_FA_TIMES, ImVec2(20, 20))) {
					sprintf_s(pattern, "");
				}
			}
			ImGui::Checkbox("Model", &showModel); ImGui::SameLine(80);
			ImGui::Checkbox("Mat", &showMat); ImGui::SameLine(160);
			ImGui::Checkbox("Tex", &showTex);
			ImGui::Checkbox("Light", &showLight); ImGui::SameLine(80);
			ImGui::Checkbox("Camera", &showCam); ImGui::SameLine(160);
			ImGui::Checkbox("Shader", &showShader);

			if (allFlag != showAll) {
				allFlag = showAll;
				showModel = showAll; showMat = showAll; showTex = showAll;
				showLight = showAll; showCam = showAll; showShader = showAll;
			}

			static float colWidth = 180;

			ImGui::Spacing();
			ImGui::Columns(2, "mycolumns", false);
			ImGui::Separator();
			ImGui::SetColumnWidth(-1, colWidth);
			ImGui::Text("Name"); ImGui::NextColumn();
			ImGui::Text("ID");
			ImGui::Separator();
			ImGui::Columns(1);

			ImGui::BeginChild("explorer_scrolling", ImVec2(0, 0), 
							  false, ImGuiWindowFlags_HorizontalScrollbar);
			ImGui::Columns(2, "mycolumns", false);
			ImGui::SetColumnWidth(-1, colWidth - 8);

			MOON_InputManager::ResetSizeChangeState();
			if (showModel) MOON_ModelManager::ListItems();
			if (showMat) MOON_MaterialManager::ListItems();
			if (showTex) MOON_TextureManager::ListItems();
			if (showLight) MOON_LightManager::ListItems();
			if (showCam) MOON_CameraManager::ListItems();
			if (showShader) MOON_ShaderManager::ListItems();
			ImGui::NextColumn();
			if (showModel) MOON_ModelManager::ListID();
			if (showMat) MOON_MaterialManager::ListID();
			if (showTex) MOON_TextureManager::ListID();
			if (showLight) MOON_LightManager::ListID();
			if (showCam) MOON_CameraManager::ListID();
			if (showShader) MOON_ShaderManager::ListID();
			ImGui::EndChild();

			ImGui::Columns(1);
			ImGui::Spacing();
			
			ImGui::End();
		}

		static void SceneWnd() {
			ImGui::Begin(Icon_Name_To_ID(ICON_FA_GAMEPAD, " Scene"),
						  &MainUI::show_scene_window, ImGuiWindowFlags_NoBackground);

			ImGui::End();
		}
		
		static void ConsoleWnd() {
			static MOON::ConsoleWnd console;
			console.Draw(Icon_Name_To_ID(ICON_FA_TERMINAL, " Console"), &MainUI::show_console_window);
		}

		static void ProjectWnd() {
			ImGui::Begin(Icon_Name_To_ID(ICON_FA_FOLDER_OPEN, " Project"), &MainUI::show_project_window);

			// tree view
			ImGui::BeginChild("TreeView", ImVec2(150, 0), true);
			static DirNode* selNode = NULL;
			if (AssetLoader::DirTree != NULL) AssetLoader::DirTree->ListNode(selNode);
			ImGui::EndChild();
			ImGui::SameLine();

			// folder view
			ImGui::BeginChild("FolderView", ImVec2(0, 0), true);
			float window_visible_x2 = ImGui::GetWindowPos().x + ImGui::GetWindowContentRegionMax().x;
			if (selNode != NULL) selNode->RenderFolderView(window_visible_x2, selNode);
			ImGui::EndChild();

			ImGui::End();
		}

		static void InspectorWnd() {
			ImGui::Begin(Icon_Name_To_ID(ICON_FA_SEARCH, " Inspector"), &MainUI::show_inspector_window);
			// loop all selected objects and list their properties
			for (auto &iter : MOON_InputManager::selection) {
				bool checker = MOON_ObjectList[iter]->selected;

				if (ImGui::CollapsingHeader((SceneManager::GetTypeIcon(MOON_ObjectList[iter]) + "  " + 
											 MOON_ObjectList[iter]->name).c_str(), &MOON_ObjectList[iter]->selected, 
											 ImGuiTreeNodeFlags_DefaultOpen, MOON_ObjectList[iter]->ID)) {
					MOON_ObjectList[iter]->ListProperties();
				}

				// remove ID in selection slot while click close button in the collapsing header
				if (checker && !MOON_ObjectList[iter]->selected) {
					auto end = MOON_InputManager::selection.end();
					for (auto it = MOON_InputManager::selection.begin(); it != end; it++)
						if (*it == MOON_ObjectList[iter]->ID) {
							it = MOON_InputManager::selection.erase(it);
							break;
						}
				}
			}

			ImGui::End();
		}

		static void ShowTimeline() {
			ImGui::Begin("TimeLine", &MainUI::show_timeline);

			ImGui::End();
		}

		static void MaterialEditorWnd() {
			ImGui::Begin(Icon_Name_To_ID(ICON_FA_FUTBOL_O, " MaterialEditor"), &MainUI::show_material_editor);

			ImGui::Columns(2, "mycolumns");
			static bool firstLoop = true;
			if (firstLoop) ImGui::SetColumnWidth(-1, 160);
			float floatWidth = ImGui::GetColumnWidth(-1) - 20;

			// left
			static Material* selectedMat = NULL;
			static unsigned int prevID = MOON_TextureManager::GetItem("moon_logo_full")->localID;

			ImGui::BeginChild("Previewer", ImVec2(floatWidth, 140), true, ImGuiWindowFlags_NoScrollbar);
			ImGui::Image((void*)(intptr_t)prevID, ImVec2(125, 125));
			ImGui::EndChild();

			// materials in the scene ------------------------------------------------------
			ImGui::BeginChild("Mat Explorer", ImVec2(floatWidth, 0), true);
			int loopID = 0;
			for (auto &mat : MOON_MaterialManager::itemMap) {
				if (ImGui::Selectable(mat.second->name.c_str(), selectedMat == mat.second))
					selectedMat = mat.second;
			}
			ImGui::EndChild();

			ImGui::NextColumn();

			// right
			ImGui::BeginChild("item view", ImVec2(0, 0));
			ImGui::Button("Save"); ImGui::SameLine();
			ImGui::Button("Load"); ImGui::SameLine();
			if (ImGui::Button("Clear")) {
				RegistStackWnd("Information");
			}
			StackWnd("Information", "Clear node editor?", &nodeEditor, &MaterialEditor::ClearEditor);
			ImGui::SameLine();
			ImGui::Text(u8"|"); ImGui::SameLine();
			ImGui::Text("Material: %d", MOON_MaterialManager::CountItem());
			ImGui::Separator();
			ImGui::Spacing();

			ImGui::Columns(2, "matView");
			if (firstLoop) {
				firstLoop = false;
				ImGui::SetColumnWidth(-1, 500);
			}
			floatWidth = ImGui::GetColumnWidth(-1) - 15;
			// node editor -------------------------------------------------------------
			ImGui::BeginChild("nodeEditor", ImVec2(floatWidth, 0), true,
				ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
			{
				nodeEditor.Draw();
			}
			ImGui::EndChild();

			ImGui::NextColumn();
			// properties view ---------------------------------------------------------
			ImGui::BeginChild("propView");
			{
				if (nodeEditor.anythingSelected) {
					selectedMat = NULL; // TODO

					nodeEditor.selectedNode->ProcessContent(false);
				} else {
					if (selectedMat == NULL) ImGui::Text("Nothing selected");
					else selectedMat->ListProperties();
				}
			}
			ImGui::EndChild();

			ImGui::Columns(1);
			ImGui::EndChild();
			ImGui::End();
		}

		static void EnviromentWnd() {
			ImGui::Begin(Icon_Name_To_ID(ICON_FA_CLOUD, " Enviroment"), &MainUI::show_enviroment_editor);
			
			ImGui::End();
		}

		static void CreateWnd() {
			ImGui::Begin(Icon_Name_To_ID(ICON_FA_CUBES, " Creator"), &MainUI::show_create_window);

			ImGuiTabBarFlags tab_bar_flags = ImGuiTabBarFlags_None | 
											 ImGuiTabBarFlags_FittingPolicyScroll | 
											 ImGuiTabBarFlags_TabListPopupButton;
			if (ImGui::BeginTabBar("CreatorTabBar", tab_bar_flags)) {
				float width = ImGui::GetWindowWidth() / 3.5;
				if (ImGui::BeginTabItem("Basic")) {
					ImGui::Spacing();
					if (ImGui::Button("Cube", ImVec2(width, 20.0))) {}
					ImGui::SameLine();
					if (ImGui::Button("Sphere", ImVec2(width, 20.0))) {}
					ImGui::SameLine();
					if (ImGui::Button("Cylinder", ImVec2(width, 20.0))) {}

					if (ImGui::Button("Plane", ImVec2(width, 20.0))) {}
					ImGui::SameLine();
					if (ImGui::Button("Capsule", ImVec2(width, 20.0))) {}
					ImGui::SameLine();
					if (ImGui::Button("Text", ImVec2(width, 20.0))) {}

					ImGui::EndTabItem();
				}
				if (ImGui::BeginTabItem("Shape")) {
					ImGui::Spacing();
					if (ImGui::Button("Line", ImVec2(width, 20.0))) {}
					ImGui::SameLine();
					if (ImGui::Button("Rectangle", ImVec2(width, 20.0))) {}
					ImGui::SameLine();
					if (ImGui::Button("Circle", ImVec2(width, 20.0))) {}

					ImGui::EndTabItem();
				}
				if (ImGui::BeginTabItem("Particle")) {
					ImGui::Spacing();
					if (ImGui::Button("ParticleSys")) {}

					ImGui::EndTabItem();
				}
				if (ImGui::BeginTabItem("Light")) {
					ImGui::Spacing();
					if (ImGui::Button("Direct", ImVec2(width, 20.0))) {}
					ImGui::SameLine();
					if (ImGui::Button("Spot", ImVec2(width, 20.0))) {}
					ImGui::SameLine();
					if (ImGui::Button("Point", ImVec2(width, 20.0))) {}

					if (ImGui::Button("MoLight", ImVec2(width, 20.0))) {}
					ImGui::SameLine();
					if (ImGui::Button("Dome", ImVec2(width, 20.0))) {}

					ImGui::EndTabItem();
				}
				if (ImGui::BeginTabItem("Camera")) {
					ImGui::Spacing();
					if (ImGui::Button("Target", ImVec2(width, 20.0))) {}
					ImGui::SameLine();
					if (ImGui::Button("Free", ImVec2(width, 20.0))) {}

					ImGui::EndTabItem();
				}
				if (ImGui::BeginTabItem("Helper")) {
					ImGui::Spacing();
					if (ImGui::Button("Dummy", ImVec2(width, 20.0))) {}
					ImGui::SameLine();
					if (ImGui::Button("Bone", ImVec2(width, 20.0))) {}

					ImGui::EndTabItem();
				}
			}
			ImGui::EndTabBar();

			ImGui::End();
		}

		static void CodeEditor() {
			static MOON::CodeEditor CEditor;
			CEditor.Draw(&MainUI::show_codeEditor);
		}

		static void RibbonBar() {
			ImGui::Begin("Ribbon", &MainUI::show_ribbon, ImGuiWindowFlags_NoDecoration | 
						 ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollWithMouse );

			ImGui::Button(ICON_FA_FILE, ImVec2(22, 22)); ImGui::SameLine();
			ImGui::Button(ICON_FA_FLOPPY_O, ImVec2(22, 22)); ImGui::SameLine();
			ImGui::Button(ICON_FA_HISTORY, ImVec2(22, 22)); ImGui::SameLine();

			ImGui::Text(u8"|"); ImGui::SameLine();

			RatioButton(ICON_FA_LINK, Gizmo::gizmoMode == GizmoMode::link,
				[]() { Gizmo::gizmoMode = GizmoMode::link; }); ImGui::SameLine();
			ImGui::Button(ICON_FA_CHAIN_BROKEN, ImVec2(22, 22)); ImGui::SameLine();

			ImGui::Text(u8"|"); ImGui::SameLine();

			SwitchButton(ICON_FA_TOGGLE_OFF, ICON_FA_TOGGLE_ON, Gizmo::manipCoord == CoordSys::WORLD,
						[]() { Gizmo::manipCoord = CoordSys::LOCAL; },
						[]() { Gizmo::manipCoord = CoordSys::WORLD; });
			ImGui::SameLine();

			ImGui::Text(u8"|"); ImGui::SameLine();

			RatioButton(ICON_FA_MOUSE_POINTER, Gizmo::gizmoMode == GizmoMode::none,
				[]() { Gizmo::gizmoMode = GizmoMode::none; }); ImGui::SameLine();
			RatioButton(ICON_FA_ARROWS, Gizmo::gizmoMode == GizmoMode::translate,
				[]() { Gizmo::gizmoMode = GizmoMode::translate; }); ImGui::SameLine();
			RatioButton(ICON_FA_REFRESH, Gizmo::gizmoMode == GizmoMode::rotate,
				[]() { Gizmo::gizmoMode = GizmoMode::rotate; }); ImGui::SameLine();
			RatioButton(ICON_FA_EXPAND, Gizmo::gizmoMode == GizmoMode::scale,
				[]() { Gizmo::gizmoMode = GizmoMode::scale; }); ImGui::SameLine();

			//ImGui::Text(u8"|"); ImGui::SameLine();
			//ImGui::Button(ICON_FA_DELICIOUS, ImVec2(22, 22)); ImGui::SameLine();

			ImGui::Text(u8"|"); ImGui::SameLine();

			ImGui::Button(ICON_FA_BOLT, ImVec2(22, 22)); ImGui::SameLine();
			ImGui::Button(ICON_FA_FIRE, ImVec2(22, 22)); ImGui::SameLine();
			ImGui::Button(ICON_FA_TINT, ImVec2(22, 22)); ImGui::SameLine();
			ImGui::Button(ICON_FA_FLAG, ImVec2(22, 22)); ImGui::SameLine();
			ImGui::Button(ICON_FA_GLASS, ImVec2(22, 22)); ImGui::SameLine();

			ImGui::Text(u8"|"); ImGui::SameLine();

			ImGui::Button(ICON_FA_TH, ImVec2(22, 22)); ImGui::SameLine();
			ImGui::Button(ICON_FA_RANDOM, ImVec2(22, 22)); ImGui::SameLine();

			ImGui::Text(u8"|"); ImGui::SameLine();

			ImGui::Button(ICON_FA_PAINT_BRUSH, ImVec2(22, 22)); ImGui::SameLine();
			ImGui::Button(ICON_FA_AREA_CHART, ImVec2(22, 22)); ImGui::SameLine();

			ImGui::Text(u8"|"); ImGui::SameLine();

			ImGui::Button(ICON_FA_LEAF, ImVec2(22, 22)); ImGui::SameLine();
			ImGui::Button(ICON_FA_ROAD, ImVec2(22, 22)); ImGui::SameLine();
			ImGui::Button(ICON_FA_UNIVERSAL_ACCESS, ImVec2(22, 22)); ImGui::SameLine();
			ImGui::Button(ICON_FA_STREET_VIEW, ImVec2(22, 22)); ImGui::SameLine();

			ImGui::Text(u8"|"); ImGui::SameLine();

			ImGui::Button(ICON_FA_CAMERA, ImVec2(22, 22)); ImGui::SameLine();
			ImGui::Button(ICON_FA_LIST_ALT, ImVec2(22, 22)); ImGui::SameLine();

			ImGui::Text(u8"|"); ImGui::SameLine();

			ImGui::Button(ICON_FA_QUESTION_CIRCLE, ImVec2(22, 22));

			ImGui::Separator();

			ImGui::End();
		}

		static void CurveEditor() {
		
		}

		static void OperatorExplorer() {
			
		}

	private:
		static std::string OpenFile() {
			TCHAR szBuffer[MAX_PATH] = { 0 };
			BROWSEINFO bi;
			ZeroMemory(&bi, sizeof(BROWSEINFO));
			bi.hwndOwner = NULL;
			bi.pszDisplayName = szBuffer;
			bi.lpszTitle = "Select model file:";
			bi.ulFlags = BIF_BROWSEINCLUDEFILES;
			LPITEMIDLIST idl = SHBrowseForFolder(&bi);
			if (NULL == idl) return NULL;
			SHGetPathFromIDList(idl, szBuffer);
			return szBuffer;
		}
		
		static std::string OpenFolder() {
			TCHAR szBuffer[MAX_PATH] = { 0 };
			BROWSEINFO bi;
			ZeroMemory(&bi, sizeof(BROWSEINFO));
			bi.hwndOwner = NULL;
			bi.pszDisplayName = szBuffer;
			bi.lpszTitle = "Select export folder:";
			bi.ulFlags = BIF_RETURNFSANCESTORS;
			LPITEMIDLIST idl = SHBrowseForFolder(&bi);
			if (NULL == idl) return NULL;
			SHGetPathFromIDList(idl, szBuffer);
			return szBuffer;
		}

		static void ShowFileMenu() {
			//ImGui::MenuItem("(dummy menu)", NULL, false, false);
			if (ImGui::MenuItem("New")) {}
			if (ImGui::MenuItem("Open", "Ctrl+O")) {}
			if (ImGui::BeginMenu("Open Recent")) {
				ImGui::MenuItem("test_1.moon");
				ImGui::MenuItem("test_2.moon");
				if (ImGui::BeginMenu("More...")) {
					ImGui::MenuItem("test_3.moon");
					ImGui::MenuItem("test_4.moon");
					ImGui::EndMenu();
				}
				ImGui::EndMenu();
			}
			if (ImGui::MenuItem("Save", "Ctrl+S")) {}
			if (ImGui::MenuItem("Save As...")) {}

			ImGui::Separator();
			if (ImGui::BeginMenu("Import")) {
				if (ImGui::MenuItem("Model...")) {
					std::cout << "Selected file: " << OpenFile() << std::endl;
				}
				if (ImGui::MenuItem("Scene...")) {
					std::cout << "Selected file: " << OpenFile() << std::endl;
				}
				ImGui::EndMenu();
			}

			if (ImGui::BeginMenu("Export")) {
				if (ImGui::MenuItem("Export All...")) {
					std::cout << "Selected folder: " << OpenFolder() << std::endl;
				}
				if (ImGui::MenuItem("Export Selected...")) {
					std::cout << "Selected folder: " << OpenFolder() << std::endl;
				}
				ImGui::EndMenu();
			}

			ImGui::Separator();
			if (ImGui::MenuItem("Quit", "Alt+F4")) {
				SceneManager::exitFlag = true;
			}

			/*if (ImGui::BeginMenu("Options")) {
				static bool enabled = true;
				ImGui::MenuItem("Enabled", "", &enabled);
				ImGui::BeginChild("child", ImVec2(0, 60), true);
				for (int i = 0; i < 10; i++)
					ImGui::Text("Scrolling Text %d", i);
				ImGui::EndChild();
				static float f = 0.5f;
				static int n = 0;
				static bool b = true;
				ImGui::SliderFloat("Value", &f, 0.0f, 1.0f);
				ImGui::InputFloat("Input", &f, 0.1f);
				ImGui::Combo("Combo", &n, "Yes\0No\0Maybe\0\0");
				ImGui::Checkbox("Check", &b);
				ImGui::EndMenu();
			}
			if (ImGui::BeginMenu("Colors")) {
				float sz = ImGui::GetTextLineHeight();
				for (int i = 0; i < ImGuiCol_COUNT; i++) {
					const char* name = ImGui::GetStyleColorName((ImGuiCol)i);
					ImVec2 p = ImGui::GetCursorScreenPos();
					ImGui::GetWindowDrawList()->AddRectFilled(p, ImVec2(p.x + sz, p.y + sz), ImGui::GetColorU32((ImGuiCol)i));
					ImGui::Dummy(ImVec2(sz, sz));
					ImGui::SameLine();
					ImGui::MenuItem(name);
				}
				ImGui::EndMenu();
			}
			if (ImGui::BeginMenu("Disabled", false)) { IM_ASSERT(0); }
			if (ImGui::MenuItem("Checked", NULL, true)) {}*/
		}
	};

}