#pragma once
#include <vector>
#include <string>
#include <imgui.h>

#include "ImNodesEz.h"

namespace MOON {
	// *NOTE: ID can not be 0
	enum NodeSlotTypes {
		InnerData = 1,
		Slot_Material,
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
			return input_node == other.input_node  &&
				input_slot == other.input_slot  &&
				output_node == other.output_node &&
				output_slot == other.output_slot;
		}

		bool operator!=(const Connection& other) const {
			return !operator ==(other);
		}
	};

	// A structure holding node state.
	struct MyNode {
		std::string title;
		bool selected = false;
		bool executed = false;
		float zoomFactor;
		ImVec2 pos{};

		std::vector<Connection> connections{};
		std::vector<ImNodes::Ez::SlotInfo> input_slots{};
		std::vector<ImNodes::Ez::SlotInfo> output_slots{};

		void (*content)(MyNode*, bool);

		~MyNode() = default;

		explicit MyNode(const std::string& title,
			const std::vector<ImNodes::Ez::SlotInfo>&& input_slots,
			const std::vector<ImNodes::Ez::SlotInfo>&& output_slots,
			void (*content)(MyNode*, bool)) {
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

		// TODO: Need some fix.
		void Execute() {
			executed = !executed;

			for (auto &input : input_slots) {
				if (input.parent != NULL) {
					if (((MyNode*)input.parent)->executed != executed)
						((MyNode*)input.parent)->Execute();
					input.data = input.con->data;
				}
			}

			//(*content)(this, true);
		}

		void ProcessContent(bool hideInNode) {
			for (auto &input : input_slots) {
				if (input.parent != NULL) {
					if (((MyNode*)input.parent)->executed != executed)
						((MyNode*)input.parent)->Execute();
					input.data = input.con->data;
				}
			}

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
}