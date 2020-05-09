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
//
#pragma once
#include <string>
#include <vector>
#include "../Vector3.h"
#include "../Matrix.h"
#include "../Matrix4x4.h"

#include "ImNodes.h"

namespace ImNodes {

	// *NOTE: ID can not be 0
	enum SlotType {
		In = 1, Out,
		InnerData,
		Slot_Material,
		Slot_Texture,
		Slot_Number,
		Slot_Matrix,
		Slot_Color,
	};

	/// 'union' is unsafe with class object
	struct SlotData {
		int* id;
		MOON::Vector3* col;
		MOON::Matrix* mat;

		unsigned int idSize, colSize;
		bool hasMat;

		void clear() {
			if (idSize > 0 && id != NULL) {
				delete[] id;
				id = NULL;
			}
			if (colSize > 0 && col != NULL) {
				delete[] col;
				col = NULL;
			}
			if (hasMat) {
				delete mat;
				mat = NULL;
			}
		}

		bool operator==(const SlotData& other) const {
			if (idSize == other.idSize && colSize == other.colSize &&
				id == other.id && col == other.col && hasMat == other.hasMat &&
				mat == other.mat) 
				return true;
			else return false;
		}

		// *NOTE: just tunning data, not actually equal
		SlotData& operator=(const SlotData &data) {
			if (idSize > 0 && data.idSize > 0) {
				int newIDSize = std::min(idSize, data.idSize);
				for (int i = 0; i < newIDSize; i++) {
					id[i] = data.id[i];
				}
			}
			if (colSize > 0 && data.colSize > 0) {
				int newSize = std::min(colSize, data.colSize);
				for (int i = 0; i < newSize; i++) {
					col[i] = data.col[i];
				}
			}
			if (hasMat && data.hasMat) {
				*mat = *data.mat;
			}
			return *this;
		}

		SlotData(const SlotData& data) : idSize(data.idSize), colSize(data.colSize), hasMat(data.hasMat) {
			if (idSize > 0) {
				id = new int[idSize];
				memcpy(id, data.id, idSize * sizeof(int));
			} else id = NULL;
			if (colSize > 0) {
				col = new MOON::Vector3[colSize];
				for (int i = 0; i < colSize; i++)
					col[i].setValue(data.col[i]);
			} else col = NULL;
			if (hasMat) {
				mat = new MOON::Matrix(*data.mat);
			} else mat = NULL;
		}

		SlotData(const unsigned int& itemCnt) : idSize(itemCnt), colSize(0), hasMat(false) {
			if (itemCnt > 0) {
				id = new int[itemCnt];
				memset(id, 0, itemCnt * sizeof(unsigned int));
			} else id = NULL;
			col = NULL;
			mat = NULL;
		}

		SlotData(const unsigned int& itemCnt, const unsigned int& colorCnt) :
			idSize(itemCnt), colSize(colorCnt), hasMat(false) {
			if (itemCnt > 0) {
				id = new int[itemCnt];
				memset(id, 0, itemCnt * sizeof(unsigned int));
			} else id = NULL;
			if (colorCnt > 0) col = new MOON::Vector3[colorCnt];
			else col = NULL;
			mat = NULL;
		}

		SlotData(const unsigned int& itemCnt, const unsigned int& colorCnt, const bool& matrixCnt) :
			idSize(itemCnt), colSize(colorCnt), hasMat(matrixCnt) {
			if (itemCnt > 0) {
				id = new int[itemCnt];
				memset(id, 0, itemCnt * sizeof(unsigned int));
			} else id = NULL;
			if (colorCnt > 0) col = new MOON::Vector3[colorCnt];
			else col = NULL;
			if (hasMat) mat = new MOON::Matrix(1, 0);
			else mat = NULL;
		}

		~SlotData() {
			// why 'delete' knows the size of dynamic array?
			/// 通过观察"new"调用了HeapAlloc函数，
			/// 而这一函数使用可以将内存分配情况用结构体保存起来，
			/// "delete"估计就是通过这一结构体得到数组大小的。
			/// 也就是说C++这一语法特点是借用了HeapAlloc等函数
			/// 对Windows堆内存的管理方式实现的。
			if (id != NULL) delete[] id;
			if (col != NULL) delete[] col;
			if (mat != NULL) delete mat;
		}
	};

	extern struct MyNode;
	struct SlotInfo {
		std::string title;
		/// NodeSlotTypes
		int kind;
		/// Invisible slot
		bool hideSlot;
		/// Data holder
		SlotData data;
		/// Connected parent node
		MyNode* parent;
		/// Which output slot of parent is connected 
		SlotInfo* con;
		/// Childs connected to this slot
		std::vector<MyNode*> childs;

		SlotInfo(const std::string& title, const int &kind,
			const SlotData &data, const bool &hideSlot = false) :
			data(data), title(title), kind(kind),
			hideSlot(hideSlot), con(nullptr), parent(nullptr) {}

		bool operator==(const SlotInfo& other) const {
			if (title._Equal(other.title) && kind == other.kind &&
				hideSlot == other.hideSlot && data == other.data &&
				parent == other.parent && con == other.con &&
				childs == other.childs) return true;
			else return false;
		}

