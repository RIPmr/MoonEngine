#pragma once
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <shellapi.h>
#include <cstdlib>
#include <string.h>
#include <algorithm>
#include <string>

#include "Texture.h"
#include "IconsFontAwesome4.h"
#pragma warning(disable:4996)

namespace moon {
	
	struct ConsoleWnd {
		char                  InputBuf[256];
		ImVector<char*>       Items;
		ImVector<const char*> Commands;
		ImVector<char*>       History;
		int                   HistoryPos;    // -1: new line, 0..History.Size-1 browsing history.
		ImGuiTextFilter       Filter;
		bool                  AutoScroll;
		bool                  ScrollToBottom;

		ConsoleWnd() {
			ClearLog();
			memset(InputBuf, 0, sizeof(InputBuf));
			HistoryPos = -1;
			Commands.push_back("HELP");
			Commands.push_back("HISTORY");
			Commands.push_back("CLEAR");
			Commands.push_back("CLASSIFY");  // "classify" is only here to provide an example of "C"+[tab] completing to "CL" and displaying matches.
			AutoScroll = true;
			ScrollToBottom = false;
			AddLog("Welcome to the MOON!");
		}
		~ConsoleWnd() {
			ClearLog();
			for (int i = 0; i < History.Size; i++)
				free(History[i]);
		}

		// Portable helpers
		static int   Stricmp(const char* str1, const char* str2) { int d; while ((d = toupper(*str2) - toupper(*str1)) == 0 && *str1) { str1++; str2++; } return d; }
		static int   Strnicmp(const char* str1, const char* str2, int n) { int d = 0; while (n > 0 && (d = toupper(*str2) - toupper(*str1)) == 0 && *str1) { str1++; str2++; n--; } return d; }
		static char* Strdup(const char *str) { size_t len = strlen(str) + 1; void* buf = malloc(len); IM_ASSERT(buf); return (char*)memcpy(buf, (const void*)str, len); }
		static void  Strtrim(char* str) { char* str_end = str + strlen(str); while (str_end > str && str_end[-1] == ' ') str_end--; *str_end = 0; }

		void ClearLog() {
			for (int i = 0; i < Items.Size; i++)
				free(Items[i]);
			Items.clear();
		}

		void AddLog(const char* fmt, ...) IM_FMTARGS(2) {
			// FIXME-OPT
			char buf[1024];
			va_list args;
			va_start(args, fmt);
			vsnprintf(buf, IM_ARRAYSIZE(buf), fmt, args);
			buf[IM_ARRAYSIZE(buf) - 1] = 0;
			va_end(args);
			Items.push_back(Strdup(buf));
		}

