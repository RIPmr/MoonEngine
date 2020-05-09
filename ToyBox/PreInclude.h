/*	   ___           ___           ___           ___
*	  /\__\         /\  \         /\  \         /\__\
*  	 /::|  |       /::\  \       /::\  \       /::|  |
*   /:|:|  |      /:/\:\  \     /:/\:\  \     /:|:|  |
*  /:/|:|__|__   /:/  \:\  \   /:/  \:\  \   /:/|:|  |__
* /:/ |::::\__\ /:/__/ \:\__\ /:/__/ \:\__\ /:/ |:| /\__\
* \/__/~~/:/  / \:\  \ /:/  / \:\  \ /:/  / \/__|:|/:/  /
*	    /:/  /   \:\  /:/  /   \:\  /:/  /      |:/:/  /
*	   /:/  /     \:\/:/  /     \:\/:/  /       |::/  /
*	  /:/  /       \::/  /       \::/  /        /:/  /
*	  \/__/         \/__/         \/__/         \/__/
*
* @author	HZT
* @date		2020-05-08
* @version	0.0.3
*/

#pragma once
#define STB_IMAGE_IMPLEMENTATION
#define HAVE_STRUCT_TIMESPEC
#pragma comment(lib, "pthreadVC2.lib")

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <pthread.h>
#include <algorithm>
#include <iostream>
#include <chrono>
#include <thread>
#include <random>
#include <map>

#include "ThreadPool.h"
#include "AssetLoader.h"
#include "PipelineMgr.h"
#include "MoonEnums.h"
#include "Debugger.h"
#include "Gizmo.h"
#include "SceneMgr.h"
#include "Vector2.h"
#include "Vector3.h"
#include "Quaternion.h"
#include "Matrix3x3.h"
#include "Matrix4x4.h"
#include "MathUtils.h"
#include "Material.h"
#include "Utility.h"
#include "Renderer.h"
#include "UIController.h"
#include "OperatorBase.h"
#include "ObjectBase.h"
#include "MShader.h"
#include "Camera.h"
#include "MatSphere.h"
#include "Transform.h"
#include "Model.h"
#include "Light.h"
#include "OBJMgr.h"

// NN heads
#include "Plotter.h"
#include "NGraph.h"
#include "Neuron.h"
#include "Optimizer.h"
#include "NFunction.h"
#include "NVariable.h"
#include "NNManager.h"
#include "FNN.h"

using namespace MOON;

// function declaration
GLFWwindow* InitWnd();
void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);
void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void MOON_GenerateGround(const float &space, const int &gridCnt);
void MOON_DrawGround(const Shader* groundShader);
void MOON_InputProcessor(GLFWwindow *window);
void MOON_UpdateClock();
void MOON_DrawMainUI();
void MOON_InitEngine();
void MOON_CleanUp();

// global parameters
std::vector<float> grid;

// init Pipe Mgr
float PipelineManager::quadVertices[24] = {
	// positions   // texCoords
	-1.0f,  1.0f,  0.0f, 1.0f,
	-1.0f, -1.0f,  0.0f, 0.0f,
	 1.0f, -1.0f,  1.0f, 0.0f,
	-1.0f,  1.0f,  0.0f, 1.0f,
	 1.0f, -1.0f,  1.0f, 0.0f,
	 1.0f,  1.0f,  1.0f, 1.0f
};
ShadingMode PipelineManager::mode			= ShadingMode::FORWARD_SHADING;
unsigned int PipelineManager::quadVAO		= MOON_UNSPECIFIEDID;
unsigned int PipelineManager::quadVBO		= MOON_UNSPECIFIEDID;

// init Main UI
bool MainUI::show_control_window			= true;
bool MainUI::show_explorer_window			= true;
bool MainUI::show_scene_window				= false;

bool MainUI::show_demo_window				= false;
bool MainUI::show_VFB_window				= false;
bool MainUI::show_preference_window			= false;
bool MainUI::show_about_window				= false;

bool MainUI::show_inspector_window			= true;
bool MainUI::show_project_window			= true;
bool MainUI::show_console_window			= true;
bool MainUI::show_create_window				= true;
bool MainUI::show_enviroment_editor			= false;

bool MainUI::show_ribbon					= true;
bool MainUI::show_timeline					= false;
bool MainUI::show_profiler					= true;
bool MainUI::show_codeEditor				= false;
bool MainUI::show_render_setting	 		= false;
bool MainUI::show_material_editor			= false;
bool MainUI::show_nn_manager				= false;

// init class-like windows
MaterialEditor								  MainUI::matEditor;
std::vector<Plotter*>						  PlotManager::plotList;
unsigned int PlotManager::cnt				= 1;

ImGuiStyle*									  MainUI::style;
Texture* MainUI::icon						= MOON_UNSPECIFIEDID;
Texture* MainUI::logo						= MOON_UNSPECIFIEDID;
Texture* MainUI::logoFull					= MOON_UNSPECIFIEDID;
ImGuiIO* MainUI::io							= MOON_UNSPECIFIEDID;

