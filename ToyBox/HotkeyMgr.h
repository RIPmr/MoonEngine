#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "MoonEnums.h"

namespace MOON {

	extern class MObject;
	class HotKeyManager {
	public:
		enum SelectType {
			rectangle,
			circle,
			lasso,
			paint
		};

		static ViewportState state;
		static SnapMode snapType;
		static bool enableSnap;

		static MObject* editTarget;

		static void MOON_InputProcessor(GLFWwindow *window);
		static void SelectRegion(const SelectType& type = rectangle) {

		}
	};

}
