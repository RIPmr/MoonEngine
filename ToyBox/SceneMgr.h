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
		static bool sizeFlag;
		static std::multimap<std::string, T*> itemMap;

		static int CountItem() {
			return itemMap.size();
		}

		static void PrintID(T* item) {
			if (typeid(item) == typeid(Shader*)) std::cout << " local(shader)";
			else if (typeid(item) == typeid(Texture*)) std::cout << " local(texture)";
			else {
				std::cout << " id: " << item->ID << "" << std::endl;
				return;
			}
			std::cout << " id: [" << item->ID << "]" << std::endl;
		}

		static void ListItems(int &loopID) {
			auto iter = itemMap.begin();
			for (int i = 0; i < itemMap.size(); i++, iter++, loopID++) {
				if (ImGui::Selectable(iter->second->name.c_str(), MOON_InputManager::selection[loopID], ImGuiSelectableFlags_SpanAllColumns)) {
					if (!MainUI::io->KeyCtrl) memset(MOON_InputManager::selection, 0, SceneManager::TotalObjectNum() * sizeof(bool));
					MOON_InputManager::selection[loopID] ^= 1;
				}
			}
		}

		static void ListID() {
			for (auto &iter : itemMap) {
				char label[32];
				if (typeid(itemMap.begin()->second) == typeid(Texture*))
					sprintf_s(label, "tex-%d", iter.second->ID);
				else if (typeid(itemMap.begin()->second) == typeid(Shader*))
					sprintf_s(label, "sha-%d", iter.second->ID);
				else
					sprintf_s(label, "%d", iter.second->ID);
				ImGui::Text(label);
			}
		}

		static void PrintAllItems() {
			if (itemMap.count() < 1) {
				std::cout << "empty type!" << std::endl;
				return;
			}

			auto type = typeid(itemMap.begin()->second);
			if (type == typeid(Shader*)) std::cout << " Shaders in scene: " << std::endl;
			else if (type == typeid(Texture*)) std::cout << " Textures in scene: " << std::endl;
			else if (type == typeid(Material*)) std::cout << " Materials in scene: " << std::endl;
			else if (type == typeid(Light*)) std::cout << " Lights in scene: " << std::endl;
			else if (type == typeid(Model*)) std::cout << " Models in scene: " << std::endl;
			else if (type == typeid(Camera*)) std::cout << " Cameras in scene: " << std::endl;

			auto end = itemMap.end();
			std::cout << "items(" << itemMap.count() << "):" << std << std::endl;
			for (auto it = itemMap.begin(); it != end; it++) {
				std::cout << "- name: " << it->first; PrintID(it->second);
			}
		}

		static bool AddItem(T* item) {
			std::cout << std::endl;
			if (typeid(item) == typeid(Shader*))
				std::cout << "------------------------------------------------ new shader added ------------------------------------------------" << std::endl;
			else if (typeid(item) == typeid(Texture*))
				std::cout << "------------------------------------------------ new texture added -----------------------------------------------" << std::endl;
			else if (typeid(item) == typeid(Material*))
				std::cout << "------------------------------------------------ new Material added ----------------------------------------------" << std::endl;
			else if (typeid(item) == typeid(Light*))
				std::cout << "-------------------------------------------------- new Light added -----------------------------------------------" << std::endl;
			else if (typeid(item) == typeid(Model*))
				std::cout << "-------------------------------------------------- new Model added -----------------------------------------------" << std::endl;
			else if (typeid(item) == typeid(Camera*))
				std::cout << "------------------------------------------------- new Camera added -----------------------------------------------" << std::endl;
			else 
				std::cout << "------------------------------------------ new object added [unknown type] ---------------------------------------" << std::endl;
			//itemMap.insert(std::make_pair(static_cast<ObjectBase*>(item)->name, item));
			itemMap.insert(std::pair<std::string, T*>(item->name, item));
			auto iter = itemMap.end(); iter--;
			std::cout << "- name: \'" << iter->first << "\' added."; PrintID(iter->second);

			sizeFlag = true;
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
				else for (; beg != end; beg++)
					if (beg->second->ID == ID) return beg->second;
			}
			return NULL;
		}

		static bool IsUnique(const std::string &name, const int &ID = MOON_FIRSTMATCH) {
			if (GetItem(name, ID) != NULL) return false;
			else return true;
		}

		static bool DeleteItem(const int &ID) {
			auto end = itemMap.end();
			for (auto it = itemMap.begin(); it != end; it++) {
				if (it->second->ID == ID) {
					delete it->second;
					itemMap.erase(it);

					sizeFlag = true;
					return true;
				}
			}
			return false;
		}

		static bool DeleteItem(const std::string &name, const int &ID = MOON_FIRSTMATCH) {
			typename std::multimap<std::string, T*>::iterator beg, end;
			if (GetItems(name, beg, end)) {
				if (ID == MOON_FIRSTMATCH) {
					delete beg->second;
					itemMap.erase(beg);

					sizeFlag = true;
					return true;
				} else for (; beg != end; beg++)
					if (beg->second->ID == ID) {
						delete beg->second;
						itemMap.erase(beg);

						sizeFlag = true;
						return true;
					}
			}
			return false;
		}

		static bool DeleteItem(const T* item) {
			return DeleteItem(item->name, item->ID);
		}

		static bool DeleteItems(const std::string &name) {
			typename std::multimap<std::string, T*>::iterator beg, end;
			if (GetItems(name, beg, end)) {
				for (auto itr = beg; itr != end; ) {
					if (itr->first == name) {
						std::cout << "item: " << itr->first << "\t" << itr->second->name << " deleted." << std::endl;
						delete itr->second;
						itr = itemMap.erase(itr);

						sizeFlag = true;
					} else itr++;
				}
			} else return false;
		}

		static T* RemoveItem(const int &ID) {
			T* res = NULL;
			auto end = itemMap.end();
			for (auto it = itemMap.begin(); it != end; it++) {
				if (it->second->ID == ID) {
					res = it->second;
					itemMap.erase(it);

					sizeFlag = true;
					return res;
				}
			}
			return res;
		}

		static T* RemoveItem(const std::string &name, const int &ID = MOON_FIRSTMATCH) {
			T* res = NULL;
			typename std::multimap<std::string, T*>::iterator beg, end;
			if (GetItems(name, beg, end)) {
				if (ID == MOON_FIRSTMATCH) {
					res = beg->second;
					itemMap.erase(beg);

					sizeFlag = true;
					return res;
				} else for (; beg != end; beg++)
					if (beg->second->ID == ID) {
						res = beg->second;
						itemMap.erase(beg);

						sizeFlag = true;
						return res;
					}
			}
			return res;
		}

		static T* RemoveItem(const T* item) {
			return RemoveItem(item->name, item->ID);
		}

		static bool RemoveItems(const std::string &name,
								typename std::multimap<std::string, T*>::iterator &lower,
								typename std::multimap<std::string, T*>::iterator &upper) {
			if (GetItems(name, lower, upper)) {
				for (auto itr = lower; itr != upper; ) {
					if (itr->first == name) {
						itr = itemMap.erase(itr);

						sizeFlag = true;
					} else itr++;
				}
			} else return false;
		}

		static bool RenameItem(const int &ID, const std::string newName) {
			T* item = RemoveItem(ID);
			item->name = newName;
			AddItem(item);
			return true;
		}

		static bool RenameItem(T* item, const std::string newName) {
			RemoveItem(item->name, item->ID);
			item->name = newName;
			AddItem(item);
			return true;
		}

		// all items will be deleted
		static bool Clear() {
			auto end = itemMap.end();
			for (auto itr = itemMap.begin(); itr != end; ) {
				delete itr->second;
				itr = itemMap.erase(itr);
			}

			sizeFlag = true;
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
		static int TotalObjectNum() {
			int count = ModelManager::CountItem();
			count += TextureManager::CountItem();
			count += LightManager::CountItem();
			count += CameraManager::CountItem();
			count += ShaderManager::CountItem();
			count += MaterialManager::CountItem();

			return count;
		}

		// scene components
		struct Clock {
			static float deltaTime;
			static float lastFrame;
		};

		struct InputManager {
			static Vector2 mousePos;
			static Vector2 mouseOffset;
			static Vector2 mouseScrollOffset;
			static int mouseButton;
			static int mouseAction;
			static int mouseMods;
			static bool isHoverUI;
			static bool mouse_left_hold;
			static bool mouse_middle_hold;
			static bool mouse_right_hold;
			static bool* selection;

			static void UpdateSelectionState() {
				if (ModelManager::sizeFlag || TextureManager::sizeFlag || LightManager::sizeFlag ||
					MaterialManager::sizeFlag || CameraManager::sizeFlag || ShaderManager::sizeFlag) {

					if(selection != NULL) free(selection);
					selection = (bool*)malloc(TotalObjectNum() * sizeof(bool));
					memset(selection, 0, TotalObjectNum() * sizeof(bool));

					ModelManager::sizeFlag = false;
					TextureManager::sizeFlag = false;
					LightManager::sizeFlag = false;
					MaterialManager::sizeFlag = false;
					CameraManager::sizeFlag = false;
					ShaderManager::sizeFlag = false;
				}
			}

			static void Clear() {
				if (selection != NULL) free(selection);
			}
		};

		struct LightManager : ObjectManager<Light> {

		};

		struct MaterialManager : ObjectManager<Material> {
			static Material* defaultMat;

			static void CreateDefaultMats() {
				defaultMat = SceneManager::MaterialManager::CreateMaterial("MoonMtl", "default");
			}

			static Material* CreateMaterial(const std::string &type, const std::string &name) {
				if (type._Equal("MoonMtl")) {
					Material* newMat = new MoonMtl(name);
					AddItem(newMat);
					return newMat;
				} else {
					
				}
			}
		};

		struct ShaderManager : ObjectManager<Shader> {
			static void LoadDefaultShaders() {
				AddItem(new Shader("ConstColor", "ConstColor.vs", "ConstColor.fs"));
				//AddItem(new Shader("SimplePhong", "SimplePhong.vs", "SimplePhong.fs"));
			}

			static Shader* CreateShader(const std::string &name, const char *vs, const char *fs) {
				Shader* res = GetItem(name);
				if (res == NULL) {
					res = new Shader(name, vs, fs);
					AddItem(res);
				}
				return res;
			}

			static bool Clear() {
				auto end = itemMap.end();
				for (auto itr = itemMap.begin(); itr != end; ) {
					glDeleteShader(itr->second->ID);
					delete itr->second;
					itr = itemMap.erase(itr);
				}
				return true;
			}
		};

		struct TextureManager : ObjectManager<Texture> {
			// load resources
			static bool LoadImagesForUI() {
				AddItem(new Texture("./Resources/Icon.jpg", "moon_icon"));
				AddItem(new Texture("./Resources/logo.png", "moon_logo"));
				return true;
			}

			// TODO
			static Texture* CreateTexture();

			static Texture* LoadTexture(const std::string &path, bool gamma = false) {
				Texture* tex = new Texture(path);

				AddItem(tex);
				return tex;
			}

			static bool Clear() {
				auto end = itemMap.end();
				for (auto itr = itemMap.begin(); itr != end; ) {
					glDeleteTextures(1, &itr->second->ID);
					delete itr->second;
					itr = itemMap.erase(itr);
				}
				return true;
			}
		};

		struct ModelManager : ObjectManager<Model> {
			static bool Hit(const Ray &r, float tmin, float tmax, HitRecord &rec) {
				HitRecord tempRec;
				bool hitAnything = false;
				double closestSoFar = tmax;

				for (auto &iter : itemMap) {
					if (iter.second->Hit(r, tmin, closestSoFar, tempRec)) {
						hitAnything = true;
						closestSoFar = tempRec.t;
						rec = tempRec;
					}
				}

				return hitAnything;
			}

			static Model* CreateModel() {
			
			}

			static Model* LoadModel(const std::string &path, const std::string &name = "FILENAME") {
				Model* newModel = new Model(path, name);
				AddItem(newModel);

				return newModel;
			}
		};

		struct CameraManager : ObjectManager<Camera> {
			static Camera* sceneCamera;

			static Camera* CreateCamera() {
				Camera* newCam = new Camera();
				AddItem(newCam);

				return newCam;
			}
			static Camera* CreateCamera(const std::string name) {
				Camera* newCam = new Camera(name);
				AddItem(newCam);

				return newCam;
			}

			static bool LoadSceneCamera() {
				sceneCamera = new Camera("SceneCamera", Vector3(0.0f, 5.0f, 20.0f), 0.0f, MOON_UNSPECIFIEDID);
				Renderer::targetCamera = sceneCamera;
				return true;
			}
		};

	};
}