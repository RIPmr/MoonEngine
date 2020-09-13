#pragma once
#define IMGUI_DEFINE_MATH_OPERATORS
#include <imgui.h>
#include <imgui_internal.h>
#include <functional>

namespace ImGui {

	// original source of timeline code is
	// https://github.com/nem0/LumixEngine
	// modified further according to imgui issue 76 and adding a panzoomer
	// https://github.com/ocornut/imgui/issues/76
	class ImTimeline {
	private:
		static float s_max_timeline_value;
		static float s_pixel_offset;

		static double s_time_in;
		static double s_time_out;

		static double s_time_offset;
		static double s_time_scale;

	public:
		static const float TIMELINE_RADIUS;

		static bool BeginTimeline(const char* str_id, float pixel_offset, float max_value, int num_visible_rows) {
			s_time_scale = 1.0 / (s_time_out - s_time_in);
			s_time_offset = s_time_in * s_time_scale;

			if (num_visible_rows <= 0) num_visible_rows = 5;

			ImGuiWindow * win = GetCurrentWindow();
			float height = win->ContentRegionRect.Max.y 
				- win->ContentRegionRect.Min.y
				- ImGui::GetTextLineHeightWithSpacing()   // space for the time bar
				- ImGui::GetTextLineHeightWithSpacing();  // space for horizontal scroller

			bool rv = BeginChild(str_id, ImVec2(0, height), false);

			//ImGui::PushStyleColor(ImGuiCol_Column, GImGui->Style.Colors[ImGuiCol_Border]);
			ImGui::Columns(2, str_id);

			static float _pixel_offset = 0;
			if (pixel_offset != _pixel_offset) {
				_pixel_offset = pixel_offset;
				ImGui::SetColumnOffset(1, pixel_offset);
			}
			s_max_timeline_value = max_value >= 0 ? max_value : (ImGui::GetWindowContentRegionWidth() * 0.85f);
			return rv;
		}

		static bool TimelineEvent(const char* str_id, double &val1, double &val2, const double range01) {
			double values[2] = { val1, val2 };
			ImGuiWindow* win = GetCurrentWindow();
			const ImU32 inactive_color = ColorConvertFloat4ToU32(GImGui->Style.Colors[ImGuiCol_SliderGrab]);
			const ImU32 active_color = ColorConvertFloat4ToU32(GImGui->Style.Colors[ImGuiCol_SliderGrabActive]);
			const ImU32 line_color = ColorConvertFloat4ToU32(GImGui->Style.Colors[ImGuiCol_SliderGrabActive]);
			bool changed = false, hovered = false;

			ImGui::Text("%s", str_id);
			ImGui::NextColumn();

			const float columnOffset = ImGui::GetColumnOffset(1);
			const float columnWidth = ImGui::GetColumnWidth(1) - GImGui->Style.ScrollbarSize;
			ImVec2 cursor_pos(GetWindowContentRegionMin().x + win->Pos.x + columnOffset - TIMELINE_RADIUS, win->DC.CursorPos.y);
			float posx[2] = { 0, 0 }; ImVec2 p[2]; bool pact[2];
			for (int i = 0; i < 2; ++i) {
				ImVec2 pos = cursor_pos;
				pos.x += s_time_scale * columnWidth * float(values[i]) / s_max_timeline_value - columnWidth * s_time_offset + TIMELINE_RADIUS;
				pos.y += TIMELINE_RADIUS;
				posx[i] = pos.x;

				SetCursorScreenPos(ImVec2(pos.x - TIMELINE_RADIUS, pos.y - TIMELINE_RADIUS));
				PushID(i);
				InvisibleButton(str_id, ImVec2(2 * TIMELINE_RADIUS, 2 * TIMELINE_RADIUS));
				if (IsItemHovered()) {
					ImGui::SetTooltip("%f", float(values[i]));
					ImVec2 a(pos.x, GetWindowContentRegionMin().y + win->Pos.y + win->Scroll.y);

					ImGuiWindow * parent_win = win->ParentWindow;
					float endy = parent_win->ContentRegionRect.Max.y + win->Pos.y; // draw all the way to the bottom of the parent window

					ImVec2 b(pos.x, endy);
					win->DrawList->AddLine(a, b, line_color);
					hovered = true;
				}
				if (IsItemActive() && IsMouseDragging(0)) {
					//values[i] += GetIO().MouseDelta.x / win->Size.x * s_max_timeline_value;
					values[i] += GetIO().MouseDelta.x / (ImGui::GetColumnWidth() - GImGui->Style.ScrollbarSize) * s_max_timeline_value * range01;
					if (values[i] < 0) values[i] = 0;
					else if (values[i] > s_max_timeline_value) values[i] = s_max_timeline_value;
					changed = hovered = true;
				}
				PopID();
				p[i] = pos; pact[i] = IsItemActive() || IsItemHovered();
			}

			ImVec2 start = cursor_pos;
			start.x = posx[0] + TIMELINE_RADIUS * 0.9f;
			start.y += TIMELINE_RADIUS * 0.5f;
			ImVec2 end = start;
			end.x = posx[1] - TIMELINE_RADIUS * 0.9f;
			end.y += TIMELINE_RADIUS;

			PushID(-1);
			SetCursorScreenPos(start);
			InvisibleButton(str_id, ImVec2(end.x - start.x, end.y - start.y));
			if (IsItemActive() && IsMouseDragging(0)) {
				bool check = true;
				auto delta = GetIO().MouseDelta.x / (ImGui::GetColumnWidth() - GImGui->Style.ScrollbarSize) * s_max_timeline_value * range01;

				if (delta < 0) {
					auto tmp = values[0] + delta;
					if (tmp < 0) delta -= tmp;
				} else {
					auto tmp = values[1] + delta;
					if (tmp > s_max_timeline_value) delta -= (tmp - s_max_timeline_value);
				}

				values[0] += delta;
				values[1] += delta;
				changed = hovered = true;
			}
			PopID();

			SetCursorScreenPos(ImVec2(cursor_pos.x, cursor_pos.y + GetTextLineHeightWithSpacing()));
			win->DrawList->AddRectFilled(start, end, IsItemActive() || IsItemHovered() ? active_color : inactive_color);
			hovered |= (IsItemActive() || IsItemHovered());

			win->DrawList->AddCircleFilled(p[0], TIMELINE_RADIUS, pact[0] ? active_color : inactive_color);
			win->DrawList->AddCircleFilled(p[1], TIMELINE_RADIUS, pact[1] ? active_color : inactive_color);

			if (hovered) ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeEW);
			ImGui::NextColumn();

			val1 = values[0];
			val2 = values[1];

			return changed;
		}

		static void EndTimeline(int num_vertical_grid_lines, double &time_in, double &time_out, const double &range);
	};

}