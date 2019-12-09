#pragma once
#include <glad/glad.h> 
#include <GLFW/glfw3.h>
#include <string>
#include <iostream>

#include "stb_image.h"
#include "Vector3.h"
#include "Vector4.h"
#include "Matrix4x4.h"
#include "Color.h"

namespace MOON {

	extern class Model;
	class DEBUG {
	public:
		static void DebugLine(const Vector3 &start, const Vector3 &end, const Vector4 &color = Color::WHITE(), const float &lineWidth = 1.0f);
		
		static void DrawBBox(const Model* object, const Vector4 &color = Color::WHITE(), const float &lineWidth = 1.0f);

		inline static void Separator(unsigned int cnt) {
			std::cout << std::endl;
			while (cnt--) std::cout << "-";
			std::cout << std::endl;
		}

		// TODO
		static void Log(const std::string& info);
	};

}