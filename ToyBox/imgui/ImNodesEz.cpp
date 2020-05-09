//
// Copyright (c) 2019 Rokas Kupstys.
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to deal
// in the Software without restriction, including without limitation the rights
// to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
// copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
// OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
// THE SOFTWARE.
//
#ifndef IMGUI_DEFINE_MATH_OPERATORS
#   define IMGUI_DEFINE_MATH_OPERATORS
#endif
#include "ImNodesEz.h"

namespace ImNodes {

	unsigned int MyNode::animLen = 30;

	//extern CanvasState* gCanvas;
	bool BeginNodeEz(CanvasState* canvas, void* node_id, const char* title, ImVec2* pos, bool* selected) {
		bool result = ImNodes::BeginNode(canvas, node_id, pos, selected);

		auto* storage = ImGui::GetStateStorage();
		float node_width = storage->GetFloat(ImGui::GetID("node-width"));
		if (node_width > 0) {
			// Center node title
			ImVec2 title_size = ImGui::CalcTextSize(title);
			if (node_width > title_size.x)
				ImGui::SetCursorPosX(ImGui::GetCursorPosX() + (node_width - title_size.x) / 2.f);
		}

		// Render node title
		ImGui::TextUnformatted(title);
		ImGui::Spacing();

		ImGui::BeginGroup();
		return result;
	}

	void EndNodeEz(CanvasState* canvas, const bool& blockMouseAction, const unsigned int& anythingWrong) {
		// Store node width which is needed for centering title.
		auto* storage = ImGui::GetStateStorage();
		ImGui::EndGroup();
		storage->SetFloat(ImGui::GetID("node-width"), ImGui::GetItemRectSize().x);
		ImNodes::EndNode(canvas, blockMouseAction, anythingWrong);
	}

	bool Slot(CanvasState* canvas, const std::string &title, int kind) {
		auto* storage = ImGui::GetStateStorage();
		const auto& style = ImGui::GetStyle();
		const float CIRCLE_RADIUS = 5.f * canvas->zoom;
		ImVec2 title_size = ImGui::CalcTextSize(title.c_str());
		// Pull entire slot a little bit out of the edge so that curves connect into int without visible seams
		float item_offset_x = style.ItemSpacing.x * canvas->zoom;
		if (!ImNodes::IsOutputSlotKind(kind)) item_offset_x = -item_offset_x;
		ImGui::SetCursorScreenPos(ImGui::GetCursorScreenPos() + ImVec2{ item_offset_x, -style.ItemSpacing.y });

		if (ImNodes::BeginSlot(canvas, title.c_str(), kind)) {
			auto* draw_lists = ImGui::GetWindowDrawList();

			// Slot appearance can be altered depending on curve hovering state.
			bool is_active = ImNodes::IsSlotCurveHovered(canvas) ||
				(ImNodes::IsConnectingCompatibleSlot(canvas) /*&& !IsAlreadyConnectedWithPendingConnection(title, kind)*/);

			ImColor color = canvas->colors[is_active ? ImNodes::ColConnectionActive : ImNodes::ColConnection];

			ImGui::PushStyleColor(ImGuiCol_Text, color.Value);

			if (ImNodes::IsOutputSlotKind(kind)) {
				// Align output slots to the right edge of the node.
				ImGuiID max_width_id = ImGui::GetID("output-max-title-width");
				float output_max_title_width = ImMax(storage->GetFloat(max_width_id, title_size.x), title_size.x);
				storage->SetFloat(max_width_id, output_max_title_width);
				//float offset = (output_max_title_width + style.ItemSpacing.x) - title_size.x;
				//ImGui::SetCursorPosX(ImGui::GetCursorPosX() + offset);

				ImGui::TextUnformatted(title.c_str());
				ImGui::SameLine();
				ImGui::SetCursorPosX(ImGui::GetCursorPosX() - style.ItemSpacing.x * (1.0f - canvas->zoom));
			}

			ImRect circle_rect{
				ImGui::GetCursorScreenPos(),
				ImGui::GetCursorScreenPos() +
				ImVec2{CIRCLE_RADIUS * 2, CIRCLE_RADIUS * 2}
			};
			// Vertical-align circle in the middle of the line.
			float circle_offset_y = title_size.y / 2.f - CIRCLE_RADIUS;
			circle_rect.Min.y += circle_offset_y;
			circle_rect.Max.y += circle_offset_y;
			draw_lists->AddCircleFilled(circle_rect.GetCenter(), CIRCLE_RADIUS, color);

			ImGui::ItemSize(circle_rect.GetSize());
			ImGui::ItemAdd(circle_rect, ImGui::GetID(title.c_str()));

			if (ImNodes::IsInputSlotKind(kind)) {
				ImGui::SameLine();
				ImGui::SetCursorPosX(ImGui::GetCursorPosX() - style.ItemSpacing.x * (1.0f - canvas->zoom));
				ImGui::TextUnformatted(title.c_str());
			}

			ImGui::PopStyleColor();
			ImNodes::EndSlot(canvas);

			// A dirty trick to place output slot circle on the border.
			ImGui::GetCurrentWindow()->DC.CursorMaxPos.x -= item_offset_x;
			return true;
		}
		return false;
	}

	void InputSlotsEz(CanvasState* canvas, const SlotInfo* slots, int snum) {
		if (snum > 0) {
			const auto& style = ImGui::GetStyle();
			int visibleCnt = -1;
			// Render input slots
			ImGui::BeginGroup();
			{
				for (int i = 0; i < snum; i++) if (!slots[i].hideSlot) {
					visibleCnt++;
					ImNodes::Slot(canvas, slots[i].title, ImNodes::InputSlotKind(slots[i].kind));
				}
			}
			ImGui::EndGroup();

			// Move cursor to the next column
			ImGui::SetCursorScreenPos({ ImGui::GetItemRectMax().x - style.ItemSpacing.x * canvas->zoom * visibleCnt, ImGui::GetItemRectMin().y });
		}
		// Begin region for node content
		ImGui::BeginGroup();
	}

	void OutputSlotsEz(CanvasState* canvas, const SlotInfo* slots, int snum) {
		//const auto& style = ImGui::GetStyle();
		// End region of node content
		ImGui::EndGroup();
		if (snum > 0) {
			// Render output slots in the next column
			ImGui::SetCursorScreenPos({ ImGui::GetItemRectMax().x, ImGui::GetItemRectMin().y });
			ImGui::BeginGroup();
			{
				for (int i = 0; i < snum; i++) if (!slots[i].hideSlot)
					ImNodes::Slot(canvas, slots[i].title, ImNodes::OutputSlotKind(slots[i].kind));
			}
			ImGui::EndGroup();
		}
	}

}
