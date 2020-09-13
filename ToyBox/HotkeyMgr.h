#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include "MoonEnums.h"

namespace MOON {
	class HotKeyManager {
	public:
		static ViewportState state;
		static SnapMode snapType;
		static bool enableSnap;

		static void MOON_InputProcessor(GLFWwindow *window);
	};
}
