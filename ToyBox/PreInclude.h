#pragma once
#define MOON_WndSize SceneManager::SCR_SIZE
#define MOON_OutputSize Renderer::OUTPUT_SIZE
#define MOON_OutputTexID Renderer::outputTexID
#define MOON_SceneCamera SceneManager::CameraManager::sceneCamera
#define MOON_MousePos SceneManager::InputManager::mousePos
#define MOON_Clock SceneManager::Clock

#define MOON_LightManager SceneManager::LightManager
#define MOON_MaterialManager SceneManager::MaterialManager
#define MOON_ShaderManager SceneManager::ShaderManager
#define MOON_TextureManager SceneManager::TextureManager
#define MOON_ModelManager SceneManager::ModelManager
#define MOON_CameraManager SceneManager::CameraManager

#define STB_IMAGE_IMPLEMENTATION
#define HAVE_STRUCT_TIMESPEC
#define OBJL_CONSOLE_OUTPUT
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
#include "ObjectBase.h"
#include "Camera.h"
#include "MShader.h"
#include "Model.h"
#include "Light.h"

using namespace moon;

// function declaration
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void processInput(GLFWwindow *window);
GLFWwindow* InitWnd();
void MOON_CleanUp();
void DrawGround(const float &space, const int &gridCnt, const Shader* groundShader);

// init main ui
bool MainUI::show_control_window = true;
bool MainUI::show_explorer_window = false;
bool MainUI::show_scene_window = false;

bool MainUI::show_demo_window = false;
bool MainUI::show_VFB_window = false;
bool MainUI::show_preference_window = false;
bool MainUI::show_about_window = false;

bool MainUI::show_inspector_window = true;
bool MainUI::show_project_window = false;
bool MainUI::show_console_window = false;
bool MainUI::show_create_window = true;

GLuint MainUI::iconID = -1;
GLuint MainUI::logoID = -1;
int MainUI::iconWidth = 0;
int MainUI::iconHeight = 0;
int MainUI::logoWidth = 0;
int MainUI::logoHeight = 0;

// init math tool
unsigned long long MoonMath::seed = 1;

// init scene manager
unsigned int SceneManager::objectCounter = 1;
Camera* MOON_SceneCamera = new Camera(Vector3(0.0f, 5.0f, 20.0f));
float SceneManager::Clock::deltaTime = 0;
float SceneManager::Clock::lastFrame = 0;
Vector2 MOON_MousePos(-2.0f, -2.0f);

std::multimap<std::string, Light*> MOON_LightManager::itemMap;
std::multimap<std::string, Material*> MOON_MaterialManager::itemMap;
std::multimap<std::string, Shader*> MOON_ShaderManager::itemMap;
std::multimap<std::string, Texture*> MOON_TextureManager::itemMap;
std::multimap<std::string, Model*> MOON_ModelManager::itemMap;
std::multimap<std::string, Camera*> MOON_CameraManager::itemMap;