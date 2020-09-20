#include "OperatorMgr.h"
#include "ObjectBase.h"
#include "SceneMgr.h"
#include <cstring>

namespace MOON {
	Operators*				OperatorManager::available_operators;
	std::vector<Operator*>	OperatorManager::matchList;

	bool					OperatorManager::showList = false;
	bool					OperatorManager::focusKey = false;
	char					OperatorManager::buf[64] = "";
	int						OperatorManager::selection = 0;
	unsigned int			OperatorManager::parentID = -1;

	void OperatorManager::SearchOps_Fuzzy(const char* typeName) {
		std::map<int, Operator*> fuzzyRes;

		for (auto lower = available_operators->begin(); lower != available_operators->end(); lower++) {
			int score = -INFINITY_INT;
			auto node = (*lower).second();
			node->type = lower->first;
			std::string name = node->name;
			MatchTool::fuzzy_match(typeName, name.c_str(), score);
			if (score > 0) fuzzyRes.insert(std::pair<int, Operator*>(score, node));
			else delete node;
		}

		for (auto it = fuzzyRes.rbegin(); it != fuzzyRes.rend(); it++)
			matchList.push_back((*it).second);
	}

	void OperatorManager::Instantiate(Operator* op, MObject::OPStack& opstack) {
		opstack.Add(op);
	}

	void OperatorManager::ListOperators(const ImVec2& position, MObject::OPStack& opstack) {
		if (showList) {
			//ImGui::SetCursorPos(position);
			ImVec2 availableSize = ImGui::GetContentRegionAvail();
			availableSize.y = 20 * std::min(10, (int)(matchList.empty() ? available_operators->size() : matchList.size())) + 50;
			ImGui::BeginChild(
				"searchFrame",
				availableSize, true,
				ImGuiWindowFlags_NoScrollbar |
				ImGuiWindowFlags_NoScrollWithMouse
			);

			bool notEmpty = strcmp(buf, "");
			if (!notEmpty && focusKey) {
				focusKey = false;
				ImGui::SetKeyboardFocusHere();
			}
			ImGui::SetNextItemWidth(availableSize.x - (notEmpty ? 45 : 15));
			// if input changes, clear last search result and restart searching
			if (ImGui::InputTextWithHint("opSearcher", "type and search", buf, IM_ARRAYSIZE(buf))) {
				Utility::ReleaseVector(matchList);
				selection = 0;
				SearchOps_Fuzzy(buf);
				availableSize.y = 20 * std::min(10, (int)(matchList.empty() ? available_operators->size() : matchList.size())) + 50;
			}
			if (notEmpty) {
				ImGui::SameLine();
				if (ImGui::Button(ICON_FA_TIMES, ImVec2(22, 22))) {
					sprintf_s(buf, "");
					Utility::ReleaseVector(matchList);
					selection = 0;
				}
			}

			// loop search result
			int maxNum = matchList.size() ? matchList.size() : available_operators->size();
			if (MOON_KeyDown(KEY_DOWN) || MOON_KeyRepeat(KEY_DOWN)) {
				selection++;
				if (selection >= maxNum) selection = 0;
			} else if (MOON_KeyDown(KEY_UP) || MOON_KeyRepeat(KEY_UP)) {
				selection--;
				if (selection < 0) selection = maxNum - 1;
			}

			// showing search result
			if (ImGui::ListBoxHeader("opList", ImVec2(availableSize.x - 15, availableSize.y - 45))) {
				if (!matchList.empty()) {
					for (int i = 0; i < matchList.size(); i++) {
						if (selection == i && (MOON_KeyDown(KEY_DOWN) || MOON_KeyDown(KEY_UP)))
							ImGui::SetScrollHereY();

						ImGui::Selectable(matchList[i]->name.c_str(), selection == i);
						if (ImGui::IsItemClicked() || (selection == i && MOON_KeyDown(KEY_ENTER))) {
							Instantiate(matchList[i], opstack);
							for (int j = 0; j < matchList.size(); j++)
								if (j != i) delete matchList[j];
							matchList.clear();
							showList = false;
							sprintf_s(buf, "");
							break;
						}
					}
				} else if (notEmpty) {
					ImGui::Selectable("no result", false, ImGuiSelectableFlags_Disabled);
				} else {
					unsigned int loop = 0;
					ListCategory("Model", opstack, loop);
					ListCategory("Spline", opstack, loop);
					ListCategory("Shape", opstack, loop);
				}
				ImGui::ListBoxFooter();
			}

			ImGui::EndChild();

			// blocking operations while mouse hovering rect
			ImVec2 rectmin = ImVec2(
				position.x + ImGui::GetWindowPos().x,
				position.y + ImGui::GetWindowPos().y - ImGui::GetScrollY()
			);
			if (!ImGui::IsMouseHoveringRect(rectmin,
				ImVec2(rectmin.x + availableSize.x, rectmin.y + availableSize.y)) &&
				ImGui::IsAnyMouseDown()) {
				showList = false;
				sprintf_s(buf, "");
				Utility::ReleaseVector(matchList);
			}
		}
	}

	void OperatorManager::ListCategory(const std::string &typeName, MObject::OPStack& opstack, unsigned int& loop) {
		auto lower = available_operators->lower_bound(typeName);
		auto upper = available_operators->upper_bound(typeName);

		// category title
		ImGui::SetWindowFontScale(0.8f);
		ImGui::Selectable(typeName.c_str(), false, ImGuiSelectableFlags_Disabled);
		ImGui::SetWindowFontScale(1.0f);

		// operators
		for (; lower != upper; lower++) {
			auto op = (*lower).second();
			op->type = typeName;

			if (selection == loop && (MOON_KeyDown(KEY_DOWN) || MOON_KeyDown(KEY_UP)))
				ImGui::SetScrollHereY();

			ImGui::Selectable(op->name.c_str(), selection == loop);
			if (ImGui::IsItemClicked() || (selection == loop++ && MOON_KeyDown(KEY_ENTER))) {
				Instantiate(op, opstack);
				showList = false;
				sprintf_s(buf, "");
			} else delete op;
		}
	}
}