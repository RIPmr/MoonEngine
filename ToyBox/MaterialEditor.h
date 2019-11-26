#pragma once
#include <vector>
#include <map>
#include <string>
#include <imgui.h>

#include "ImNodesEz.h"
#include "Vector3.h"
#include "Vector4.h"
#include "Color.h"
#include "Matrix4x4.h"

#define DataSizeDef ImNodes::Ez::SlotData
#define IMGUI_DEFINE_MATH_OPERATORS

namespace MOON {
	// *NOTE: ID can not be 0
	enum NodeSlotTypes {
		Slot_Material = 1,
		Slot_Texture,
		Slot_Number,
		Slot_Matrix,
		Slot_Color,
	};

	// A structure defining a connection between two slots of two nodes.
	struct Connection {
		/// `id` that was passed to BeginNode() of input node.
		void* input_node = nullptr;
		/// Descriptor of input slot.
		const char* input_slot = nullptr;
		void* output_node = nullptr;
		const char* output_slot = nullptr;

		bool operator==(const Connection& other) const {
			return input_node  == other.input_node  &&
				   input_slot  == other.input_slot  &&
				   output_node == other.output_node &&
				   output_slot == other.output_slot ;
		}

		bool operator!=(const Connection& other) const {
			return !operator ==(other);
		}
	};

	// A structure holding node state.
	struct MyNode {
		std::string title;
		bool selected = false;
		ImVec2 pos{};

		std::vector<Connection> connections{};
		std::vector<ImNodes::Ez::SlotInfo> input_slots{};
		std::vector<ImNodes::Ez::SlotInfo> output_slots{};

		void(*content)(MyNode*, bool);

		~MyNode() = default;

		explicit MyNode(const std::string& title,
			const std::vector<ImNodes::Ez::SlotInfo>&& input_slots,
			const std::vector<ImNodes::Ez::SlotInfo>&& output_slots,
			void(*content)(MyNode*, bool)) {
			this->title = title;
			this->input_slots = input_slots;
			this->output_slots = output_slots;
			this->content = content;
		}

		void ListName() {
			// list name
			char buf[64]; strcpy(buf, title.c_str());

			ImGui::Text("Name:"); ImGui::SameLine();
			ImGui::SetNextItemWidth(ImGui::GetContentRegionAvailWidth() - 30);
			ImGui::InputText("Node_Name", buf, 64);

			// if user renamed this node
			if (strcmp(buf, title.c_str())) title = buf;

			ImGui::Separator();
		}

		void ProcessContent(bool hideInNode) {
			(*content)(this, hideInNode);
		}

		/// Deletes connection from this node.
		void DeleteConnection(const Connection& connection) {
			for (auto it = connections.begin(); it != connections.end(); ++it) {
				if (connection == *it) {
					connections.erase(it);
					break;
				}
			}
		}

	};

	struct MaterialEditor {
		bool anythingSelected;
		int selectedID;
		MyNode* selectedNode;