		void Draw(const char* title, bool* p_open) {
			ImGui::SetNextWindowSize(ImVec2(520, 600), ImGuiCond_FirstUseEver);
			if (!ImGui::Begin(title, p_open)) {
				ImGui::End();
				return;
			}

			// As a specific feature guaranteed by the library, after calling Begin() the last Item represent the title bar. So e.g. IsItemHovered() will return true when hovering the title bar.
			// Here we create a context menu only available from the title bar.
			if (ImGui::BeginPopupContextItem()) {
				if (ImGui::MenuItem("Close Console")) *p_open = false;
				ImGui::EndPopup();
			}

			//ImGui::TextWrapped("Enter 'HELP' for help, press TAB to use text completion.");

			// TODO: display items starting from the bottom

			//if (ImGui::SmallButton("Test Text")) { AddLog("%d some text", Items.Size); AddLog("some more text"); AddLog("display message here!"); } ImGui::SameLine();
			//if (ImGui::SmallButton("Test Error")) { AddLog("[error] something went wrong"); } ImGui::SameLine();
			if (ImGui::SmallButton("Clear")) { ClearLog(); } ImGui::SameLine();
			bool copy_to_clipboard = ImGui::SmallButton("Copy"); ImGui::SameLine();
			//static float t = 0.0f; if (ImGui::GetTime() - t > 0.02f) { t = ImGui::GetTime(); AddLog("Spam %f", t); }
			if (ImGui::SmallButton("Options")) ImGui::OpenPopup("Options");

			ImGui::Separator();

			// Options menu
			if (ImGui::BeginPopup("Options")) {
				ImGui::Checkbox("Auto-scroll", &AutoScroll);
				ImGui::EndPopup();
			}
			
			// Filter
			//ImGui::SameLine();
			//Filter.Draw("Filter (\"incl,-excl\") (\"error\")", 180);
			//ImGui::Separator();

			const float footer_height_to_reserve = ImGui::GetStyle().ItemSpacing.y + ImGui::GetFrameHeightWithSpacing(); // 1 separator, 1 input text
			ImGui::BeginChild("ScrollingRegion", ImVec2(0, -footer_height_to_reserve), false, ImGuiWindowFlags_HorizontalScrollbar); // Leave room for 1 separator + 1 InputText
			if (ImGui::BeginPopupContextWindow()) {
				if (ImGui::Selectable("Clear")) ClearLog();
				ImGui::EndPopup();
			}

			// Display every line as a separate entry so we can change their color or add custom widgets. If you only want raw text you can use ImGui::TextUnformatted(log.begin(), log.end());
			// NB- if you have thousands of entries this approach may be too inefficient and may require user-side clipping to only process visible items.
			// You can seek and display only the lines that are visible using the ImGuiListClipper helper, if your elements are evenly spaced and you have cheap random access to the elements.
			// To use the clipper we could replace the 'for (int i = 0; i < Items.Size; i++)' loop with:
			//     ImGuiListClipper clipper(Items.Size);
			//     while (clipper.Step())
			//         for (int i = clipper.DisplayStart; i < clipper.DisplayEnd; i++)
			// However, note that you can not use this code as is if a filter is active because it breaks the 'cheap random-access' property. We would need random-access on the post-filtered list.
			// A typical application wanting coarse clipping and filtering may want to pre-compute an array of indices that passed the filtering test, recomputing this array when user changes the filter,
			// and appending newly elements as they are inserted. This is left as a task to the user until we can manage to improve this example code!
			// If your items are of variable size you may want to implement code similar to what ImGuiListClipper does. Or split your data into fixed height items to allow random-seeking into your list.
			ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(4, 1)); // Tighten spacing
			if (copy_to_clipboard)
				ImGui::LogToClipboard();
			for (int i = 0; i < Items.Size; i++) {
				const char* item = Items[i];
				if (!Filter.PassFilter(item))
					continue;

				// Normally you would store more information in your item (e.g. make Items[] an array of structure, store color/type etc.)
				bool pop_color = false;
				if (strstr(item, "[error]")) { ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.4f, 0.4f, 1.0f)); pop_color = true; } 
				else if (strncmp(item, "# ", 2) == 0) { ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.8f, 0.6f, 1.0f)); pop_color = true; }
				ImGui::TextUnformatted(item);
				if (pop_color) ImGui::PopStyleColor();
			}
			if (copy_to_clipboard) ImGui::LogFinish();

			if (ScrollToBottom || (AutoScroll && ImGui::GetScrollY() >= ImGui::GetScrollMaxY()))
				ImGui::SetScrollHereY(1.0f);
			ScrollToBottom = false;

			ImGui::PopStyleVar();
			ImGui::EndChild();
			ImGui::Separator();

			// Command-line
			bool reclaim_focus = false;
			if (ImGui::InputText("Input", InputBuf, IM_ARRAYSIZE(InputBuf), 
								  ImGuiInputTextFlags_EnterReturnsTrue | 
								  ImGuiInputTextFlags_CallbackCompletion |
								  ImGuiInputTextFlags_CallbackHistory, 
								 &TextEditCallbackStub, (void*)this)) {
				char* s = InputBuf;
				Strtrim(s);
				if (s[0]) ExecCommand(s);
				strcpy(s, "");
				reclaim_focus = true;
			}

			// Auto-focus on window apparition
			ImGui::SetItemDefaultFocus();
			if (reclaim_focus)
				ImGui::SetKeyboardFocusHere(-1); // Auto focus previous widget

			ImGui::End();
		}

		void ExecCommand(const char* command_line) {
			AddLog("# %s\n", command_line);

			// Insert into history. First find match and delete it so it can be pushed to the back. This isn't trying to be smart or optimal.
			HistoryPos = -1;
			for (int i = History.Size - 1; i >= 0; i--)
				if (Stricmp(History[i], command_line) == 0) {
					free(History[i]);
					History.erase(History.begin() + i);
					break;
				}
			History.push_back(Strdup(command_line));

			// Process command
			if (Stricmp(command_line, "CLEAR") == 0) {
				ClearLog();
			} else if (Stricmp(command_line, "HELP") == 0) {
				AddLog("Commands:");
				for (int i = 0; i < Commands.Size; i++)
					AddLog("- %s", Commands[i]);
			} else if (Stricmp(command_line, "HISTORY") == 0) {
				int first = History.Size - 10;
				for (int i = first > 0 ? first : 0; i < History.Size; i++)
					AddLog("%3d: %s\n", i, History[i]);
			} else {
				AddLog("Unknown command: '%s'\n", command_line);
			}

			// On commad input, we scroll to bottom even if AutoScroll==false
			ScrollToBottom = true;
		}

		// In C++11 you are better off using lambdas for this sort of forwarding callbacks
		static int TextEditCallbackStub(ImGuiInputTextCallbackData* data) {
			ConsoleWnd* console = (ConsoleWnd*)data->UserData;
			return console->TextEditCallback(data);
		}

		int TextEditCallback(ImGuiInputTextCallbackData* data) {
			//AddLog("cursor: %d, selection: %d-%d", data->CursorPos, data->SelectionStart, data->SelectionEnd);
			switch (data->EventFlag) {
			case ImGuiInputTextFlags_CallbackCompletion:
			{
				// Example of TEXT COMPLETION
				// Locate beginning of current word
				const char* word_end = data->Buf + data->CursorPos;
				const char* word_start = word_end;
				while (word_start > data->Buf) {
					const char c = word_start[-1];
					if (c == ' ' || c == '\t' || c == ',' || c == ';')
						break;
					word_start--;
				}

				// Build a list of candidates
				ImVector<const char*> candidates;
				for (int i = 0; i < Commands.Size; i++)
					if (Strnicmp(Commands[i], word_start, (int)(word_end - word_start)) == 0)
						candidates.push_back(Commands[i]);

				if (candidates.Size == 0) {
					// No match
					AddLog("No match for \"%.*s\"!\n", (int)(word_end - word_start), word_start);
				} else if (candidates.Size == 1) {
					// Single match. Delete the beginning of the word and replace it entirely so we've got nice casing
					data->DeleteChars((int)(word_start - data->Buf), (int)(word_end - word_start));
					data->InsertChars(data->CursorPos, candidates[0]);
					data->InsertChars(data->CursorPos, " ");
				} else {
					// Multiple matches. Complete as much as we can, so inputing "C" will complete to "CL" and display "CLEAR" and "CLASSIFY"
					int match_len = (int)(word_end - word_start);
					for (;;) {
						int c = 0;
						bool all_candidates_matches = true;
						for (int i = 0; i < candidates.Size && all_candidates_matches; i++)
							if (i == 0)
								c = toupper(candidates[i][match_len]);
							else if (c == 0 || c != toupper(candidates[i][match_len]))
								all_candidates_matches = false;
						if (!all_candidates_matches)
							break;
						match_len++;
					}

					if (match_len > 0) {
						data->DeleteChars((int)(word_start - data->Buf), (int)(word_end - word_start));
						data->InsertChars(data->CursorPos, candidates[0], candidates[0] + match_len);
					}

					// List matches
					AddLog("Possible matches:\n");
					for (int i = 0; i < candidates.Size; i++)
						AddLog("- %s\n", candidates[i]);
				}

				break;
			}
			case ImGuiInputTextFlags_CallbackHistory:
			{
				// Example of HISTORY
				const int prev_history_pos = HistoryPos;
				if (data->EventKey == ImGuiKey_UpArrow) {
					if (HistoryPos == -1)
						HistoryPos = History.Size - 1;
					else if (HistoryPos > 0)
						HistoryPos--;
				} else if (data->EventKey == ImGuiKey_DownArrow) {
					if (HistoryPos != -1)
						if (++HistoryPos >= History.Size)
							HistoryPos = -1;
				}

				// A better implementation would preserve the data on the current input line along with cursor position.
				if (prev_history_pos != HistoryPos) {
					const char* history_str = (HistoryPos >= 0) ? History[HistoryPos] : "";
					data->DeleteChars(0, data->BufTextLen);
					data->InsertChars(0, history_str);
				}
			}
			}
			return 0;
		}
	};

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

		static bool show_codeEditor;
		static bool show_ribbon;
		static bool show_timeline;

		static bool show_curve_editor;
		static bool show_operator_editor;

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
				if (ImGui::MenuItem("Enviroment", "8")) {}
				if (ImGui::MenuItem("Render", "F9")) {}
				if (ImGui::MenuItem("Settings", "F10")) {}
				ImGui::EndMenu();
			}
			if (ImGui::BeginMenu("Utility")) {
				if (ImGui::MenuItem("Baker", "0")) {}
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

			ImGui::Begin((std::string(ICON_FA_COGS) + " ControlPanel").c_str());

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
				if (Renderer::PrepareRendering()) {
					MainUI::show_VFB_window = true;

					pthread_t renderThread;
					int ret = pthread_create(&renderThread, NULL, Renderer::rendering, NULL);
					if (!ret) std::cout << "renderer thread created!" << std::endl;
					else std::cout << "renderer thread error! pthread_create error: error_code=" << ret << std::endl;
				}
			}

			/*ImGui::OpenPopup("RenderingStacked");
			bool dummy_open = true;
			if (ImGui::BeginPopupModal("RenderingStacked", &dummy_open)) {
				ImGui::Text("Hello from Stacked The Second!");
				if (ImGui::Button("Close"))
					ImGui::CloseCurrentPopup();
				ImGui::EndPopup();
			}*/

			ImGui::End();
		}

		static void PreferencesWnd() {
			ImGui::Begin((std::string(ICON_FA_COG) + " Style Editor").c_str(), &MainUI::show_preference_window);
			ImGui::ShowStyleEditor();
			ImGui::End();
		}

		static void ShowVFB() {
			ImGui::SetNextWindowSize(ImVec2(MOON_OutputSize.x < 100 ? 100 : MOON_OutputSize.x + 18,
				MOON_OutputSize.y < 100 ? 100 : MOON_OutputSize.y + 115));

			ImGui::Begin((std::string(ICON_FA_FILM) + " VFB").c_str(), &MainUI::show_VFB_window, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoResize);

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
			if (Renderer::progress > 0) {
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

			ImGui::Begin((std::string(ICON_FA_USER) + " ABOUT ME").c_str(), &MainUI::show_about_window, ImGuiWindowFlags_NoResize);
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
			ImGui::Begin((std::string(ICON_FA_SHOPPING_BASKET) + " Explorer").c_str(), &MainUI::show_explorer_window);

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

			ImGui::BeginChild("explorer_scrolling", 
							  ImVec2(0, ImGui::GetFrameHeightWithSpacing() * 10 + 30), 
							  false, ImGuiWindowFlags_HorizontalScrollbar);
			ImGui::Columns(2, "mycolumns", false);
			ImGui::SetColumnWidth(-1, colWidth - 8);
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
			ImGui::EndChild();

			ImGui::Columns(1);
			ImGui::Spacing();
			
			ImGui::End();
		}

		static void SceneWnd() {
			ImGui::Begin((std::string(ICON_FA_GAMEPAD) + " Scene").c_str(), 
						  &MainUI::show_scene_window, 
						  ImGuiWindowFlags_NoBackground);

			ImGui::End();
		}
		
		static void ConsoleWnd() {
			// class-like window
			static moon::ConsoleWnd console;
			console.Draw((std::string(ICON_FA_TERMINAL) + "Console").c_str(), &MainUI::show_console_window);
		}

		static void ProjectWnd() {
			ImGui::Begin((std::string(ICON_FA_FOLDER_OPEN) + "Project").c_str(), &MainUI::show_project_window);

			ImGui::End();
		}

		static void InspectorWnd() {
			ImGui::Begin((std::string(ICON_FA_SEARCH) + " Inspector").c_str(), &MainUI::show_inspector_window);
			// loop all selected objects and list their properties
			for (auto &iter : MOON_InputManager::selected) {
				bool checker = MOON_InputManager::selection[MOON_ObjectList[iter]->ID];

				if (ImGui::CollapsingHeader((SceneManager::GetTypeIcon(MOON_ObjectList[iter]) + 
											 "  " + MOON_ObjectList[iter]->name).c_str(),
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
			ImGui::Begin((std::string(ICON_FA_FUTBOL_O) + " MaterialEditor").c_str(), &MainUI::show_material_editor);

			ImGui::Columns(2, "mycolumns");
			ImGui::SetColumnWidth(-1, 160);
			// left
			static int selected = 0;
			static unsigned int prevID = MOON_TextureManager::GetItem("moon_logo_full")->localID;
			ImGui::BeginChild("Previewer", ImVec2(150, 140), true);
			ImGui::Image((void*)(intptr_t)prevID, ImVec2(125, 125));
			ImGui::EndChild();
			//ImGui::SameLine();

			ImGui::BeginChild("Mat Explorer", ImVec2(150, 0), true);
			for (int i = 0; i < 100; i++) {
				char label[128];
				sprintf(label, "Material %d", i);
				if (ImGui::Selectable(label, selected == i))
					selected = i;
			}
			ImGui::EndChild();

			ImGui::NextColumn();

			// right
			ImGui::BeginGroup();
			ImGui::BeginChild("item view", ImVec2(0, -ImGui::GetFrameHeightWithSpacing())); // Leave room for 1 line below us
			ImGui::Text("Material: %d", selected);
			ImGui::Separator();
			if (ImGui::BeginTabBar("##Tabs", ImGuiTabBarFlags_None)) {
				if (ImGui::BeginTabItem("Information")) {
					ImGui::TextWrapped("Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. ");
					ImGui::EndTabItem();
				}
				if (ImGui::BeginTabItem("Node Editor")) {
					ImGui::Text("ID: 0123456789");
					ImGui::EndTabItem();
				}
				ImGui::EndTabBar();
			}
			ImGui::EndChild();
			if (ImGui::Button("Revert")) {}
			ImGui::SameLine();
			if (ImGui::Button("Save")) {}
			ImGui::EndGroup();

			ImGui::Columns(1);

			ImGui::End();
		}

		static void EnviromentWnd() {
			ImGui::Begin((std::string(ICON_FA_CLOUD) + " Enviroment").c_str(), &MainUI::show_enviroment_editor);

			ImGui::End();
		}

		static void CreateWnd() {
			ImGui::Begin((std::string(ICON_FA_CUBES) + " Creator").c_str(), &MainUI::show_create_window);

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
			ImGui::Begin((std::string(ICON_FA_CODE) + " Code Editor").c_str(), &MainUI::show_codeEditor);
			static char text[1024 * 16] =
				"/*\n"
				" The Pentium F00F bug, shorthand for F0 0F C7 C8,\n"
				" the hexadecimal encoding of one offending instruction,\n"
				" more formally, the invalid operand with locked CMPXCHG8B\n"
				" instruction bug, is a design flaw in the majority of\n"
				" Intel Pentium, Pentium MMX, and Pentium OverDrive\n"
				" processors (all in the P5 microarchitecture).\n"
				"*/\n\n"
				"label:\n"
				"\tlock cmpxchg8b eax\n";

			ImGui::InputTextMultiline("##source", text, IM_ARRAYSIZE(text), ImVec2(-FLT_MIN, ImGui::GetTextLineHeight() * 16));
			ImGui::End();
		}

		static void RibbonBar() {
			//ImGui::PushStyleVar(ImGuiStyleVar_IndentSpacing, 0.1f);

			ImGui::Begin("Ribbon", &MainUI::show_ribbon, 
						 ImGuiWindowFlags_NoDecoration | 
						 ImGuiWindowFlags_NoMove |
						 ImGuiWindowFlags_NoScrollWithMouse
						);

			ImGui::Button(ICON_FA_FILE, ImVec2(22, 22)); ImGui::SameLine();
			ImGui::Button(ICON_FA_FLOPPY_O, ImVec2(22, 22)); ImGui::SameLine();
			ImGui::Button(ICON_FA_HISTORY, ImVec2(22, 22)); ImGui::SameLine();

			ImGui::Text(u8"|"); ImGui::SameLine();

			ImGui::Button(ICON_FA_LINK, ImVec2(22, 22)); ImGui::SameLine();
			ImGui::Button(ICON_FA_CHAIN_BROKEN, ImVec2(22, 22)); ImGui::SameLine();

			ImGui::Text(u8"|"); ImGui::SameLine();

			ImGui::Button(ICON_FA_MOUSE_POINTER, ImVec2(22, 22)); ImGui::SameLine();
			ImGui::Button(ICON_FA_ARROWS, ImVec2(22, 22)); ImGui::SameLine();
			ImGui::Button(ICON_FA_REFRESH, ImVec2(22, 22)); ImGui::SameLine();
			ImGui::Button(ICON_FA_EXPAND, ImVec2(22, 22)); ImGui::SameLine();

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

			//ImGui::PopStyleVar();
			ImGui::End();
		}

		static void CurveEditor() {
		
		}

		static void OperatorEditor() {
			
		}

	private:
		static void ShowExampleAppLayout(bool* p_open) {
			ImGui::SetNextWindowSize(ImVec2(500, 440), ImGuiCond_FirstUseEver);
			if (ImGui::Begin("Example: Simple layout", p_open, ImGuiWindowFlags_MenuBar)) {
				if (ImGui::BeginMenuBar()) {
					if (ImGui::BeginMenu("File")) {
						if (ImGui::MenuItem("Close")) *p_open = false;
						ImGui::EndMenu();
					}
					ImGui::EndMenuBar();
				}

				// left
				static int selected = 0;
				ImGui::BeginChild("left pane", ImVec2(150, 0), true);
				for (int i = 0; i < 100; i++) {
					char label[128];
					sprintf(label, "MyObject %d", i);
					if (ImGui::Selectable(label, selected == i))
						selected = i;
				}
				ImGui::EndChild();
				ImGui::SameLine();

				// right
				ImGui::BeginGroup();
				ImGui::BeginChild("item view", ImVec2(0, -ImGui::GetFrameHeightWithSpacing())); // Leave room for 1 line below us
				ImGui::Text("MyObject: %d", selected);
				ImGui::Separator();
				if (ImGui::BeginTabBar("##Tabs", ImGuiTabBarFlags_None)) {
					if (ImGui::BeginTabItem("Description")) {
						ImGui::TextWrapped("Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. ");
						ImGui::EndTabItem();
					}
					if (ImGui::BeginTabItem("Details")) {
						ImGui::Text("ID: 0123456789");
						ImGui::EndTabItem();
					}
					ImGui::EndTabBar();
				}
				ImGui::EndChild();
				if (ImGui::Button("Revert")) {}
				ImGui::SameLine();
				if (ImGui::Button("Save")) {}
				ImGui::EndGroup();
			}
			ImGui::End();
		}

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