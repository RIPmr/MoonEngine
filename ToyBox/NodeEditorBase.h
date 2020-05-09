#pragma once
#include <map>
#include <vector>
#include <string>
#include <imgui.h>

#include "Vector3.h"
#include "Vector4.h"
#include "Color.h"
#include "Matrix4x4.h"
#include "ImNodesEz.h"
#include "IconsFontAwesome4.h"

namespace MOON {
	#define Nodes						std::multimap<std::string, ImNodes::MyNode*(*)()>

	#define DataSizeDef					ImNodes::SlotData
	#define getSlot(type, name)			node->GetSlot(type, name)
	#define parentNode(slotID)			node->input_slots[slotID].parent
	#define parentSlot(slotID)			node->input_slots[slotID].con

	#define inputSlotCnt				node->input_slots.size()
	#define inputSlot(slotID)			node->input_slots[slotID]
	#define inputData(slotID)			node->input_slots[slotID].data
	#define inputID(slotID,dataID)		node->input_slots[slotID].data.id[dataID]
	#define inputVec(slotID,vecID)		node->input_slots[slotID].data.col[vecID]
	#define inputMat(slotID)			(*node->input_slots[slotID].data.mat)

	#define outputSlotCnt				node->output_slots.size()
	#define outputSlot(slotID)			node->output_slots[slotID]
	#define outputData(slotID)			node->output_slots[slotID].data
	#define outputID(slotID,dataID)		node->output_slots[slotID].data.id[dataID]
	#define outputVec(slotID,vecID)		node->output_slots[slotID].data.col[vecID]
	#define outputMat(slotID)			(*node->output_slots[slotID].data.mat)
	#define IMGUI_DEFINE_MATH_OPERATORS

	class NodeEditor {
	protected:
		void CleanUp() {
			for (auto it = nodes.begin(); it != nodes.end(); it++) {
				OnDeleteNode(*it);
				delete (*it);
			}
			if (available_nodes != NULL) {
				for (auto it = available_nodes->begin(); it != available_nodes->end(); it++) {
					delete (*it).second();
				}
			}
		}

	public:
		ImNodes::CanvasState* canvas = NULL;

		ImNodes::MyNode* selectedNode;
		bool anythingSelected;
		bool blockMouseAction = false;

		Nodes* available_nodes = NULL;
		std::vector<ImNodes::MyNode*> nodes;

		NodeEditor() = default;

		virtual ~NodeEditor() {
			CleanUp();
			if (available_nodes != NULL) 
				delete available_nodes;
		}

		virtual void PopMenu() {
			if (anythingSelected) {
				ImGui::Separator();
				if (ImGui::MenuItem("Delete")) {}
				if (ImGui::MenuItem("Copy")) {}
				if (ImGui::MenuItem("Paste")) {}
			}
		}

		virtual void Previewer(const ImGuiStyle* style) {
			//static bool open = true;
			//ImGui::Begin("Previewer", &open);
			if (canvas == NULL) return;

			ImDrawList* draw_list = ImGui::GetWindowDrawList();
			for (auto iter = nodes.begin(); iter != nodes.end(); iter++) {
				draw_list->AddRectFilled(Graph2Preview((*iter)->pos), Graph2Preview((*iter)->end), 
										 ImColor(255, 255, 255), style->FrameRounding);
			}
			draw_list->AddRectFilled(Graph2Preview(canvas->viewStart), Graph2Preview(canvas->viewEnd),
				ImColor(255, 255, 255, 50), style->FrameRounding);
			draw_list->AddRect(Graph2Preview(canvas->viewStart), Graph2Preview(canvas->viewEnd),
				ImColor(100, 255, 100, 255), style->FrameRounding);
			//ImGui::End();
		}

		ImVec2 Graph2Preview(const ImVec2& pos) {
			if (canvas == NULL) return ImVec2(0, 0);
			ImVec2 wndSize = ImGui::GetWindowSize();
			ImVec2 scale(wndSize.x / canvas->size.x, wndSize.y / canvas->size.y);
			return ImVec2(pos.x * scale.x + ImGui::GetWindowPos().x, pos.y * scale.y + ImGui::GetWindowPos().y);
		}

		void ListSlots(const std::string &typeName) {
			auto lower = available_nodes->lower_bound(typeName);
			auto upper = available_nodes->upper_bound(typeName);

			for (; lower != upper; lower++) {
				if (ImGui::MenuItem((*lower).second()->title.c_str())) {
					nodes.push_back((*lower).second());
					nodes.back()->parent = this;
					OnCreateNode(nodes.back(), typeName);
					ImNodes::AutoPositionNode(canvas, nodes.back());
				}
			}
		}

