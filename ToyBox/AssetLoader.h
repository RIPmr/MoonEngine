#pragma once
#include <imgui.h>
#include <imgui_internal.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>
#include <iostream>
#include <string>
#include <vector>
#include <io.h>

namespace MOON {
	struct DirNode {
		bool isFolder;
		bool hasSubFolder;
		bool isClicked;
		std::string name;
		DirNode* parent;
		DirNode* root;
		std::vector<DirNode*> childs;

		DirNode(const bool &isFolder, const std::string &name) :
				parent(NULL), root(this), isFolder(isFolder), hasSubFolder(false), isClicked(false), name(name) {}
		DirNode(const bool &isFolder, const std::string &name, DirNode* parent, DirNode* root) :
				parent(parent), isFolder(isFolder), hasSubFolder(false), isClicked(false), name(name), root(root) {
			parent->AddNode(this);
		}
		DirNode(const DirNode &node) : isFolder(isFolder), name(name), 
									   parent(parent), childs(childs),
									   hasSubFolder(false), isClicked(false) {}
		~DirNode() {
			for (auto &iter : childs) {
				delete iter;
			}
		}

		inline void RenderFolderView(float window_visible_x2, DirNode* &parent) {
			static ImVec2 button_sz(60, 45);
			static float itemSpacing = ImGui::GetStyle().ItemSpacing.x;
			int n = 0;
			for (auto &file : childs) {
				//ImGui::PushID(n);
				std::string icon = file->isFolder ? std::string(u8"\uf07b") : std::string(u8"\uf016");
				if (ImGui::ButtonEx((icon + "\n" + file->name).c_str(), button_sz, ImGuiButtonFlags_PressedOnDoubleClick)) {
					if (isFolder) {
						root->ResetFolderClickState();
						file->isClicked = true;
						parent = file;
					}
				}
				/*
				float offset = (button_sz.x - ImGui::CalcTextSize("folder").x) / 2.0f;
				ImGui::Button(ICON_FA_FOLDER, button_sz);
				ImGui::Indent(offset);
				ImGui::Text("folder");
				ImGui::Unindent(offset);
				*/
				//ImGui::PopID();

				float last_button_x2 = ImGui::GetItemRectMax().x;
				float next_button_x2 = last_button_x2 + itemSpacing + button_sz.x;
				if (n++ < childs.size() && next_button_x2 < window_visible_x2)
					ImGui::SameLine();
			}
		}

		inline void ResetFolderClickState() {			
			isClicked = false;
			for (auto &node : childs) {
				node->ResetFolderClickState();
			}
		}

		inline void ListNode(DirNode* &selectedNode) {
			if (!isFolder) return;

			ImGuiTreeNodeFlags baseFlag = ImGuiTreeNodeFlags_OpenOnArrow |
										  ImGuiTreeNodeFlags_OpenOnDoubleClick |
										  ImGuiTreeNodeFlags_SpanAvailWidth |
										  ImGuiTreeNodeFlags_DefaultOpen;

			if (!hasSubFolder) baseFlag |= ImGuiTreeNodeFlags_Leaf;
			if (isClicked) baseFlag |= ImGuiTreeNodeFlags_Selected;
			bool node_open = ImGui::TreeNodeEx(parent == NULL ? "Assets" : name.c_str(), 
				baseFlag, parent == NULL ? "Assets" : name.c_str());

			if (ImGui::IsItemClicked()) {
				// reset all click state
				if (!ImGui::GetIO().KeyCtrl) root->ResetFolderClickState();
				isClicked = !isClicked;
				if (isClicked) selectedNode = this;
			}

			if (node_open) {
				ImGui::Indent(-12.0f);
				for (auto &node : childs) {
					node->ListNode(selectedNode);
				}
				ImGui::Unindent(-12.0f);
				ImGui::TreePop();
			}
		}

		inline void PrintNode() {
			if (isFolder) {
				std::cout << "[" << name << "]" << std::endl;
				for (auto &iter : childs) {
					iter->PrintNode();
				}
			} else std::cout << name << std::endl;
		}

		inline void SetParent(DirNode* parent) { this->parent = parent; }

		inline void AddNode(DirNode* node) {
			childs.push_back(node);
		}
		inline void RemoveNode(DirNode* node);
		inline void ClearChild() { childs.clear(); }
	};

	class AssetLoader {
	public:
		static DirNode* DirTree;

		inline static void BuildDirTree(const std::string &path, const bool &loopSubFolder = true) {
			CleanUp();
			DirTree = new DirNode(true, path);
			UpdateTree(path, loopSubFolder, DirTree);
		}

		inline static void CleanUp() {
			if (DirTree != NULL) delete DirTree;
		}

		inline static void PrintDir() {
			if (DirTree != NULL) DirTree->PrintNode();
		}

		inline static void PrintDir_Direct(const std::string &path, const int &depth, const bool &loopSubFolder = true) {
			long hFile = 0;
			struct _finddata_t fileInfo;
			std::string pathName, exdName;

			// '\\*' means to traverse all types files
			// change to '\\*.jpg' to traverse JPG type files
			if ((hFile = _findfirst(pathName.assign(path).append("\\*").c_str(), &fileInfo)) == -1)
				return;

			do {
				if (strcmp(fileInfo.name, ".") && strcmp(fileInfo.name, "..")) {
					for (int i = 0; i < depth; i++) std::cout << "  ";
					if (fileInfo.attrib&_A_SUBDIR) {
						std::cout << "[" << fileInfo.name << "]" << std::endl;
						PrintDir_Direct(path + "\\" + fileInfo.name, depth + 1, loopSubFolder);
					} else
						std::cout << fileInfo.name << std::endl;
				}
			} while (_findnext(hFile, &fileInfo) == 0);

			_findclose(hFile);
		}

	private:
		inline static void UpdateTree(const std::string &path, const bool &loopSubFolder = true, DirNode* parentNode = NULL) {
			long hFile = 0;
			std::string pathName;
			struct _finddata_t fileInfo;

			if ((hFile = _findfirst(pathName.assign(path).append("\\*").c_str(), &fileInfo)) == -1)
				return;

			do {
				if (strcmp(fileInfo.name, ".") && strcmp(fileInfo.name, "..")) {
					DirNode* currNode = NULL;
					if (fileInfo.attrib&_A_SUBDIR) {
						currNode = new DirNode(true, fileInfo.name, parentNode, DirTree);
						if (parentNode != NULL) parentNode->hasSubFolder = true;
						UpdateTree(path + "\\" + fileInfo.name, loopSubFolder, currNode);
					} else {
						currNode = new DirNode(false, fileInfo.name, parentNode, DirTree);
					}
				}
			} while (_findnext(hFile, &fileInfo) == 0);

			_findclose(hFile);
		}

	};
}