		void ResetParent() {
			con = nullptr;
			parent = nullptr;
		}

		void DeleteChild(const MyNode* conIn) {
			for (auto child = childs.begin(); child != childs.end(); child++) {
				if ((*child) == conIn) {
					//std::cout << "del child: " << conIn->title << std::endl;
					childs.erase(child);
					break;
				}
			}
		}
	};

	// A structure defining a connection between two slots of two nodes.
	struct MyConnection {
		/// `id` that was passed to BeginNode() of input node.
		void* input_node = nullptr;
		/// Descriptor of input slot.
		const char* input_slot = nullptr;
		void* output_node = nullptr;
		const char* output_slot = nullptr;

		bool operator==(const MyConnection& other) const {
			return input_node == other.input_node  &&
				input_slot == other.input_slot  &&
				output_node == other.output_node &&
				output_slot == other.output_slot;
		}

		bool operator!=(const MyConnection& other) const {
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
		ImVec2 end{};

		unsigned int animCnt;
		static unsigned int animLen;

		// node editor
		void* parent = nullptr;
		// any type of data can be placed here
		void* attachment;

		std::vector<MyConnection> connections{};
		std::vector<SlotInfo> input_slots{};
		std::vector<SlotInfo> output_slots{};

		void(*content)(MyNode*, bool);
		void(*execute)(MyNode*);

		~MyNode() = default;

		explicit MyNode(const std::string& title, const std::vector<SlotInfo>&& input_slots,
			const std::vector<SlotInfo>&& output_slots, void(*content)(MyNode*, bool), 
			void(*execute)(MyNode*), void* attach = nullptr) {
			this->title = title;
			this->input_slots = input_slots;
			this->output_slots = output_slots;
			this->content = content;
			this->execute = execute;
			this->attachment = attach;
		}

		SlotInfo* GetSlot(const SlotType type, const std::string &slotName) {
			std::vector<SlotInfo>& target = (type == In) ? input_slots : output_slots;
			for (auto &slot : target) {
				if (slot.title._Equal(slotName)) {
					//std::cout << slot.title << ", " << slotName << std::endl;
					return &slot;
				}
			}
			return nullptr;
		}

		inline float GetAnim() {
			animCnt--;
			return (float)animCnt / animLen;
		}

		inline void ListName() {
			// list name
			char buf[64]; strcpy_s(buf, title.c_str());

			ImGui::Text("Name:"); ImGui::SameLine();
			ImGui::SetNextItemWidth(ImGui::GetContentRegionAvailWidth() - 30);
			ImGui::InputText("Node_Name", buf, 64);

			// if user renamed this node
			if (strcmp(buf, title.c_str())) title = buf;

			ImGui::Separator();
		}

		inline void Forward() {
			if (executed) return;

			// get data from parent
			for (auto &in : input_slots) {
				if (in.parent != nullptr) {
					if (!in.parent->executed)
						in.parent->Forward();
					in.data = in.con->data;
				}
			}

			// process data from parent
			//std::cout << "execute: " << title << std::endl;
			(*execute)(this);

			executed = true;
			animCnt = animLen;
		}

		inline void Backward() {
			executed = false;
			Forward();
			//std::cout << title << " backward" << std::endl;
			for (auto &out : output_slots) {
				if (out.childs.size() > 0) {
					for (auto &child : out.childs)
						child->Backward();
				}
			}
		}

		inline void ProcessContent(bool hideInNode) {
			// process data
			Forward();

			// then draw UI
			(*content)(this, hideInNode);
		}

		/// Deletes connection from this node.
		inline void DeleteConnection(const MyConnection& connection) {
			for (auto it = connections.begin(); it != connections.end(); ++it) {
				if (connection == *it) {
					connections.erase(it);
					break;
				}
			}
		}

	};

	// Begin rendering of node in a graph. Render node content when returns `true`.
	IMGUI_API bool BeginNodeEz(CanvasState* canvas, void* node_id, const char* title, ImVec2* pos, bool* selected);
	// Terminates current node. Should be called regardless of BeginNode() returns value.
	//IMGUI_API void EndNodeEz(CanvasState* canvas, const bool& blockMouseAction, const bool& anythingWrong);
	IMGUI_API void EndNodeEz(CanvasState* canvas, const bool& blockMouseAction, const unsigned int& anythingWrong);
	// Renders input slot region. Kind is unique value whose sign is ignored.
	/// This function must always be called after BeginNode() and before OutputSlots().
	/// When no input slots are rendered call InputSlots(nullptr, 0);
	IMGUI_API void InputSlotsEz(CanvasState* canvas, const SlotInfo* slots, int snum);
	// Renders output slot region. Kind is unique value whose sign is ignored. function call is required (not optional).
	/// This function must always be called after InputSlots() and before EndNode().
	/// When no input slots are rendered call OutputSlots(nullptr, 0);
	IMGUI_API void OutputSlotsEz(CanvasState* canvas, const SlotInfo* slots, int snum);

}
