#pragma once
#include "Vector2.h"
#include "Camera.h"
#include "Model.h"
#include "Light.h"
#include "Texture.h"
#include "MShader.h"

#include <map>
#include <vector>
#include <string>

namespace moon {
	template <class T>
	struct ObjectManager {
		static std::multimap<std::string, T*> itemMap;

		static void PrintID(T* item) {
			if (typeid(item) == typeid(Shader*)) std::cout << " local id(shader): " << item->ID << std::endl;
			else if (typeid(item) == typeid(Texture*)) std::cout << " local id(texture): " << item->ID << std::endl;
			else std::cout << " id: " << item->ID << std::endl;
		}

		static void PrintAllItems() {
			auto end = itemMap.end();
			std::cout << "items(" << itemMap.count() << "):" << std << std::endl;
			for (auto it = itemMap.begin(); it != end; it++) {
				std::cout << "name: " << it->first; PrintID(it->second);
			}
		}

		static bool AddItem(T* item) {
			//itemMap.insert(std::make_pair(static_cast<ObjectBase*>(item)->name, item));
			itemMap.insert(std::pair<std::string, T*>(item->name, item));
			std::cout << "name: " << itemMap.begin()->first << " added."; PrintID(itemMap.begin()->second);
			return true;
		}

		static T* GetItem(const int &ID) {
			auto end = itemMap.end();
			for (auto it = itemMap.begin(); it != end; it++) {
				if (it->second->ID == ID) return it->second;
			}
			return NULL;
		}

		static bool GetItems(const std::string &name, 
							 typename std::multimap<std::string, T*>::iterator &lower, 
							 typename std::multimap<std::string, T*>::iterator &upper) {
								lower = itemMap.lower_bound(name);
								upper = itemMap.upper_bound(name);
								if (lower == itemMap.end()) return false;
								else return true;
							}

		static T* GetItem(const std::string &name, const int &ID = MOON_FIRSTMATCH) {
			typename std::multimap<std::string, T*>::iterator beg, end;
			if (GetItems(name, beg, end)) {
				if (ID == MOON_FIRSTMATCH) return beg->second;
				else {
					for (; beg != end; beg++) {
						if (beg->second->ID == ID) return beg->second;
					}
					return NULL;
				}
			} else return NULL;
		}

		static bool RemoveItem(const int &ID) {
			auto end = itemMap.end();
			for (auto it = itemMap.begin(); it != end; it++) {
				if (it->second->ID == ID) {
					delete it->second;
					itemMap.erase(it);
					return true;
				}
			}
			return false;
		}

		static bool RemoveItem(const T* item) {
			return RemoveItem(item->ID);
		}

		static bool RemoveItems(const std::string &name) {
			typename std::multimap<std::string, T*>::iterator beg, end;
			if (GetItems(name, beg, end)) {
				for (auto itr = beg; itr != end; ) {
					if (itr->first == name) {
						std::cout << "item: " << itr->first << "\t" << itr->second->name << " deleted." << std::endl;
						delete itr->second;
						itr = itemMap.erase(itr);
					} else itr++;
				}
			} else return false;
		}

		static bool RemoveItem(const std::string &name, const int &ID = MOON_FIRSTMATCH) {
			typename std::multimap<std::string, T*>::iterator beg, end;
			if (GetItems(name, beg, end)) {
				if (ID == MOON_FIRSTMATCH) {
					itemMap.erase(beg);
					return true;
				} else {
					for (; beg != end; beg++) {
						if (beg->second->ID == ID) {
							itemMap.erase(beg);
							return true;
						}
					}
					return false;
				}
			} else return false;
		}

		static bool Clear() {
			auto end = itemMap.end();
			for (auto itr = itemMap.begin(); itr != end; ) {
				delete itr->second;
				itr = itemMap.erase(itr);
			}
			return true;
		}
	};

	class SceneManager {
	private:
		static unsigned int objectCounter;
	public:
		// global parameters
		static Vector2 SCR_SIZE;

		static unsigned int GenUniqueID() {
			return objectCounter++;
		}
		static unsigned int GetObjectCount() {
			return objectCounter;
		}

		// scene components
		struct Clock {
			static float deltaTime;
			static float lastFrame;
		};

		struct InputManager {
			static Vector2 mousePos;
		};

		struct LightManager : ObjectManager<Light> {

		};

		struct MaterialManager : ObjectManager<Material> {

		};

		struct ShaderManager : ObjectManager<Shader> {
			static void CreateDefaultShaders() {
				AddItem(new Shader("pointShader", "point.vs", "point.fs"));
			}
		};

		struct TextureManager : ObjectManager<Texture> {

		};

		struct ModelManager : ObjectManager<Model> {

		};

		struct CameraManager : ObjectManager<Camera> {
			static Camera* sceneCamera;
		};

	};
}