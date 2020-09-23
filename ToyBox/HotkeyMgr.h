#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "MoonEnums.h"

namespace MOON {
	extern class Vector2;
	extern class MObject;
	extern class Dummy;
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

		static MObject* globalEditTarget;
		static Element  globalEditElem;

		static void MOON_InputProcessor(GLFWwindow *window);
		static void SelectRegion(const SelectType& type = rectangle);
		static void ConvertRegionToSelection(const Vector2& start, const Vector2& end);
	};

}