		std::multimap<std::string, MyNode*(*)()> available_nodes{
			// Numeric nodes -----------------------------------------------------------------
			{"Numeric", []() -> MyNode* { return new MyNode("Color", {
				/// Input slots
			}, {
				/// Output slots
				{ "Color", Slot_Color, DataSizeDef(0, 1) }
			}, 
				/// content renderer
				[](MyNode* node, bool hideInNode) {
					if (!hideInNode) {
						node->ListName();
						ImGui::Text("Output Color: ");
						ImGui::ColorEdit3("Color", (float*)&node->output_slots[0].data.col[0],
							ImGuiColorEditFlags_NoLabel);
					} else {
						ImGui::ColorEdit3("Color", (float*)&node->output_slots[0].data.col[0],
							ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel);
					}
				}
			); } },

			{"Numeric", []() -> MyNode* { return new MyNode("Bitmap", {
				/// Input slots
			}, {
				/// Output slots
				{ "Texture", Slot_Texture, DataSizeDef(1, 0) }
			}, 
				/// content renderer
				[](MyNode* node, bool hideInNode) {
					if (!hideInNode) node->ListName();
				}
			); }},

			// Operators ---------------------------------------------------------------------
			{"Operator", []() -> MyNode* { return new MyNode("ColorMixer", {
				/// Input slots
				{ "Color_1", Slot_Color, DataSizeDef(0, 1) },
				{ "Color_2", Slot_Color, DataSizeDef(0, 1) }
			}, {
				/// Output slots
				{ "Color",	 Slot_Color, DataSizeDef(0, 1) }
			},
				/// content renderer
				[](MyNode* node, bool hideInNode) {
					if (!hideInNode) node->ListName();
				}
			); } },

			{ "Operator", []() -> MyNode* { return new MyNode("ColtoTex", {
				/// Input slots
				{ "Color",	 Slot_Color, DataSizeDef(0, 1) }
			}, {
				/// Output slots
				{ "Texture", Slot_Texture, DataSizeDef(1, 0) }
			},
				/// content renderer
				[](MyNode* node, bool hideInNode) {
					if (!hideInNode) node->ListName();
				}
			); } },

			{"Operator", []() -> MyNode* { return new MyNode("TexAdjustor", {
				/// Input slots
				{ "Texture", Slot_Texture, DataSizeDef(1, 0) }
			}, {
				/// Output slots
				{ "Texture", Slot_Texture, DataSizeDef(1, 0) }
			},
				/// content renderer
				[](MyNode* node, bool hideInNode) {
					if (!hideInNode) node->ListName();
				}
			); } },

			// Materials ---------------------------------------------------------------------
			{"Material", []() -> MyNode* { return new MyNode("MoonMtl", {
				/// Input slots
				{ "Ambient",	Slot_Texture, DataSizeDef(1, 0) },
				{ "Diffuse",	Slot_Texture, DataSizeDef(1, 0) },
				{ "Specular",	Slot_Texture, DataSizeDef(1, 0) },
				{ "SpecExpo",	Slot_Texture, DataSizeDef(1, 0) },
				{ "OptiDens",	Slot_Texture, DataSizeDef(1, 0) },
				{ "Dissolve",	Slot_Texture, DataSizeDef(1, 0) },
				{ "Illumina",	Slot_Texture, DataSizeDef(1, 0) }
			}, {
				/// Output slots
				{ "MoonMtl",	Slot_Material, DataSizeDef(1, 0) }
			},
				/// content renderer
				[](MyNode* node, bool hideInNode) {
					node->output_slots[0].data.id[0] = 5;
					if (!hideInNode)
						MOON_ObjectList[node->output_slots[0].data.id[0]]->ListProperties();
				}
			); } }

		};
		std::vector<MyNode*> nodes;

		// TODO
		void ClearEditor() {
			std::cout << "node editor cleared." << std::endl;
		}

		void ListSlots(const std::string &typeName) {
			auto lower = available_nodes.lower_bound(typeName);
			auto upper = available_nodes.upper_bound(typeName);

			for (; lower != upper; lower++) {
				if (ImGui::MenuItem((*lower).second()->title.c_str())) {
					nodes.push_back((*lower).second());
					ImNodes::AutoPositionNode(nodes.back());
				}
			}
		}

		void CleanUp() {
			for (auto it = nodes.begin(); it != nodes.end(); it++) {
				delete (*it);
			}
			for (auto it = available_nodes.begin(); it != available_nodes.end(); it++) {
				delete (*it).second();
			}
		}

