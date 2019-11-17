#include "ObjectBase.h"
#include "SceneMgr.h"

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

	void MObject::ListProperties() {
		// list name
		char* buf = (char*)name.c_str();
		ImGui::Text("Name:"); ImGui::SameLine();
		ImGui::InputText("NameInput_" + ID, buf, 64); ImGui::SameLine();
		ImGui::Checkbox("isVisible_" + ID, &visible);
		if (strcmp(buf, name.c_str())) Rename(buf);

		// list transform
		static float pos[3] = { transform.position.x, transform.position.y, transform.position.z };
		static float rotEuler[3] = { transform.rotation.eulerAngles.x, transform.rotation.eulerAngles.y, transform.rotation.eulerAngles.z };
		static float scale[3] = { transform.scale.x, transform.scale.y, transform.scale.z };

		ImGui::Text("Transform:");
		ImGui::DragFloat3("Position", pos, 0.0f, 0.0f, 1.0f);
		ImGui::DragFloat3("Rotation", rotEuler, 0.0f, 0.0f, 1.0f);
		ImGui::DragFloat3("Scale", scale, 0.0f, 0.0f, 1.0f);

		// list operators


	}
}