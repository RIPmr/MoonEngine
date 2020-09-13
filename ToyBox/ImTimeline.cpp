#include <iostream>

#include "ImTimeline.h"
#include "Icons.h"

namespace ImGui {

	float ImTimeline::s_max_timeline_value = 100.f;
	float ImTimeline::s_pixel_offset = 0.f;

	double ImTimeline::s_time_in = 0.f;
	double ImTimeline::s_time_out = 1.f;

	double ImTimeline::s_time_offset = 0;
	double ImTimeline::s_time_scale = 1;

	const float ImTimeline::TIMELINE_RADIUS = 6;

	void ImTimeline::EndTimeline(int num_vertical_grid_lines, double &time_in, double &time_out, const double &range) {
		ImGui::NextColumn();

		ImGuiWindow* win = GetCurrentWindow();

		const float columnOffset = ImGui::GetColumnOffset(1);
		const float columnWidth = ImGui::GetColumnWidth(1) - GImGui->Style.ScrollbarSize;
		const float horizontal_interval = columnWidth / num_vertical_grid_lines;

		const ImU32 pz_inactive_color = ColorConvertFloat4ToU32(GImGui->Style.Colors[ImGuiCol_SliderGrab]);
		const ImU32 pz_active_color = ColorConvertFloat4ToU32(GImGui->Style.Colors[ImGuiCol_SliderGrabActive]);
		//const ImU32 pz_line_color = ColorConvertFloat4ToU32(GImGui->Style.Colors[ImGuiCol_ColumnActive]);

		const ImU32 color = ColorConvertFloat4ToU32(GImGui->Style.Colors[ImGuiCol_SliderGrab]);
		const ImU32 line_color = ColorConvertFloat4ToU32(GImGui->Style.Colors[ImGuiCol_Border]);
		const ImU32 text_color = ColorConvertFloat4ToU32(GImGui->Style.Colors[ImGuiCol_Text]);
		const float rounding = GImGui->Style.ScrollbarRounding;
		const float startY = ImGui::GetWindowHeight() + win->Pos.y;

		// vertical lines
		for (int i = 0; i <= num_vertical_grid_lines; ++i) {
			ImVec2 a(GetWindowContentRegionMin().x + win->Pos.x, GetWindowContentRegionMin().y + win->Pos.y);
			a.x += s_time_scale * i * horizontal_interval + columnOffset - columnWidth * s_time_offset;
			win->DrawList->AddLine(a, ImVec2(a.x, startY), line_color);
		}

		ImGui::Columns(1);
		//ImGui::PopStyleColor();

		EndChild();

		// draw bottom axis ribbon outside scrolling region
		win = GetCurrentWindow();

		ImGui::SetCursorPos(ImVec2(GetWindowContentRegionMin().x + TIMELINE_RADIUS, ImGui::GetCursorPosY() - 5));
		ImGui::Button(ICON_FA_COG, ImVec2(22, 22));
		float startx = ImGui::GetCursorScreenPos().x + columnOffset;
		float endy = GetWindowContentRegionMax().y + win->Pos.y;
		ImVec2 start(startx, endy - 2 * ImGui::GetTextLineHeightWithSpacing());
		ImVec2 end(startx + columnWidth, endy - ImGui::GetTextLineHeightWithSpacing());
		win->DrawList->AddRectFilled(ImVec2(start.x - 5.0f, start.y), ImVec2(end.x + 10.0f, end.y), color, rounding);
		/*win->DrawList->AddRectFilled(ImVec2(start.x - 5.0f, start.y), ImVec2(start.x +
			10.0f + s_time_scale * num_vertical_grid_lines * horizontal_interval -
			columnWidth * s_time_offset, endy - ImGui::GetTextLineHeightWithSpacing()),
			ColorConvertFloat4ToU32(GImGui->Style.Colors[ImGuiCol_ButtonHovered]), rounding);*/

		char tmp[256] = "";
		for (int i = 0; i <= num_vertical_grid_lines; ++i) {
			ImVec2 a = start;
			ImFormatString(tmp, sizeof(tmp), "%.2f", i * s_max_timeline_value / num_vertical_grid_lines);
			auto textWidth = ImGui::CalcTextSize(tmp).x;
			a.x = start.x + s_time_scale * i * horizontal_interval - columnWidth * s_time_offset - (i == num_vertical_grid_lines ? textWidth : 0);
			if (a.x < startx) continue;
			a.y = start.y;
			if (a.x <= end.x - textWidth) win->DrawList->AddText(a, text_color, tmp);
		}

		// draw time panzoomer
		float posx[2] = { 0, 0 }; ImVec2 p[2]; bool pact[2];
		double values[2] = { time_in, time_out };

		bool hovered = false;
		bool changed = false;
		ImVec2 cursor_pos = { start.x - TIMELINE_RADIUS, end.y };

		for (int i = 0; i < 2; ++i) {
			ImVec2 pos = cursor_pos;
			pos.x += columnWidth * float(values[i]) + TIMELINE_RADIUS;
			pos.y += TIMELINE_RADIUS;
			posx[i] = pos.x;

			SetCursorScreenPos(ImVec2(pos.x - TIMELINE_RADIUS, pos.y - TIMELINE_RADIUS));
			PushID(i);
			InvisibleButton("zoompanner", ImVec2(2 * TIMELINE_RADIUS, 2 * TIMELINE_RADIUS));
			if (IsItemActive() || IsItemHovered()) {
				if (ImGui::IsWindowHovered()) ImGui::SetTooltip("%f", float(values[i] * range));
				hovered = true;
			}
			if (IsItemActive() && IsMouseDragging(0)) {
				values[i] += GetIO().MouseDelta.x / columnWidth;
				changed = hovered = true;
			}
			PopID();
			p[i] = pos; pact[i] = IsItemActive() || IsItemHovered();
		}

		start.x = posx[0];
		start.y += TIMELINE_RADIUS * 0.5f + ImGui::GetTextLineHeightWithSpacing();
		end.x = posx[1]; end.y = start.y + TIMELINE_RADIUS;

		PushID(-1);
		SetCursorScreenPos(start);

		auto width = end.x - start.x, height = end.y - start.y;
		if (!width) width = 0.001f; if (!height) height = 0.001f;
		InvisibleButton("zoompanner", ImVec2(width, height));
		if (IsItemActive() && IsMouseDragging(0)) {
			auto delta = GetIO().MouseDelta.x / columnWidth;

			if (delta < 0) {
				auto tmp = values[0] + delta;
				if (tmp < 0) delta -= tmp;
			} else {
				auto tmp = values[1] + delta;
				if (tmp > 1) delta -= (tmp - 1);
			}

			values[0] += delta;
			values[1] += delta;
			changed = hovered = true;
		}
		PopID();

		win->DrawList->AddRectFilled(start, end, IsItemActive() || IsItemHovered() ? pz_active_color : pz_inactive_color);
		win->DrawList->AddCircleFilled(p[0], TIMELINE_RADIUS, pact[0] ? pz_active_color : pz_inactive_color);
		win->DrawList->AddCircleFilled(p[1], TIMELINE_RADIUS, pact[1] ? pz_active_color : pz_inactive_color);

		hovered |= (IsItemActive() || IsItemHovered());
		if (hovered && ImGui::IsWindowHovered()) ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeEW);

		for (int i = 0; i < 2; ++i) {
			if (values[i] < 0) values[i] = 0;
			else if (values[i] > 1) values[i] = 1;
		}

		time_in = values[0];
		time_out = values[1];

		s_time_in = time_in;
		s_time_out = time_out;

		ImGui::SetCursorPosY(ImGui::GetCursorPosY() + 2 * ImGui::GetTextLineHeightWithSpacing());
	}
}