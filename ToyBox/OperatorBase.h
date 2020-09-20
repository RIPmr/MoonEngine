#pragma once
#include <map>
#include <string>
#include <imgui.h>

#include "ObjectBase.h"
#include "imgui/ImNodesEz.h"

namespace MOON {
#define Operators std::multimap<std::string, Operator*(*)()>

	struct Operator {
	public:
		std::string name;
		std::string type;
		//std::string path;

		bool enabled = true;
		bool opened = true;
		//bool executed = false;

		MObject* processed = nullptr;

		// if anything has been changed, return true
		bool(*content)(Operator*, MObject*);
		ImNodes::SlotData dataHolder;

		MObject* Execute(MObject* object) {
			//if (processed != nullptr) delete processed;
			processed = execute(this, object);
			return processed;
		}

		explicit Operator(const std::string& type, const ImNodes::SlotData &data,
			bool(*content)(Operator*, MObject*), MObject*(*execute)(Operator*, MObject*)) :
			dataHolder(data) {
			this->name = this->type = type;
			this->content = content;
			this->execute = execute;
		}

		~Operator() {
			if (processed != nullptr) {
				delete processed;
				processed = nullptr;
			}
		}

		void ListName(const unsigned int& ID) {
			// list name
			char buf[64]; strcpy(buf, name.c_str());

			//ImGui::Text("Name:"); ImGui::SameLine();
			ImGui::SetNextItemWidth(ImGui::GetContentRegionAvailWidth() - 20);
			ImGui::InputText(("Name_" + std::to_string(ID)).c_str(), buf, 64);

			// if user renamed this object
			if (strcmp(buf, name.c_str())) name = std::string(buf);
		}

	private:
		MObject*(*execute)(Operator*, MObject*);
	};
}