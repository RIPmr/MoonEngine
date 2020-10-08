#pragma once
#include <imgui.h>
#include <cstdlib>
#include <cstring>
#include <string>
#include <algorithm>
#include <fstream>

#include "Icons.h"
#include "StackWindow.h"
#include "ImTextEditor.h"
#pragma warning(disable:4996)

namespace MOON {

#pragma region console_window
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
			// A typical application wanting coarse clipping and filtering may want to pre-compute an array of triangles that passed the filtering test, recomputing this array when user changes the filter,
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
				if (strstr(item, "[error]")) { ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.4f, 0.4f, 1.0f)); pop_color = true; } else if (strncmp(item, "# ", 2) == 0) { ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 0.8f, 0.6f, 1.0f)); pop_color = true; }
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
#pragma endregion

#pragma region code_editor_impl
	struct CodeEditor {
	public:
		TextEditor editor;
		std::string fileToEdit;

		CodeEditor() {
			ConfigureEditor_GLSL();
		}

		void ConfigureEditor_GLSL() {
			auto lang = TextEditor::LanguageDefinition::GLSL();

			// set your own known preprocessor symbols and their corresponding values
			static const char* ppnames[] = { 
				"NULL", "assert", "attribute", "uniform", "varying", "in", "out",
			};
			static const char* ppvalues[] = {
				"#define NULL ((void*)0)",
				" #define assert(expression) (void)(                                                  \n"
				"    (!!(expression)) ||                                                              \n"
				"    (_wassert(_CRT_WIDE(#expression), _CRT_WIDE(__FILE__), (unsigned)(__LINE__)), 0) \n"
				" )"
			};
			for (int i = 0; i < sizeof(ppnames) / sizeof(ppnames[0]); ++i) {
				TextEditor::TextIdentifier id;
				id.mDeclaration = i < 2 ? ppvalues[i] : "";
				lang.mPreprocIdentifiers.insert(std::make_pair(std::string(ppnames[i]), id));
			}

			// set your own identifiers
			static const char* identifiers[] = {
				"HWND", "TextEditor" 
			};
			static const char* idecls[] = {
				"typedef HWND_* HWND", "class TextEditor" 
			};
			for (int i = 0; i < sizeof(identifiers) / sizeof(identifiers[0]); ++i) {
				TextEditor::TextIdentifier id;
				id.mDeclaration = std::string(idecls[i]);
				lang.mIdentifiers.insert(std::make_pair(std::string(identifiers[i]), id));
			}

			//editor.SetPalette(TextEditor::GetLightPalette());
			editor.SetLanguageDefinition(lang);

			// error markers
			/*TextEditor::ErrorMarkers markers;
			markers.insert(std::make_pair<int, std::string>(6, "Example error here:\nInclude file not found: \"TextEditor.h\""));
			markers.insert(std::make_pair<int, std::string>(41, "Another example error"));
			editor.SetErrorMarkers(markers);*/

			// "breakpoint" markers
			/*TextEditor::Breakpoints bpts;
			bpts.insert(24);
			bpts.insert(47);
			editor.SetBreakpoints(bpts);*/
		}

		void LoadFile(const std::string& path) {
			fileToEdit = path;
			std::ifstream tmp(fileToEdit);
			if (tmp.good()) {
				std::string str((std::istreambuf_iterator<char>(tmp)), std::istreambuf_iterator<char>());
				editor.SetText(str);
			}
		}

		void Draw(bool* isOpen) {
			auto cpos = editor.GetCursorPosition();
			ImGui::Begin(Icon_Name_To_ID(ICON_FA_CODE, " Code Editor"), isOpen, ImGuiWindowFlags_HorizontalScrollbar | ImGuiWindowFlags_MenuBar);
			ImGui::SetWindowSize(ImVec2(800, 600), ImGuiCond_FirstUseEver);
			if (ImGui::BeginMenuBar()) {
				if (ImGui::BeginMenu("File")) {
					if (ImGui::MenuItem("New", "Ctrl+N")) {
						fileToEdit.clear();
						editor.SetText("");
					}
					ImGui::Separator();
					if (ImGui::MenuItem("Open", "Ctrl+O")) {
						// TODO: open file ...

					}
					if (ImGui::MenuItem("Save", "Ctrl+S")) {
						auto textToSave = editor.GetText();
						// TODO: save text ...

					}
					if (ImGui::MenuItem("Save As...", "Ctrl+Shift+N")) {
						auto textToSave = editor.GetText();
						// TODO: save text ...

					}
					ImGui::Separator();
					if (ImGui::MenuItem("Quit", "Alt+F4")) isOpen = false;
					ImGui::EndMenu();
				}
				if (ImGui::BeginMenu("Edit")) {
					bool ro = editor.IsReadOnly();
					if (ImGui::MenuItem("Read-only mode", nullptr, &ro))
						editor.SetReadOnly(ro);
					ImGui::Separator();

					if (ImGui::MenuItem("Undo", "Alt+Backspace", nullptr, !ro && editor.CanUndo()))
						editor.Undo();
					if (ImGui::MenuItem("Redo", "Ctrl+Y", nullptr, !ro && editor.CanRedo()))
						editor.Redo();

					ImGui::Separator();

					if (ImGui::MenuItem("Copy", "Ctrl+C", nullptr, editor.HasSelection()))
						editor.Copy();
					if (ImGui::MenuItem("Cut", "Ctrl+X", nullptr, !ro && editor.HasSelection()))
						editor.Cut();
					if (ImGui::MenuItem("Delete", "Del", nullptr, !ro && editor.HasSelection()))
						editor.Delete();
					if (ImGui::MenuItem("Paste", "Ctrl+V", nullptr, !ro && ImGui::GetClipboardText() != nullptr))
						editor.Paste();

					ImGui::Separator();

					if (ImGui::MenuItem("Select all", nullptr, nullptr))
						editor.SetSelection(TextEditor::Coordinates(), TextEditor::Coordinates(editor.GetTotalLines(), 0));

					ImGui::EndMenu();
				}

				if (ImGui::BeginMenu("View")) {
					if (ImGui::MenuItem("Dark palette"))
						editor.SetPalette(TextEditor::GetDarkPalette());
					if (ImGui::MenuItem("Light palette"))
						editor.SetPalette(TextEditor::GetLightPalette());
					if (ImGui::MenuItem("Retro blue palette"))
						editor.SetPalette(TextEditor::GetRetroBluePalette());
					ImGui::EndMenu();
				}
				ImGui::EndMenuBar();
			}

			ImGui::Text(
				"%6d/%-6d %6d lines  | %s | %s | %s%s", 
				cpos.mLine + 1, cpos.mColumn + 1, 
				editor.GetTotalLines(),
				editor.IsOverwrite() ? "Ovr" : "Ins",
				editor.GetLanguageDefinition().mName.c_str(),
				editor.CanUndo() ? "*" : " ",
				fileToEdit.empty() ? "NewDocument" : fileToEdit.c_str()
			);

			editor.Render("CodeEditor");
			ImGui::End();
		}
	};
#pragma endregion

}