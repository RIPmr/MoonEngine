#pragma once

#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <shellapi.h>

bool PrepareRendering();

class MainUI {
public:
	// image resources
	static GLuint iconID, logoID;
	static int iconWidth, iconHeight, logoWidth, logoHeight;

	// bool for window
	static bool show_control_window;
	static bool show_demo_window;
	static bool show_VFB_window;
	static bool show_preference_window;
	static bool show_about_window;

	// load resources
	static bool LoadImages() {
		bool flag = true;
		flag &= LoadTextureFromFile("./Resources/Icon.jpg", &iconID, &iconWidth, &iconHeight);
		flag &= LoadTextureFromFile("./Resources/logo.png", &logoID, &logoWidth, &logoHeight);
		return flag;
	}

	// window definition
	static void MainMenu() {
		ImGui::Image((void*)(intptr_t)iconID, ImVec2(20, 20));

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
			if (ImGui::MenuItem("VFB Window", NULL, MainUI::show_VFB_window)) {
				MainUI::show_VFB_window = !MainUI::show_VFB_window;
			}
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

	static void ControlPanel(const ImGuiIO &io, const ImVec4 &clear_color) {
		static int width = Renderer::OUTPUT_SIZE.x, height = Renderer::OUTPUT_SIZE.y;

		ImGui::Begin("ControlPanel");

		ImGui::Text("[Statistics]");
		ImGui::Text("FPS: %.1f (%.2f ms/frame)", ImGui::GetIO().Framerate, 1000.0f / ImGui::GetIO().Framerate);
		ImGui::Text("Hover UI: %d", io.WantCaptureMouse);

		ImGui::Separator();

		ImGui::Text("[Settings]");
		ImGui::Text("BColor:");ImGui::SameLine();
		ImGui::ColorEdit3("", (float*)&clear_color);

		ImGui::Text("Output Size:");
		ImGui::SetNextItemWidth(ImGui::GetWindowWidth() / 2.7);
		ImGui::InputInt("output_width", &width, 100, 1000, 0, true);ImGui::SameLine();
		ImGui::Text(u8"×");ImGui::SameLine();
		ImGui::SetNextItemWidth(ImGui::GetWindowWidth() / 2.7);
		ImGui::InputInt("output_height", &height, 100, 1000, 0, true);
		if (width < 1) width = 1; if (height < 1) height = 1;
		Renderer::OUTPUT_SIZE.x = width; Renderer::OUTPUT_SIZE.y = height;

		if (ImGui::Button("Rendering")) {
			if (PrepareRendering()) {
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

	static void ShowVFB(const ImGuiIO &io) {
		ImGui::SetNextWindowSize(ImVec2(Renderer::OUTPUT_SIZE.x < 100 ? 100 : Renderer::OUTPUT_SIZE.x + 18, 
										Renderer::OUTPUT_SIZE.y < 100 ? 100 : Renderer::OUTPUT_SIZE.y + 80));

		ImGui::Begin("VFB", &MainUI::show_VFB_window, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoResize);
		ImGui::Text("size = %.0f x %.0f", Renderer::OUTPUT_SIZE.x, Renderer::OUTPUT_SIZE.y);

		ImVec2 pos = ImGui::GetCursorScreenPos();
		if (Renderer::outputTexID != -1)
			ImGui::Image((void*)(intptr_t)Renderer::outputTexID, ImVec2(Renderer::OUTPUT_SIZE.x, Renderer::OUTPUT_SIZE.y));

		if (ImGui::IsItemHovered() && ImGui::IsMouseDown(2)) {
			ImGui::BeginTooltip();
			float region_sz = 32.0f;
			float region_x = io.MousePos.x - pos.x - region_sz * 0.5f;
			float region_y = io.MousePos.y - pos.y - region_sz * 0.5f;

			if (region_x < 0.0f) region_x = 0.0f;
			else if (region_x > Renderer::OUTPUT_SIZE.x - region_sz) region_x = Renderer::OUTPUT_SIZE.x - region_sz;
			if (region_y < 0.0f) region_y = 0.0f;
			else if (region_y > Renderer::OUTPUT_SIZE.y - region_sz) region_y = Renderer::OUTPUT_SIZE.y - region_sz;

			float zoom = 4.0f;
			ImGui::Text("Min: (%.2f, %.2f)", region_x, region_y);
			ImGui::Text("Max: (%.2f, %.2f)", region_x + region_sz, region_y + region_sz);
			ImVec2 uv0 = ImVec2((region_x) / Renderer::OUTPUT_SIZE.x, (region_y) / Renderer::OUTPUT_SIZE.y);
			ImVec2 uv1 = ImVec2((region_x + region_sz) / Renderer::OUTPUT_SIZE.x, (region_y + region_sz) / Renderer::OUTPUT_SIZE.y);
			ImGui::Image((void*)(intptr_t)Renderer::outputTexID, ImVec2(region_sz * zoom, region_sz * zoom), uv0, uv1, ImVec4(1.0f, 1.0f, 1.0f, 1.0f), ImVec4(1.0f, 1.0f, 1.0f, 0.5f));
			ImGui::EndTooltip();
		}
		if (Renderer::progress > 0) {
			if (ImGui::Button("Abort")) {

			}
			ImGui::SameLine();
			ImGui::ProgressBar(Renderer::progress);
		}
		if (Renderer::end != -1)
			ImGui::Text("Total time: %.4f s", (double)(Renderer::end - Renderer::start) / CLOCKS_PER_SEC);
		ImGui::End();
	}

	static void AboutWnd() {
		ImGui::Begin("ABOUT ME", &MainUI::show_about_window, ImGuiWindowFlags_NoResize);
		ImGui::Image((void*)(intptr_t)logoID, ImVec2(logoWidth / 3, logoHeight / 3));


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