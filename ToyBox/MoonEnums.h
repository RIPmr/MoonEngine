#pragma once

namespace MOON {
	#define MOON_WndSize SceneManager::SCR_SIZE
	#define MOON_OutputSize Renderer::OUTPUT_SIZE
	#define MOON_OutputTexID Renderer::outputTexID
	#define MOON_CountObject SceneManager::GetObjectNum()
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

	enum Direction {
		FORWARD,
		BACKWARD,
		LEFT,
		RIGHT,
		UP,
		DOWN
	};

	enum CoordSys {
		WORLD,
		LOCAL,
		PARENT,
		SCREEN
	};

	enum class GizmoPos {
		center,
		pivot
	};

	enum class GizmoMode {
		none,
		translate,
		rotate,
		scale,
		link
	};
}