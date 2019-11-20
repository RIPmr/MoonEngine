#pragma once
#define MOON_WndSize SceneManager::SCR_SIZE
#define MOON_OutputSize Renderer::OUTPUT_SIZE
#define MOON_OutputTexID Renderer::outputTexID
#define MOON_CountObject SceneManager::TotalObjectNum()
#define MOON_SceneCamera SceneManager::CameraManager::sceneCamera
#define MOON_CurrentCamera SceneManager::CameraManager::currentCamera
#define MOON_MousePos SceneManager::InputManager::mousePos
#define MOON_Clock SceneManager::Clock

#define MOON_ObjectList SceneManager::objectList
#define MOON_LightManager SceneManager::LightManager
#define MOON_MaterialManager SceneManager::MaterialManager
#define MOON_ShaderManager SceneManager::ShaderManager
#define MOON_TextureManager SceneManager::TextureManager
#define MOON_ModelManager SceneManager::ModelManager
#define MOON_CameraManager SceneManager::CameraManager
#define MOON_InputManager SceneManager::InputManager

#define STB_IMAGE_IMPLEMENTATION
#define HAVE_STRUCT_TIMESPEC
#pragma comment(lib, "pthreadVC2.lib")

//#include <windows.h>
//#include <GL/gl.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <pthread.h>
#include <algorithm>
#include <iostream>
#include <chrono>
#include <thread>
#include <random>
#include <map>

#include "SceneMgr.h"
#include "Vector2.h"
#include "Vector3.h"
#include "Quaternion.h"
#include "Matrix4x4.h"
#include "MathUtils.h"
#include "Utility.h"
#include "Renderer.h"
#include "UIController.h"
#include "OperatorBase.h"
#include "ObjectBase.h"
#include "MShader.h"
#include "Camera.h"
#include "MatSphere.h"
#include "Model.h"
#include "Light.h"
#include "OBJLoader.h"

using namespace moon;

// function declaration
GLFWwindow* InitWnd();
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void DrawGround(const float &space, const int &gridCnt, const Shader* groundShader);
Vector3 unProjectMouse();
void MOON_InputProcessor(GLFWwindow *window);
void MOON_UpdateClock();
void MOON_CleanUp();

// init main ui
bool MainUI::show_control_window = true;
bool MainUI::show_explorer_window = true;
bool MainUI::show_scene_window = false;

bool MainUI::show_demo_window = false;
bool MainUI::show_VFB_window = false;
bool MainUI::show_preference_window = false;
bool MainUI::show_about_window = false;

bool MainUI::show_inspector_window = true;
bool MainUI::show_project_window = false;
bool MainUI::show_console_window = false;
bool MainUI::show_create_window = true;

Texture* MainUI::icon = NULL;
Texture* MainUI::logo = NULL;
ImGuiIO* MainUI::io;

// init math tool
unsigned long long MoonMath::seed = 1;

// init scene manager
Camera* MOON_SceneCamera = NULL;
Camera* MOON_CurrentCamera = NULL;
float Camera::MouseSensitivity = 0.025f;
int SceneManager::delID = -1;
unsigned int SceneManager::objectCounter = 1;
std::vector<ObjectBase*> SceneManager::objectList;
std::vector<ObjectBase*> SceneManager::matchedList;
float SceneManager::Clock::deltaTime = 0;
float SceneManager::Clock::lastFrame = 0;

std::multimap<std::string, Light*> MOON_LightManager::itemMap;
std::multimap<std::string, Material*> MOON_MaterialManager::itemMap;
std::multimap<std::string, Shader*> MOON_ShaderManager::itemMap;
std::multimap<std::string, Texture*> MOON_TextureManager::itemMap;
std::multimap<std::string, Model*> MOON_ModelManager::itemMap;
std::multimap<std::string, Camera*> MOON_CameraManager::itemMap;

bool MOON_LightManager::sizeFlag = true;
bool MOON_MaterialManager::sizeFlag = true;
bool MOON_ShaderManager::sizeFlag = true;
bool MOON_TextureManager::sizeFlag = true;
bool MOON_ModelManager::sizeFlag = true;
bool MOON_CameraManager::sizeFlag = true;
bool* MOON_InputManager::selection = NULL;
Material* MOON_MaterialManager::defaultMat = NULL;
Sphere* MOON_MaterialManager::matBall = NULL;

Vector2 MOON_MousePos(-2.0f, -2.0f);
Vector2 MOON_InputManager::mouseOffset = Vector2::ZERO();
Vector2 MOON_InputManager::mouseScrollOffset = Vector2::ZERO();
int MOON_InputManager::mouseButton = 0;
int MOON_InputManager::mouseAction = 0;
int MOON_InputManager::mouseMods = 0;
bool MOON_InputManager::mouse_left_hold = false;
bool MOON_InputManager::mouse_middle_hold = false;
bool MOON_InputManager::mouse_right_hold = false;
bool MOON_InputManager::isHoverUI = false;
std::vector<unsigned int> MOON_InputManager::selected;