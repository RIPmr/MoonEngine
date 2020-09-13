#include <string>

#include "NodeEditorBase.h"
#include "SceneMgr.h"
#include "MathUtils.h"
#include "Utility.h"

namespace MOON {

	void NodeEditor::Searcher::Draw() {
		// press 'S' to show searcher
		if (!enableSearcher && MOON_KeyDown(KEY_S)) {
			enableSearcher = true;
			mousePos = ImVec2(ImGui::GetMousePos().x - ImGui::GetWindowPos().x, ImGui::GetMousePos().y - ImGui::GetWindowPos().y);
		}

		// start drawing
		if (enableSearcher) {
			ImGui::SetWindowFontScale(1.f);
			char buf[64]; strcpy(buf, searchWord.c_str());
			ImGui::SetCursorPosX(mousePos.x);
			ImGui::SetCursorPosY(mousePos.y - std::min(4, (int)resVerbs.size()) * 20.0f);
			childSize.y = 40 + (std::min(6, (int)resNodes.size()) + std::min(4, (int)resVerbs.size())) * 20.0f;
			ImGui::BeginChild("searchFrame", childSize, true, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse);
			ImGui::SetKeyboardFocusHere();

			// draw verbs
			if (enableFlowSense && resVerbs.size() > 0 && ImGui::ListBoxHeader("verbList", ImVec2(childSize.x - 15, std::min(4, (int)resVerbs.size()) * 20.0f))) {
				ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(100, 100, 255, 255));
				for (int i = 0; i < resVerbs.size(); i++) {
					ImGui::Selectable(resVerbs[i]->str.c_str(), selection == i + resNodes.size());

					// TODO: confirm input by clicking
					if (ImGui::IsItemClicked()) {
						selection = i + resNodes.size();
						ImGui::SetScrollHereY();
					}
					if (selection == i + resNodes.size() && 
						(MOON_KeyDown(KEY_DOWN) || MOON_KeyDown(KEY_UP) || 
						 MOON_KeyRelease(KEY_DOWN) || MOON_KeyRelease(KEY_UP)))
						ImGui::SetScrollHereY();
					if (resetVerbViewScroll && i == resVerbs.size() - 1) {
						resetVerbViewScroll = false;
						ImGui::SetScrollHereY();
					}
				}
				ImGui::PopStyleColor();
				ImGui::ListBoxFooter();
			}

			// draw query Word(s)
			childSize.x = 170.0f;
			for (auto &q : query) {
				if (q.first == node) {
					ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV(1.0f, 0.6f, 0.6f));
					//ImGui::PushStyleColor(ImGuiCol_ButtonHovered, (ImVec4)ImColor::HSV(1.0f, 0.7f, 0.7f));
					//ImGui::PushStyleColor(ImGuiCol_ButtonActive, (ImVec4)ImColor::HSV(1.0f, 0.8f, 0.8f));
					ImGui::Button(((ImNodes::MyNode*)q.second)->title.c_str()); ImGui::SameLine();
					ImGui::PopStyleColor(1);
				} else if (q.first == verb) {
					ImGui::PushStyleColor(ImGuiCol_Button, (ImVec4)ImColor::HSV(0.6f, 0.6f, 1.0f));
					ImGui::Button(((Verb*)q.second)->str.c_str()); ImGui::SameLine();
					ImGui::PopStyleColor(1);
				}
				childSize.x += ImGui::GetItemRectSize().x + 8.0f;
			}

