#include "ObjectBase.h"
#include "SceneMgr.h"
#include "OperatorBase.h"

#include <iostream>

namespace moon {
	ObjectBase::ObjectBase(const int &_id) : visible(true) {
		if (_id == MOON_AUTOID) {
			ID = SceneManager::GenUniqueID();
		} else ID = _id;
		name = "Object_" + ID;
	}

	ObjectBase::ObjectBase(const std::string &_name, const int &_id) : name(_name), visible(true) {
		if (_id == MOON_AUTOID) {
			ID = SceneManager::GenUniqueID();
		} else ID = _id;
	}

	void ObjectBase::Rename(const std::string &newName) {
		if (typeid(this) == typeid(Shader*)) SceneManager::ShaderManager::RenameItem(this, newName);
		else if (typeid(this) == typeid(Texture*)) SceneManager::TextureManager::RenameItem(this, newName);
		else if (typeid(this) == typeid(Material*)) SceneManager::MaterialManager::RenameItem(this, newName);
		else if (typeid(this) == typeid(Light*)) SceneManager::LightManager::RenameItem(this, newName);
		else if (typeid(this) == typeid(Model*)) SceneManager::ModelManager::RenameItem(this, newName);
		else if (typeid(this) == typeid(Camera*)) SceneManager::CameraManager::RenameItem(this, newName);
		else std::cout << "Unknown type, rename failed!" << std::endl;
	}

	void ObjectBase::ListProperties() {
		// list name
		char* buf = (char*)name.c_str();
		ImGui::Text("Name:"); ImGui::SameLine();
		ImGui::InputText("NameInput_" + ID, buf, 64); ImGui::SameLine();
		ImGui::Checkbox("isVisible_" + ID, &visible);
		if (strcmp(buf, name.c_str())) Rename(buf);
	}

	void MObject::OPStack::ExecuteAll() const {
		for (auto &iter : opList) {
			iter->Execute();
		}
	}
	void MObject::OPStack::ListStack() const {
		for (auto &iter : opList) {
			iter->ListProperties();
		}
	}

	void MObject::OPStack::AddStack(Operator* op) {
		opList.push_back(op);
	}
	void MObject::OPStack::RemoveStack(Operator* op) {
		auto end = opList.end();
		for (auto iter = opList.begin(); iter != end; iter++) {
			if ((*iter)->name._Equal(op->name)) {
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

	void MObject::ListProperties() {
		// list name
		char* buf = (char*)name.c_str();
		ImGui::Text("Name:"); ImGui::SameLine();
		ImGui::InputText("NameInput_" + ID, buf, 64); ImGui::SameLine();
		ImGui::Checkbox("isVisible_" + ID, &visible);
		if (strcmp(buf, name.c_str())) {
			//std::cout << "new name " << buf << std::endl;
			Rename(buf);
		}

		ImGui::Separator();

		// list transform
		Vector3 &euler = transform.rotation.eulerAngles;
		float pos[3] = { transform.position.x, transform.position.y, transform.position.z };
		float rotEuler[3] = { euler.x, euler.y, euler.z };
		float scale[3] = { transform.scale.x, transform.scale.y, transform.scale.z };

		ImGui::Text("Transform:");
		ImGui::DragFloat3("Position", pos, 0.1f, -INFINITY, INFINITY);
		ImGui::DragFloat3("Rotation", rotEuler, 0.1f, -INFINITY, INFINITY);
		ImGui::DragFloat3("Scale", scale, 0.1f, -INFINITY, INFINITY);

		Quaternion deltaQ = Quaternion(rotEuler[0] - euler.x,
									   rotEuler[1] - euler.y,
									   rotEuler[2] - euler.z);
		// world
		transform.rotation = deltaQ * transform.rotation;
		// local
		//transform.rotation *= deltaQ;
		transform.set(&Vector3(pos), NULL, &Vector3(scale));

		// list operators
		

	}
}