		void Draw() {
			// Canvas must be created after ImGui initializes, because constructor accesses ImGui style to configure default colors.
			static ImNodes::CanvasState canvas{};
			const ImGuiStyle& style = ImGui::GetStyle();

			ImNodes::BeginCanvas(&canvas);

			// restore props
			selectedID = -1;
			selectedNode = NULL;
			anythingSelected = false;

			for (auto it = nodes.begin(); it != nodes.end();) {
				MyNode* node = *it;

				if (node->selected) {
					anythingSelected = true;
					selectedNode = node;
					if (node->output_slots[0].data.id != NULL)
						selectedID = node->output_slots[0].data.id[0];
				}

				// Start rendering node
				if (ImNodes::Ez::BeginNode(node, node->title.c_str(), &node->pos, &node->selected)) {
					// Render input nodes first (order is important)
					ImNodes::Ez::InputSlots(node->input_slots.data(), node->input_slots.size());

					// node content go here
					node->ProcessContent(true);

					// Render output nodes first (order is important)
					ImNodes::Ez::OutputSlots(node->output_slots.data(), node->output_slots.size());

					// Store new connections when they are created
					Connection new_connection;
					if (ImNodes::GetNewConnection(&new_connection.input_node, &new_connection.input_slot,
						&new_connection.output_node, &new_connection.output_slot)) {
						((MyNode*)new_connection.input_node)->connections.push_back(new_connection);
						((MyNode*)new_connection.output_node)->connections.push_back(new_connection);
					}

					// Render output connections of this node
					for (const Connection& connection : node->connections) {
						// Node contains all it's connections (both from output and to input slots). This means that multiple
						// nodes will have same connection. We render only output connections and ensure that each connection
						// will be rendered once.
						if (connection.output_node != node)
							continue;

						if (!ImNodes::Connection(connection.input_node, connection.input_slot, connection.output_node,
							connection.output_slot)) {
							// Remove deleted connections
							((MyNode*)connection.input_node)->DeleteConnection(connection);
							((MyNode*)connection.output_node)->DeleteConnection(connection);
						}
					}
				}
				// Node rendering is done. This call will render node background based on size of content inside node.
				ImNodes::Ez::EndNode();

				if (node->selected && ImGui::IsKeyPressedMap(ImGuiKey_Delete)) {
					// Deletion order is critical: first we delete connections to us
					for (auto& connection : node->connections) {
						if (connection.output_node == node) {
							((MyNode*)connection.input_node)->DeleteConnection(connection);
						} else {
							((MyNode*)connection.output_node)->DeleteConnection(connection);
						}
					}
					// Then we delete our own connections, so we don't corrupt the list
					node->connections.clear();

					delete node;
					it = nodes.erase(it);
				} else ++it;
			}

			const ImGuiIO& io = ImGui::GetIO();
			if (ImGui::IsMouseReleased(1) && ImGui::IsWindowHovered() && !ImGui::IsMouseDragging(1)) {
				ImGui::FocusWindow(ImGui::GetCurrentWindow());
				ImGui::OpenPopup("NodesContextMenu");
			}

			ImGui::SetWindowFontScale(1.f);
			if (ImGui::BeginPopup("NodesContextMenu")) {
				// Numerics ----------------------------------
				if (ImGui::BeginMenu("Numerics")) {
					ListSlots("Numeric");
					ImGui::EndMenu();
				}

				// Operators ---------------------------------
				if (ImGui::BeginMenu("Operators")) {
					ListSlots("Operator");
					ImGui::EndMenu();
				}

				ImGui::Separator();
				// Mtls --------------------------------------
				ListSlots("Material");

				ImGui::Separator();
				// Other funcs -------------------------------
				if (anythingSelected) {
					if (ImGui::MenuItem("Delete")) {}
					if (ImGui::MenuItem("Copy")) {}
					if (ImGui::MenuItem("Paste")) {}
				}
				//if (ImGui::MenuItem("Reset Zoom")) canvas.zoom = 1;

				//if (ImGui::IsAnyMouseDown() && !ImGui::IsWindowHovered()) ImGui::CloseCurrentPopup();
				ImGui::EndPopup();
			}

			ImNodes::EndCanvas();

		}
	};
}