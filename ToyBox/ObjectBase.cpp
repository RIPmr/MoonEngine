#include "ObjectBase.h"
#include "SceneMgr.h"

#include <iostream>

namespace moon {
	ObjectBase::ObjectBase(const int &_id) {
		if (_id == MOON_AUTOID) {
			ID = SceneManager::GenUniqueID();
		}
		name = "Object_" + ID;
	}
	ObjectBase::ObjectBase(const std::string &_name, const int &_id) : name(_name) {
		if (_id == MOON_AUTOID) {
			ID = SceneManager::GenUniqueID();
		}
	}
}