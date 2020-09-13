#include <iostream>

#include "ObjectBase.h"
#include "SceneMgr.h"
#include "OperatorBase.h"

namespace MOON {
	ObjectBase::ObjectBase(const int &_id) : visible(true), selected(false) {
		if (_id == MOON_AUTOID) {
			ID = SceneManager::GenUniqueID();
		} else ID = _id;
		name = "Object_" + ID;
	}

	ObjectBase::ObjectBase(const std::string &_name, const int &_id) : name(_name), visible(true), selected(false) {
		if (_id == MOON_AUTOID) {
			ID = SceneManager::GenUniqueID();
		} else ID = _id;
	}

	void ObjectBase::Rename(const std::string &newName) {
		SceneManager::RenameItem(this, newName);
	}

	void ObjectBase::ListName() {
		// list name
		char buf[64]; strcpy(buf, name.c_str());

		ImGui::Text("Name:"); ImGui::SameLine();
		ImGui::SetNextItemWidth(ImGui::GetContentRegionAvailWidth() - 30);
		ImGui::InputText(UniquePropName("Name"), buf, 64); ImGui::SameLine();
		ImGui::Checkbox(UniquePropName("Visible"), &visible, true);

		// if user renamed this object
		if (strcmp(buf, name.c_str())) Rename(buf);
	}

	void ObjectBase::ListProperties() {
		ListName();
		ImGui::Spacing();
	}

	void MObject::OPStack::ExecuteAll() {
		if (deliver == nullptr) deliver = parent;
		if (!enable) return;
		for (auto &iter : opList) {
			deliver = iter->Execute(deliver);
		}
	}

	void MObject::OPStack::UpdateFrom(const int& id) {
		deliver = id ? opList[id - 1]->processed : parent;
		for (int i = id; i < opList.size(); i++) {
			deliver = opList[i]->Execute(deliver);
		}
	}

	void MObject::OPStack::ListStacks() {
		static auto stackName = std::string(ICON_FA_WRENCH) + " OP-Stack";
		ImGui::Checkbox(UniquePropNameFromParent("enableOPStack"), &enable, true); ImGui::SameLine();
		ImGui::Button(ICON_FA_PLUS, ImVec2(22, 22), parent->ID); ImGui::SameLine();
		if (ImGui::CollapsingHeader(stackName.c_str(), ImGuiTreeNodeFlags_DefaultOpen, parent->ID)) {
			for (int i = 0; i < opList.size(); i++) {
				if (opList[i]->content(opList[i])) {
					UpdateFrom(i);
				}
			}
		}
	}

	void MObject::OPStack::AddStack(Operator* op) {
		opList.push_back(op);
	}
	void MObject::OPStack::RemoveStack(Operator* op) {
		auto end = opList.end();
		for (auto iter = opList.begin(); iter != end; iter++) {
			if ((*iter)->name._Equal(op->name)) {
				delete (*iter);
				iter = opList.erase(iter);
				break;
			}
		}
	}
	void MObject::OPStack::ClearStack() {
		auto end = opList.end();
		for (auto iter = opList.begin(); iter != end;) {
			delete *iter;
			iter = opList.erase(iter);
		}
	}

	void MObject::ListName() {
		// list name
		char buf[64]; strcpy(buf, name.c_str());

		ImGui::Text("Name:"); ImGui::SameLine();
		ImGui::SetNextItemWidth(ImGui::GetContentRegionAvailWidth() - 60);
		ImGui::InputText(UniquePropName("Name"), buf, 64); ImGui::SameLine();
		ImGui::ColorEdit4(UniquePropName("WireColor"), (float*)&wireColor,
			ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel); ImGui::SameLine();
		ImGui::Checkbox(UniquePropName("Visible"), &visible, true);

		// if user renamed this object
		if (strcmp(buf, name.c_str())) Rename(buf);
	}

	void MObject::ListTransform() {
		// list transform
		//auto euler = transform.parent == nullptr ? transform.rotation.eulerAngles : transform.localRotation.eulerAngles;
		auto euler = transform.rotation.eulerAngles;
		auto posVec = transform.position;
		auto scaVec = transform.scale;

		static bool isLocal = true;
		ImGui::Text("Transform:");
		if (transform.parent != nullptr) {
			float width = ImGui::GetContentRegionAvailWidth(); width /= 4.0f;
			ImGui::SameLine(ImGui::GetCursorPosX() + width * 3.0f);
			SwitchButton("Local", "World", isLocal, ImVec2(width, 22));
			if (isLocal) {
				ImGui::PushStyleColor(ImGuiCol_Border, ImVec4(0.5, 0.5, 0.8, 1));
				euler = transform.localRotation.eulerAngles;
				posVec = transform.localPosition;
				scaVec = transform.localScale;
			}
		} 
		/*else if (ImGui::Button("test")) {
			Quaternion quat(Matrix4x4(transform.rotation));
			std::cout << "rot:" << quat.EulerAngle() << std::endl;
		}*/

		float pos[3] = { posVec.x, posVec.y, posVec.z };
		float scale[3] = { scaVec.x, scaVec.y, scaVec.z };
		float rotEuler[3] = { euler.x, euler.y, euler.z };

		ImGui::Indent(10.0f);
		ImGui::Text("Position"); ImGui::SameLine(80.0f);
		if (ImGui::DragFloat3(UniquePropName("Pos"), pos, 0.1f, -INFINITY, INFINITY, "%.3f", 1.0f, true)) {
			if (transform.parent != nullptr && isLocal) transform.localPosition = Vector3(pos);
			else transform.position = Vector3(pos);
		}
		ImGui::Text("Rotation"); ImGui::SameLine(80.0f);
		if (ImGui::DragFloat3(UniquePropName("Rot"), rotEuler, 0.1f, -INFINITY, INFINITY, "%.3f", 1.0f, true)) {
			Quaternion deltaQ = Quaternion(
				rotEuler[0] - euler.x,
				rotEuler[1] - euler.y,
				rotEuler[2] - euler.z
			);
			transform.Rotate(deltaQ);
			/*if (transform.parent != nullptr) transform.localRotation = Quaternion(rotEuler[0], rotEuler[1], rotEuler[2]);
			else transform.rotation = Quaternion(rotEuler[0], rotEuler[1], rotEuler[2]);*/
		}
		ImGui::Text("Scale"); ImGui::SameLine(80.0f);
		if (ImGui::DragFloat3(UniquePropName("Sca"), scale, 0.1f, -INFINITY, INFINITY, "%.3f", 1.0f, true)) {
			if (transform.parent != nullptr && isLocal) transform.localScale = Vector3(scale);
			else transform.scale = Vector3(scale);
		}
		ImGui::Unindent(10.0f);
		if (transform.parent != nullptr && isLocal) ImGui::PopStyleColor();

		//transform.set(&Vector3(pos), NULL, &Vector3(scale));
	}

	void MObject::ListProperties() {
		// list name
		ListName();
		ImGui::Separator();

		// list transform
		ListTransform();
		ImGui::Separator();

		// list operators
		opstack.ListStacks();
		ImGui::Spacing();
	}
}