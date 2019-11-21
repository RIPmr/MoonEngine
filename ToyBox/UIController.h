#pragma once
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <shellapi.h>
#include <cstdlib>

#include "Texture.h"

namespace moon {
	class MainUI {
	public:
		// image resources
		static Texture *icon, *logo;

		// ImGui IO
		static ImGuiIO* io;

		// bool for window
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

		static bool show_ribbon;
		static bool show_timeline;

		// window definition
		static void MainMenu() {
			if (icon == NULL) icon = MOON_TextureManager::GetItem("moon_icon");

			ImGui::Image((void*)(intptr_t)icon->localID, ImVec2(20, 20));

			if (ImGui::BeginMenu("File")) {
				ShowExampleMenuFile();
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
			if (ImGui::BeginMenu("Window")) {
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
				ImGui::Separator();
				if (ImGui::MenuItem("VFB Window", NULL, MainUI::show_VFB_window)) {
					MainUI::show_VFB_window = !MainUI::show_VFB_window;
				}
				if (ImGui::MenuItem("Console Window", NULL, MainUI::show_console_window)) {
					MainUI::show_console_window = !MainUI::show_console_window;
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

		static void ControlPanel(const ImGuiIO *io, const ImVec4 &clear_color) {
			static int width = MOON_OutputSize.x, height = MOON_OutputSize.y;

			ImGui::Begin("ControlPanel");

			ImGui::Text("[Statistics]");
			ImGui::Text("FPS: %.1f (%.2f ms/frame)", ImGui::GetIO().Framerate, 1000.0f / ImGui::GetIO().Framerate);
			ImGui::Text("Hover UI: %d", io->WantCaptureMouse);

			ImGui::Separator();

			ImGui::Text("[Settings]");
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
				if (Renderer::PrepareRendering()) {
					MainUI::show_VFB_window = true;

					pthread_t renderThread;
					int ret = pthread_create(&renderThread, NULL, Renderer::rendering, NULL);
					if (!ret) std::cout << "renderer thread created!" << std::endl;
					else std::cout << "renderer thread error! pthread_create error: error_code=" << ret << std::endl;
				}
			}

			ImGui::End();
		}

		static void PreferencesWnd() {
			ImGui::Begin("Style Editor", &MainUI::show_preference_window);
			ImGui::ShowStyleEditor();
			ImGui::End();
		}

		static void ShowVFB() {
			ImGui::SetNextWindowSize(ImVec2(MOON_OutputSize.x < 100 ? 100 : MOON_OutputSize.x + 18,
				MOON_OutputSize.y < 100 ? 100 : MOON_OutputSize.y + 80));

			ImGui::Begin("VFB", &MainUI::show_VFB_window, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoResize);
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
			if (Renderer::progress > 0) {
				if (ImGui::Button("Abort")) {
					Renderer::isAbort = true;
				}
				ImGui::SameLine();
				ImGui::ProgressBar(Renderer::progress);
			}
			if (Renderer::end != -1)
				ImGui::Text("Total time: %.4f s", (double)(Renderer::end - Renderer::start) / CLOCKS_PER_SEC);
			ImGui::End();
		}

		static void AboutWnd() {
			if (logo == NULL) logo = MOON_TextureManager::GetItem("moon_logo");

			ImGui::Begin("ABOUT ME", &MainUI::show_about_window, ImGuiWindowFlags_NoResize);
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
			ImGui::Begin("Explorer", &MainUI::show_explorer_window);

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
				if (ImGui::Button(u8"×", ImVec2(20, 20))) {
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

			ImGui::Columns(2, "mycolumns");
			ImGui::Separator();
			ImGui::Text("Name"); ImGui::NextColumn();
			ImGui::Text("ID"); ImGui::NextColumn();
			ImGui::Separator();

			MOON_InputManager::UpdateSelectionState();
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

			ImGui::Columns(1);
			ImGui::Spacing();
			
			ImGui::End();
		}

		static void SceneWnd() {
			ImGui::Begin("Scene", &MainUI::show_scene_window);

			ImGui::End();
		}
		
		static void ConsoleWnd() {
			ImGui::Begin("Console", &MainUI::show_console_window);

			ImGui::End();
		}

		static void ProjectWnd() {
			ImGui::Begin("Project", &MainUI::show_project_window);

			ImGui::End();
		}

		static void InspectorWnd() {
			ImGui::Begin("Inspector", &MainUI::show_inspector_window);
			// loop all selected objects and list their properties
			for (auto &iter : MOON_InputManager::selected) {
				bool checker = MOON_InputManager::selection[MOON_ObjectList[iter]->ID];

				if (ImGui::CollapsingHeader(MOON_ObjectList[iter]->name.c_str(), 
											&MOON_InputManager::selection[MOON_ObjectList[iter]->ID], 
											ImGuiTreeNodeFlags_DefaultOpen)) {
					MOON_ObjectList[iter]->ListProperties();
				}

				// remove ID in selection slot while click close button in the collapsing header
				if (checker && !MOON_InputManager::selection[MOON_ObjectList[iter]->ID]) {
					auto end = MOON_InputManager::selected.end();
					for (auto it = MOON_InputManager::selected.begin(); it != end; it++)
						if (*it == MOON_ObjectList[iter]->ID) {
							it = MOON_InputManager::selected.erase(it);
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

		static void MaterialEditor() {
			ImGui::Begin("MaterialEditor", &MainUI::show_material_editor);

			ImGui::End();
		}

		static void EnviromentWnd() {
			ImGui::Begin("Enviroment", &MainUI::show_enviroment_editor);

			ImGui::End();
		}

		static void CreateWnd() {
			ImGui::Begin("Creator", &MainUI::show_create_window);

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
					if (ImGui::Button("ParticleSystem")) {}

					ImGui::EndTabItem();
				}
				if (ImGui::BeginTabItem("Light")) {
					ImGui::Spacing();
					if (ImGui::Button("DirectLight", ImVec2(width, 20.0))) {}
					ImGui::SameLine();
					if (ImGui::Button("SpotLight", ImVec2(width, 20.0))) {}
					ImGui::SameLine();
					if (ImGui::Button("PointLight", ImVec2(width, 20.0))) {}

					if (ImGui::Button("MoonLight", ImVec2(width, 20.0))) {}
					ImGui::SameLine();
					if (ImGui::Button("DomeLight", ImVec2(width, 20.0))) {}

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

	private:
		static void ShowExampleMenuFile() {
			ImGui::MenuItem("(dummy menu)", NULL, false, false);
			if (ImGui::MenuItem("New")) {}
			if (ImGui::MenuItem("Open", "Ctrl+O")) {}
			if (ImGui::BeginMenu("Open Recent")) {
				ImGui::MenuItem("fish_hat.c");
				ImGui::MenuItem("fish_hat.inl");
				ImGui::MenuItem("fish_hat.h");
				if (ImGui::BeginMenu("More..")) {
					ImGui::MenuItem("Hello");
					ImGui::MenuItem("Sailor");
					if (ImGui::BeginMenu("Recurse..")) {
						ShowExampleMenuFile();
						ImGui::EndMenu();
					}
					ImGui::EndMenu();
				}
				ImGui::EndMenu();
			}
			if (ImGui::MenuItem("Save", "Ctrl+S")) {}
			if (ImGui::MenuItem("Save As..")) {}
			ImGui::Separator();
			if (ImGui::BeginMenu("Options")) {
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
			if (ImGui::BeginMenu("Disabled", false)) {
				IM_ASSERT(0);
			}
			if (ImGui::MenuItem("Checked", NULL, true)) {}
			if (ImGui::MenuItem("Quit", "Alt+F4")) {}
		}
	};
}