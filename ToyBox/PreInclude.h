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
* @date		2020-10-07
* @version	0.1.2
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

#include "CodeEditor.h"
#include "Coroutine.h"
#include "ThreadPool.h"
#include "AssetLoader.h"
#include "Graphics.h"
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
#include "Shapes.h"
#include "Helpers.h"
#include "Model.h"
#include "Light.h"
#include "OBJMgr.h"
#include "HotkeyMgr.h"
#include "OperatorMgr.h"
#include "BVH.h"
#include "PostEffects.h"

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
void LoadIcons(ImGuiIO *io);
void MOON_CoroutineLoop();
void MOON_UpdateClock();
void MOON_DrawMainUI();
void MOON_InitEngine();
void MOON_CleanUp();

// global parameters
std::vector<float> Graphics::ground;
int Graphics::edit_mode_point_size = 4;

// init Pipe Mgr
GLfloat Graphics::quadVertices[20] = {
	// Positions        // Texture Coords
	-1.0f,  1.0f, 0.0f, 0.0f, 1.0f,
	-1.0f, -1.0f, 0.0f, 0.0f, 0.0f,
	 1.0f,  1.0f, 0.0f, 1.0f, 1.0f,
	 1.0f, -1.0f, 0.0f, 1.0f, 0.0f
};
PipelineMode Graphics::pipeline				= PipelineMode::FORWARD_SHADING;
ShadingMode Graphics::shading				= ShadingMode::DEFAULT;
SystemProcess Graphics::process				= SystemProcess::sys_init;
SceneView Graphics::currDrawTarget			= SceneView::top;
int Graphics::enviroment					= EnviromentType::env_pure_color;
int Graphics::AAType						= AntiAliasingType::MSAA;
unsigned int Graphics::quadVAO				= MOON_UNSPECIFIEDID;
unsigned int Graphics::quadVBO				= MOON_UNSPECIFIEDID;
float Graphics::shadowDistance				= 1000;
bool Graphics::antiAliasing					= false;

Model* MOON_ModelManager::skyDome			= MOON_UNSPECIFIEDID;

// init Main UI
bool MainUI::sceneWndFocused				= false;
bool MainUI::show_control_window			= false;
bool MainUI::show_explorer_window			= true;
bool MainUI::show_scene_window				= true;

bool MainUI::show_demo_window				= false;
bool MainUI::show_preference_window			= false;
bool MainUI::show_about_window				= false;

bool MainUI::show_inspector_window			= true;
bool MainUI::show_project_window			= true;
bool MainUI::show_console_window			= true;
bool MainUI::show_create_window				= true;
bool MainUI::show_enviroment_editor			= false;

bool MainUI::show_ribbon					= true;
bool MainUI::show_timeline					= true;
bool MainUI::show_profiler					= true;
bool MainUI::show_code_editor				= false;
bool MainUI::show_render_setting	 		= false;
bool MainUI::show_material_editor			= false;
bool MainUI::show_nn_manager				= false;
bool MainUI::show_right_click_menu			= false;

ImGuiStyle*									  MainUI::style;
Texture*	MainUI::icon					= MOON_UNSPECIFIEDID;
Texture*	MainUI::logo					= MOON_UNSPECIFIEDID;
Texture*	MainUI::logoFull				= MOON_UNSPECIFIEDID;
ImGuiIO*	MainUI::io						= MOON_UNSPECIFIEDID;

// init class-like windows
MaterialEditor								  MainUI::matEditor;
CodeEditor									  MainUI::CEditor;
std::vector<Plotter*>						  PlotManager::plotList;
unsigned int PlotManager::cnt				= 0;

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
std::vector<Camera*>						  MOON_SceneCameras;
Camera* MOON_ActiveCamera					= MOON_UNSPECIFIEDID;
int SceneManager::delID						= MOON_AUTOID;
unsigned int SceneManager::objectCounter	= 0;
float SceneManager::Clock::deltaTime		= MOON_UNSPECIFIEDID;
float SceneManager::Clock::lastFrame		= MOON_UNSPECIFIEDID;
bool SceneManager::debug					= false;
bool SceneManager::exitFlag					= false;
BVH* SceneManager::sceneBVH					= MOON_UNSPECIFIEDID;

ShadingMode SceneManager::splitShading[4]	= { DEFAULT , WIRE , WIRE , WIRE };

std::vector<ObjectBase*>					  SceneManager::objectList;
std::vector<ObjectBase*>					  SceneManager::matchedList;