// init OBJ Loader
std::string									  OBJLoader::info;
std::vector<Vertex>							  OBJLoader::LoadedVertices;
std::vector<unsigned int>					  OBJLoader::LoadedIndices;
bool OBJLoader::gammaCorrection				= false;
float OBJLoader::progress					= 0;
// init Thread Pool
std::vector<std::thread*>					  ThreadPool::pool;
// init Math Tool
unsigned long long MoonMath::seed			= 1;
// init Dir Tree
DirNode* AssetLoader::DirTree				= MOON_UNSPECIFIEDID;
// init Scene Manager
Camera* MOON_SceneCamera					= MOON_UNSPECIFIEDID;
Camera* MOON_CurrentCamera					= MOON_UNSPECIFIEDID;
int SceneManager::delID						= MOON_AUTOID;
unsigned int SceneManager::objectCounter	= 1;
float SceneManager::Clock::deltaTime		= MOON_UNSPECIFIEDID;
float SceneManager::Clock::lastFrame		= MOON_UNSPECIFIEDID;
bool SceneManager::showbbox					= false;
bool SceneManager::wireMode					= false;
bool SceneManager::exitFlag					= false;

std::vector<ObjectBase*>					  SceneManager::objectList;
std::vector<ObjectBase*>					  SceneManager::matchedList;

// init Containers
std::multimap<std::string, Light*>			  MOON_LightManager::itemMap;
std::multimap<std::string, Material*>		  MOON_MaterialManager::itemMap;
std::multimap<std::string, Shader*>			  MOON_ShaderManager::itemMap;
std::multimap<std::string, Texture*>		  MOON_TextureManager::itemMap;
std::multimap<std::string, Model*>			  MOON_ModelManager::itemMap;
std::multimap<std::string, Camera*>			  MOON_CameraManager::itemMap;

// init Managers
bool MOON_LightManager::sizeFlag				= true;
bool MOON_MaterialManager::sizeFlag				= true;
bool MOON_ShaderManager::sizeFlag				= true;
bool MOON_TextureManager::sizeFlag				= true;
bool MOON_ModelManager::sizeFlag				= true;
bool MOON_CameraManager::sizeFlag				= true;
unsigned int MOON_InputManager::hoverID			= MOON_UNSPECIFIEDID;
Shader* MOON_ShaderManager::lineShader			= MOON_UNSPECIFIEDID;
Shader* MOON_ShaderManager::outlineShader		= MOON_UNSPECIFIEDID;
Shader* MOON_ShaderManager::screenBufferShader	= MOON_UNSPECIFIEDID;
Material* MOON_MaterialManager::defaultMat		= MOON_UNSPECIFIEDID;
Texture* MOON_TextureManager::SHADOWMAP			= MOON_UNSPECIFIEDID;
FrameBuffer* MOON_TextureManager::IDLUT			= MOON_UNSPECIFIEDID;

Vector2 MOON_MousePos							= Vector2(-2.0f, -2.0f);
Vector2 MOON_InputManager::mouseOffset			= Vector2::ZERO();
Vector2 MOON_InputManager::mouseScrollOffset	= Vector2::ZERO();
int MOON_InputManager::mouseButton				= MOON_UNSPECIFIEDID;
int MOON_InputManager::mouseAction				= MOON_UNSPECIFIEDID;
int MOON_InputManager::mouseMods				= MOON_UNSPECIFIEDID;
bool MOON_InputManager::mouse_left_hold			= false;
bool MOON_InputManager::mouse_middle_hold		= false;
bool MOON_InputManager::mouse_right_hold		= false;
bool MOON_InputManager::left_ctrl_hold			= false;
bool MOON_InputManager::left_shift_hold			= false;
bool MOON_InputManager::right_ctrl_hold			= false;
bool MOON_InputManager::right_shift_hold		= false;
bool MOON_InputManager::isHoverUI				= false;
std::vector<unsigned int>						  MOON_InputManager::selection;

// init Gizmo
CoordSys Gizmo::manipCoord						= CoordSys::LOCAL;
GizmoPos Gizmo::gizmoPos						= GizmoPos::pivot;
GizmoMode Gizmo::gizmoMode						= GizmoMode::none;
std::vector<float>								  Gizmo::circle;
std::vector<float>								  Gizmo::translate;
bool Gizmo::isActive							= true;
bool Gizmo::hoverGizmo							= false;
float Gizmo::threshold							= 0.1f;

// NN params
unsigned int NN::NNM::graphCnt					= 0;
std::vector<NN::NGraph*>						  NN::NNM::graphList;
NN::NGraph*										  NN::NNM::globalNNGraph;
NN::NGraph*										  NN::NNM::currentGraph;