			// process user input
			if (MOON_KeyDown(KEY_SPACE)) { // confirm input
				searchWord = "";
				if (enableFlowSense) {
					for (int i = 0; i < resVerbs.size(); i++) {
						if (i + resNodes.size() == selection) query.push_back(Word(verb, resVerbs[selection - resNodes.size()]));
						else delete resVerbs[i];
					}
				}
				for (int i = 0; i < resNodes.size(); i++) {
					if (i == selection) query.push_back(Word(node, resNodes[selection]));
					else delete resNodes[i];
				}
				resNodes.clear(); resVerbs.clear();
			} else if (searchWord.length() < 1 && MOON_KeyDown(KEY_BACKSPACE)) { // delete Word(s)
				if (query.size() > 0) {
					if (query.back().first == node) delete ((ImNodes::MyNode*)query.back().second);
					else delete ((Verb*)query.back().second);
					query.pop_back();
				}
			} else { // if input changes, clear last search result and restart searching
				ImGui::SetNextItemWidth(150.0f);
				if (ImGui::InputTextWithHint("nodeSearcher", "input and search", buf, 64)) {
					searchWord = buf; selection = 0;
					for (auto &node : resNodes) delete node;
					resNodes = parent->SearchNodes_Fuzzy(searchWord);
					if (enableFlowSense) {
						resetVerbViewScroll = true;
						resVerbs = parent->SearchVerbs_Fuzzy(searchWord);
						std::reverse(resVerbs.begin(), resVerbs.end());
					}
				}
				// parse query
				if (ImGui::IsItemDeactivated() && MOON_KeyDown(KEY_ENTER)) {
					enableSearcher = false;
					std::cout << "parse query..." << std::endl;
					ImVec2 viewCenter((parent->canvas->viewStart.x + parent->canvas->viewEnd.x) / 2.0f - 400.0f, (parent->canvas->viewStart.y + parent->canvas->viewEnd.y) / 2.0f - 100.0f);
					for (int i = 0; i < query.size(); i++) {
						if (query[i].first == node) {
							parent->Instantiate((ImNodes::MyNode*)query[i].second, viewCenter);
							viewCenter.x += 150.0f;
						} else if (query[i].first == verb) {
							Word& fore = i ? query[i - 1] : query[i];
							Word& behind = i == (query.size() - 1) ? query[i] : query[i + 1];
							((Verb*)query[i].second)->execute(fore, behind, this);
							delete (Verb*)query[i].second;
						}
					}
					query.clear(); searchWord = "";
					for (auto &node : resNodes) delete node;
					for (auto &verb : resVerbs) delete verb;
					resNodes.clear(); resVerbs.clear();
					std::cout << "done." << std::endl;
				}
			}

			// loop search result
			if (MOON_KeyDown(KEY_DOWN) || MOON_KeyRepeat(KEY_DOWN)) {
				selection++;
				if (selection >= resNodes.size() + (enableFlowSense ? resVerbs.size() : 0))
					selection = 0;
			} else if (MOON_KeyDown(KEY_UP) || MOON_KeyRepeat(KEY_UP)) {
				selection--;
				if (selection < 0)
					selection = resNodes.size() + (enableFlowSense ? resVerbs.size() : 0) - 1;
			}

			// showing search result
			if (resNodes.size() > 0 && ImGui::ListBoxHeader("resList", ImVec2(childSize.x - 15, std::min(6, (int)resNodes.size()) * 20.0f))) {
				for (int i = 0; i < resNodes.size(); i++) {
					ImGui::Selectable(resNodes[i]->title.c_str(), selection == i);

					// TODO: confirm input by clicking
					if (ImGui::IsItemClicked()) {
						selection = i;
						ImGui::SetScrollHereY();
					}
					if (selection == i && (MOON_KeyDown(KEY_DOWN) || MOON_KeyDown(KEY_UP)))
						ImGui::SetScrollHereY();
				}
				ImGui::ListBoxFooter();
			}