		void Draw() {
			/*
				*NOTE: canvas must be created after ImGui initializes, because
				constructor accesses ImGui style to configure default colors.
			*/
			//static ImNodes::CanvasState canvas{};
			//const ImGuiStyle& style = ImGui::GetStyle();
			if (this->canvas == NULL) this->canvas = new ImNodes::CanvasState();

			ImNodes::BeginCanvas(canvas, blockMouseAction);

			if (ImGui::IsWindowHovered() && !ImGui::IsWindowFocused() && 
			   (ImGui::GetIO().MouseWheel != 0 || ImGui::IsMouseDown(2)))
					ImGui::FocusWindow(ImGui::GetCurrentWindow());

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
					try { node->ProcessContent(true); } 
					catch (...) {
						ImGui::TextUnformatted(ICON_FA_BAN);
						anythingWrong = 1;
					}

					// Render output nodes first (order is important)
					ImNodes::OutputSlotsEz(canvas, node->output_slots.data(), node->output_slots.size());

					// Store new connections when they are created
					ImNodes::MyConnection new_connection;
					if (ImNodes::GetNewConnection(canvas, &new_connection.input_node, &new_connection.input_slot,
						&new_connection.output_node, &new_connection.output_slot)) {
						ImNodes::MyNode* conIn = (ImNodes::MyNode*)new_connection.input_node;
						ImNodes::MyNode* conOut = (ImNodes::MyNode*)new_connection.output_node;

						conIn->connections.push_back(new_connection);
						conOut->connections.push_back(new_connection);

						/// store connection ptr to input slot and output slot
						auto islot = conIn->GetSlot(ImNodes::In, new_connection.input_slot);
						auto oslot = conOut->GetSlot(ImNodes::Out, new_connection.output_slot);
						islot->con = oslot;
						islot->parent = (ImNodes::MyNode*)new_connection.output_node;
						oslot->childs.push_back((ImNodes::MyNode*)new_connection.input_node);

						try {
							OnCreateConnection(new_connection, conIn, conOut, islot, oslot);
							conOut->Backward();
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
					this->canvas->lastMousePos = ImGui::GetMousePos();
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

		// delete connection ptr in input slot
		void DeleteConnection(const ImNodes::MyConnection connection) {
			ImNodes::MyNode* conIn = (ImNodes::MyNode*)connection.input_node;
			ImNodes::MyNode* conOut = (ImNodes::MyNode*)connection.output_node;

			auto oslot = conOut->GetSlot(ImNodes::Out, connection.output_slot);
			auto islot = conIn->GetSlot(ImNodes::In, connection.input_slot);

			OnDeleteConnection(connection, conIn, conOut, islot, oslot);

			oslot->DeleteChild(conIn);
			islot->ResetParent();
			conIn->Backward();

			// Remove deleted connections
			conIn->DeleteConnection(connection);
			conOut->DeleteConnection(connection);
		}

		void DeleteAllConnections(ImNodes::MyNode* node) {
			for (auto &connection : node->connections) {
				/*ImNodes::MyNode* conIn = (ImNodes::MyNode*)connection.input_node;
				if (connection.output_node == node) {
					auto islot = conIn->GetSlot(ImNodes::In, connection.input_slot);
					islot->ResetParent();
					conIn->DeleteConnection(connection);
				} else {
					ImNodes::MyNode* conOut = (ImNodes::MyNode*)connection.output_node;
					auto oslot = conOut->GetSlot(ImNodes::Out, connection.output_slot);
					oslot->DeleteChild(conIn);
					conOut->DeleteConnection(connection);
				}*/
				DeleteConnection(connection);
			}
			node->connections.clear();
		}

		// Deletion order is critical: 
		// first we need to delete all connections to this node,
		// then we delete our own connections, so we don't corrupt the list
		void DeleteNode(ImNodes::MyNode* node) {
			DeleteAllConnections(node);
			OnDeleteNode(node);
			delete node;
		}

		// This callback method will be called after all nodes are drawn
		virtual void OnDrawGraph() {}
		// This callback method will be called when any node is created
		virtual void OnCreateNode(ImNodes::MyNode* node, const std::string& type) {}
		// This callback method will be called when any node is deleted
		virtual void OnDeleteNode(ImNodes::MyNode* node) {}
		// This callback method will be called when any connection is deleted
		virtual void OnDeleteConnection(const ImNodes::MyConnection &con, 
										ImNodes::MyNode* inNode, ImNodes::MyNode* outNode,
										ImNodes::SlotInfo* inSlot, ImNodes::SlotInfo* outSlot) {}
		// This callback method will be called when any connection is created
		virtual void OnCreateConnection(const ImNodes::MyConnection &con, 
										ImNodes::MyNode* inNode, ImNodes::MyNode* outNode,
										ImNodes::SlotInfo* inSlot, ImNodes::SlotInfo* outSlot) {}
	};
}