// init Containers
std::multimap<std::string, Light*>			  MOON_LightManager::itemMap;
std::multimap<std::string, Material*>		  MOON_MaterialManager::itemMap;
std::multimap<std::string, Shader*>			  MOON_ShaderManager::itemMap;
std::multimap<std::string, Texture*>		  MOON_TextureManager::itemMap;
std::multimap<std::string, Model*>			  MOON_ModelManager::itemMap;
std::multimap<std::string, Camera*>			  MOON_CameraManager::itemMap;
std::multimap<std::string, Shape*>			  MOON_ShapeManager::itemMap;
std::multimap<std::string, Helper*>			  MOON_HelperManager::itemMap;

// init Managers
bool MOON_LightManager::sizeFlag				= true;
bool MOON_MaterialManager::sizeFlag				= true;
bool MOON_ShaderManager::sizeFlag				= true;
bool MOON_TextureManager::sizeFlag				= true;
bool MOON_ModelManager::sizeFlag				= true;
bool MOON_CameraManager::sizeFlag				= true;
bool MOON_ShapeManager::sizeFlag				= true;
bool MOON_HelperManager::sizeFlag				= true;
bool MOON_InputManager::lockSelection			= false;
bool MOON_InputManager::isSelectionChanged		= false;
unsigned int MOON_InputManager::hoverID			= MOON_UNSPECIFIEDID;
Shader* MOON_ShaderManager::lineShader			= MOON_UNSPECIFIEDID;
Shader* MOON_ShaderManager::outlineShader		= MOON_UNSPECIFIEDID;
Shader* MOON_ShaderManager::overlayShader		= MOON_UNSPECIFIEDID;
Material* MOON_MaterialManager::defaultMat		= MOON_UNSPECIFIEDID;
FrameBuffer* MOON_TextureManager::SHADOWMAP		= MOON_UNSPECIFIEDID;
FrameBuffer* MOON_TextureManager::IDLUT			= MOON_UNSPECIFIEDID;
Texture* MOON_TextureManager::HDRI				= MOON_UNSPECIFIEDID;
std::vector<FrameBuffer*>						  MOON_TextureManager::SCENEBUFFERS;

Vector2 MOON_MousePos							= Vector2(-2.0f, -2.0f);
Vector2 MOON_InputManager::mouseOffset			= Vector2::ZERO();
Vector2 MOON_InputManager::mouseScrollOffset	= Vector2::ZERO();
int MOON_InputManager::mouseButton				= MOON_UNSPECIFIEDID;
int MOON_InputManager::mouseAction				= MOON_UNSPECIFIEDID;
int MOON_InputManager::mouseMods				= MOON_UNSPECIFIEDID;
int MOON_InputManager::keyButton				= MOON_UNSPECIFIEDID;
int MOON_InputManager::keyAction				= MOON_UNSPECIFIEDID;
int MOON_InputManager::keyMods					= MOON_UNSPECIFIEDID;
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
Dummy* Gizmo::globalVirtualDummy				= nullptr;
DummyMap Gizmo::globalDummyMap					= DummyMap();

// NN params
unsigned int NN::NNM::graphCnt					= 0;
std::vector<NN::NGraph*>						  NN::NNM::graphList;
NN::NGraph*										  NN::NNM::globalNNGraph;
NN::NGraph*										  NN::NNM::currentGraph;

// Operator Manager
Operators* available_operators					= NULL;

// init Hotkey Mgr
ViewportState HotKeyManager::state				= FREE;
SnapMode HotKeyManager::snapType				= vertex;
Element  HotKeyManager::globalEditElem			= VERT;
bool	 HotKeyManager::enableSnap				= false;
MObject* HotKeyManager::globalEditTarget		= nullptr;

unsigned int BVH::divisionAxis					= 0;

// regist post-processing effects
std::map<std::string, EffectCreator>			  PostFactory::effectList;
REGIST_POST_EFFECT(ScreenSpaceReflection);
REGIST_POST_EFFECT(ColorSpaceConverter);
REGIST_POST_EFFECT(ColorCorrection);
REGIST_POST_EFFECT(ToneMapping);
REGIST_POST_EFFECT(Exposure);
REGIST_POST_EFFECT(Levels);
REGIST_POST_EFFECT(Curve);
REGIST_POST_EFFECT(Bloom);
REGIST_POST_EFFECT(Flare);
REGIST_POST_EFFECT(SSAO);
REGIST_POST_EFFECT(FXAA);