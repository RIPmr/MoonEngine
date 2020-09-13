#pragma once
#include <map>
#include <vector>
#include <string>
#include <imgui.h>

#include "MoonEnums.h"
#include "FuzzyMatch.h"
#include "Vector3.h"
#include "Vector4.h"
#include "Color.h"
#include "Matrix4x4.h"
#include "ImNodesEz.h"
#include "Icons.h"

namespace MOON {
	#define Word						std::pair<unsigned int, void*>
	#define Nodes						std::multimap<std::string, ImNodes::MyNode*(*)()>
	#define Verbs						std::vector<Verb*(*)()>

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

	struct Verb {
		std::string str;
		void(*execute)(Word&, Word&, void*);

		~Verb() = default;

		explicit Verb(const std::string& str, void(*execute)(Word&, Word&, void*)) {
			this->str = str;
			this->execute = execute;
		}
	};

	class NodeEditor {
	protected:
		enum queryType {
			node,
			verb
		};

		struct Searcher {
			bool enableFlowSense;
			bool resetVerbViewScroll;
			bool enableSearcher;
			std::string searchWord;
			ImVec2 mousePos;
			int selection;
			std::vector<ImNodes::MyNode*> resNodes;
			std::vector<Verb*> resVerbs;
			std::vector<Word> query;
			ImVec2 childSize;
			NodeEditor* parent;

			Searcher(NodeEditor* parent) {
				this->parent = parent;
				enableFlowSense = true;
				enableSearcher = false;
				resetVerbViewScroll = false;
				childSize.x = 170.0f;
				searchWord = "";
				selection = 0;
			}

			~Searcher() {
				if (resNodes.size())
					for (auto &node : resNodes) {
						delete node;
					}
				if (resVerbs.size())
					for (auto &verb : resVerbs) {
						delete verb;
					}
				if (query.size())
					for (auto &q : query) {
						if (q.first == node) delete (ImNodes::MyNode*)q.second;
						else if (q.first == verb) delete (Verb*)q.second;
					}
			}

			void Draw();
		};

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
		bool enableSearcher = false;

		Verbs* available_verbs = NULL;
		Nodes* available_nodes = NULL;
		std::vector<ImNodes::MyNode*> nodes;

		Searcher searcher;

		NodeEditor() : searcher(this) {
			DefineDefaultVerbs();
		}

		virtual ~NodeEditor() {
			CleanUp();
			if (available_nodes != NULL) delete available_nodes;
			if (available_verbs != NULL) delete available_verbs;
		}

		void DefineDefaultVerbs();

		std::vector<ImNodes::MyNode*> SearchNodes_Fuzzy(const std::string &typeName) {
			std::map<int, ImNodes::MyNode*> fuzzyRes;

			for (auto lower = available_nodes->begin(); lower != available_nodes->end(); lower++) {
				int score = -INFINITY_INT;
				auto node = (*lower).second();
				node->type = lower->first;
				std::string name = node->title;
				MatchTool::fuzzy_match(typeName.c_str(), name.c_str(), score);
				//std::cout << "name: " << node->title << " score: " << score << std::endl;
				if (score > 0) fuzzyRes.insert(std::pair<int, ImNodes::MyNode*>(score, node));
				else delete node;
			}

			std::vector<ImNodes::MyNode*> out;
			for (auto it = fuzzyRes.rbegin(); it != fuzzyRes.rend(); it++) {
				out.push_back((*it).second);
			}

			return out;
		}

		std::vector<Verb*> SearchVerbs_Fuzzy(const std::string &typeName) {
			std::map<int, Verb*> fuzzyRes;

			for (auto lower = available_verbs->begin(); lower != available_verbs->end(); lower++) {
				int score = -INFINITY_INT;
				auto verb = (*lower)();
				MatchTool::fuzzy_match(typeName.c_str(), verb->str.c_str(), score);
				if (score > 0) fuzzyRes.insert(std::pair<int, Verb*>(score, verb));
				else delete verb;
			}

			std::vector<Verb*> out;
			for (auto it = fuzzyRes.rbegin(); it != fuzzyRes.rend(); it++) {
				out.push_back((*it).second);
			}

			return out;
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

		void Instantiate(ImNodes::MyNode* node, const ImVec2& nodePos);
		void ListSlots(const std::string &typeName);
		void Draw();

		ImNodes::MyConnection* CreateConnection(ImNodes::MyNode* inNode, ImNodes::MyNode* outNode,
												const char* input_slot, const char* output_slot) {
			ImNodes::MyConnection new_connection;

			new_connection.input_node = inNode;
			new_connection.output_node = outNode;
			new_connection.input_slot = input_slot;
			new_connection.output_slot = output_slot;

			inNode->connections.push_back(new_connection);
			outNode->connections.push_back(new_connection);

			/// store connection ptr to input slot and output slot
			auto islot = inNode->GetSlot(ImNodes::In, new_connection.input_slot);
			auto oslot = outNode->GetSlot(ImNodes::Out, new_connection.output_slot);
			islot->con = oslot;
			islot->parent = outNode;
			oslot->childs.push_back(inNode);

			return &outNode->connections.back();
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
		virtual void OnCreateNode(ImNodes::MyNode* node) {}
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