			ImGui::EndChild();
			ImGui::SetWindowFontScale(parent->canvas->zoom);
		}

		// blocking operations while mouse hovering rect
		ImVec2 rectmin = ImVec2(mousePos.x + ImGui::GetWindowPos().x, mousePos.y - std::min(4, (int)resVerbs.size()) * 20.0f + ImGui::GetWindowPos().y);
		if (!ImGui::IsMouseHoveringRect(rectmin, ImVec2(rectmin.x + childSize.x, rectmin.y + childSize.y)) &&
			(ImGui::GetIO().MouseWheel != 0 || ImGui::IsAnyMouseDown())) {
			enableSearcher = false;
			for (auto &node : resNodes) delete node; resNodes.clear();
			searchWord = "";
		}
	}

	void NodeEditor::Instantiate(ImNodes::MyNode* node, const ImVec2& nodePos = ImVec2(0, 0)) {
		nodes.push_back(node);
		nodes.back()->parent = this;
		OnCreateNode(nodes.back());
		if (!nodePos.x && !nodePos.y) ImNodes::AutoPositionNode(canvas, nodes.back());
		else node->pos = nodePos;
	}

	void NodeEditor::ListSlots(const std::string &typeName) {
		auto lower = available_nodes->lower_bound(typeName);
		auto upper = available_nodes->upper_bound(typeName);

		for (; lower != upper; lower++) {
			auto node = (*lower).second();
			node->type = typeName;
			if (ImGui::MenuItem(node->title.c_str())) {
				Instantiate(node);
			} else delete node;
		}
	}

	void NodeEditor::Draw() {
		/*
			*NOTE: canvas must be created after ImGui initializes, because
			constructor accesses ImGui style to configure default colors.
		*/
		//static ImNodes::CanvasState canvas{};
		//const ImGuiStyle& style = ImGui::GetStyle();
		if (canvas == NULL) canvas = new ImNodes::CanvasState();

		ImNodes::BeginCanvas(canvas, blockMouseAction);
		if (ImGui::IsWindowHovered() && !ImGui::IsWindowFocused() && 
		   (ImGui::GetIO().MouseWheel != 0 || ImGui::IsMouseDown(2))) {
			ImGui::FocusWindow(ImGui::GetCurrentWindow());
		}

		// searcher
		searcher.Draw();

		// restore props
		{
			blockMouseAction = false;
			selectedNode = NULL;
			anythingSelected = false;
		}

		// loop all nodes
		for (auto it = nodes.begin(); it != nodes.end();) {
			ImNodes::MyNode* node = *it;
			unsigned int anythingWrong = 0;

			node->zoomFactor = canvas->zoom;

			if (node->selected) {
				anythingSelected = true;
				selectedNode = node;
				/*std::cout << "- Connection: -" << std::endl;
				for (auto &iter : node->input_slots) {
					if (iter.con != NULL)
					std::cout << iter.title << " is con to: " << ((MyNode*)iter.parent)->title << std::endl;
				}*/
			}

			// Start rendering node
			if (ImNodes::BeginNodeEz(canvas, node, node->title.c_str(), &node->pos, &node->selected)) {

				// Render input nodes first (order is important)
				ImNodes::InputSlotsEz(canvas, node->input_slots.data(), node->input_slots.size());

				ImGui::SetCursorPosY(ImGui::GetCursorPosY() - ImGui::GetStyle().ItemSpacing.y);

				// node content go here
				if (node->animCnt > 0) anythingWrong = 2;
				try { node->ProcessContent(true); } catch (...) {
					ImGui::TextUnformatted(ICON_FA_BAN);
					anythingWrong = 1;
				}

				// Render output nodes first (order is important)
				ImNodes::OutputSlotsEz(canvas, node->output_slots.data(), node->output_slots.size());

				// Store new connections when they are created
				ImNodes::MyConnection new_connection;
				if (ImNodes::GetNewConnection(canvas, &new_connection.input_node, &new_connection.input_slot,
					&new_connection.output_node, &new_connection.output_slot)) {
					ImNodes::MyNode* inNode = (ImNodes::MyNode*)new_connection.input_node;
					ImNodes::MyNode* outNode = (ImNodes::MyNode*)new_connection.output_node;

					inNode->connections.push_back(new_connection);
					outNode->connections.push_back(new_connection);

					/// store connection ptr to input slot and output slot
					auto islot = inNode->GetSlot(ImNodes::In, new_connection.input_slot);
					auto oslot = outNode->GetSlot(ImNodes::Out, new_connection.output_slot);
					islot->con = oslot;
					islot->parent = outNode;
					oslot->childs.push_back(inNode);

					try {
						OnCreateConnection(new_connection, inNode, outNode, islot, oslot);
						outNode->Backward();
					} catch (...) {
						anythingWrong = 1;
					}
				}

				// Render output connections of this node
				for (const ImNodes::MyConnection& connection : node->connections) {
					/// Node contains all it's connections (both from output and to input slots). This means that multiple
					/// nodes will have same connection. We render only output connections and ensure that each connection
					/// will be rendered once.
					if (connection.output_node != node) {
						if (node->GetSlot(ImNodes::In, connection.input_slot)->hideSlot) {
							DeleteConnection(connection);
							break;
						}
					}

					if (!ImNodes::Connection(canvas, connection.input_node, connection.input_slot,
						connection.output_node, connection.output_slot)) {
						DeleteConnection(connection);
						break;
					}
				}
			}
			// Node rendering is done. This call will render node background based on size of content inside node.
			ImNodes::EndNodeEz(canvas, blockMouseAction, anythingWrong);

			if (node->selected && ImGui::IsKeyPressedMap(ImGuiKey_Delete)) {
				if (anythingSelected && selectedNode == node) {
					anythingSelected = false;
					selectedNode = NULL;
				}
				DeleteNode(node);
				it = nodes.erase(it);
			} else ++it;
		}

		ImGui::SetWindowFontScale(1.f);
		OnDrawGraph();

		if (!blockMouseAction) {
			const ImGuiIO& io = ImGui::GetIO();
			if (ImGui::IsMouseReleased(1) && ImGui::IsWindowHovered() && !ImGui::IsMouseDragging(1)) {
				canvas->lastMousePos = ImGui::GetMousePos();
				ImGui::FocusWindow(ImGui::GetCurrentWindow());
				ImGui::OpenPopup("NodesContextMenu");
			}
			if (ImGui::BeginPopup("NodesContextMenu")) {
				PopMenu();
				ImGui::EndPopup();
			}
		}
		ImNodes::EndCanvas(canvas, blockMouseAction);
	}

	void NodeEditor::DefineDefaultVerbs() {
		available_verbs = new Verbs{
			[]() -> Verb* { return new Verb(
				"link to",
				[](Word& fore, Word& behind, void* context) {
					if (fore.first == node && behind.first == node){
						ImNodes::MyNode* outNode = (ImNodes::MyNode*)fore.second;
						ImNodes::MyNode* inNode = (ImNodes::MyNode*)behind.second;
						
						const char*(*GetInSlot)(ImNodes::MyNode*) = 
						[](ImNodes::MyNode* inNode) -> const char* {
							for (auto &slot : inNode->input_slots) {
								if (!slot.hideSlot) return slot.title.c_str();
							}
						};

						((NodeEditor*)context)->CreateConnection(inNode, outNode, GetInSlot(inNode), outNode->output_slots[0].title.c_str());
					} else throw -1;
				}
			);},
			[]() -> Verb* { return new Verb(
				"plot",
				[](Word& fore, Word& behind, void* context) {}
			); },
			[]() -> Verb* { return new Verb(
				"colored",
				[](Word& fore, Word& behind, void* context) {}
			); },
			[]() -> Verb* { return new Verb(
				"red",
				[](Word& fore, Word& behind, void* context) {}
			); },
			[]() -> Verb* { return new Verb(
				"orange",
				[](Word& fore, Word& behind, void* context) {}
			); },
			[]() -> Verb* { return new Verb(
				"yellow",
				[](Word& fore, Word& behind, void* context) {}
			); },
			[]() -> Verb* { return new Verb(
				"green",
				[](Word& fore, Word& behind, void* context) {}
			); },
			[]() -> Verb* { return new Verb(
				"cyan",
				[](Word& fore, Word& behind, void* context) {}
			); },
			[]() -> Verb* { return new Verb(
				"blue",
				[](Word& fore, Word& behind, void* context) {}
			); },
			[]() -> Verb* { return new Verb(
				"purple",
				[](Word& fore, Word& behind, void* context) {}
			); },
			[]() -> Verb* { return new Verb(
				"black",
				[](Word& fore, Word& behind, void* context) {}
			); },
			[]() -> Verb* { return new Verb(
				"white",
				[](Word& fore, Word& behind, void* context) {}
			); }
		};
	}

}