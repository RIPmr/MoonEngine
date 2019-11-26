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
#include "../Vector3.h"
#include "../Matrix4x4.h"

#include "ImNodes.h"

namespace ImNodes {
	namespace Ez {
		/// 'union' is unsafe with class object
		struct SlotData {
			unsigned int idSize, colSize;
			unsigned int* id;
			MOON::Vector3* col;

			SlotData(const SlotData& data) : idSize(data.idSize), colSize(data.colSize) {
				if (idSize > 0) {
					id = new unsigned int[idSize];
					memcpy(id, data.id, idSize * sizeof(unsigned int));
				} else id = NULL;
				if (colSize > 0) {
					col = new MOON::Vector3[colSize];
					for (int i = 0; i < colSize; i++)
						col[i].setValue(data.col[i]);
				} else col = NULL;
			}

			SlotData(const unsigned int& itemCnt, const unsigned int& colorCnt) : 
					 idSize(itemCnt), colSize(colorCnt) {
				if (itemCnt > 0) id = new unsigned int[itemCnt];
				else id = NULL;
				if (colorCnt > 0) col = new MOON::Vector3[colorCnt];
				else col = NULL;
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
			}
		};

		struct SlotInfo {
			const char* title;
			int kind; // NodeSlotTypes
			SlotData data;
		};

		// Begin rendering of node in a graph. Render node content when returns `true`.
		IMGUI_API bool BeginNode(void* node_id, const char* title, ImVec2* pos, bool* selected);
		// Terminates current node. Should be called regardless of BeginNode() returns value.
		IMGUI_API void EndNode();
		// Renders input slot region. Kind is unique value whose sign is ignored.
		/// This function must always be called after BeginNode() and before OutputSlots().
		/// When no input slots are rendered call InputSlots(nullptr, 0);
		IMGUI_API void InputSlots(const SlotInfo* slots, int snum);
		// Renders output slot region. Kind is unique value whose sign is ignored. function call is required (not optional).
		/// This function must always be called after InputSlots() and before EndNode().
		/// When no input slots are rendered call OutputSlots(nullptr, 0);
		IMGUI_API void OutputSlots(const SlotInfo* slots, int snum);

	}

}
