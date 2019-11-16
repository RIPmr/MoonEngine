#pragma once

#include "Vector4.h"

namespace moon {
	class Color {
	public:
		static Vector4 RED() { return Vector4(1.0, 0.0, 0.0, 1.0); }
		static Vector4 BLUE() { return Vector4(0.0, 0.0, 1.0, 1.0); }
		static Vector4 GREEN() { return Vector4(0.0, 1.0, 0.0, 1.0); }
		static Vector4 WHITE() { return Vector4(1.0, 1.0, 1.0, 1.0); }
		static Vector4 BLACK() { return Vector4(0.0, 0.0, 0.0, 1.0); }

		inline static float GrayScaleVal(const Vector4 &color) { return (color.x + color.y + color.z) / 3.0f; }
		inline static Vector4 GrayScaleColor(const Vector4 &color) {
			float grayCol = GrayScaleVal(color);
			return Vector4(grayCol, grayCol, grayCol, color.w);
		}

		// TODO
		// convert object ID to unique color
		inline static Vector4 IDEncoder(const unsigned int &ID) {
		
		}
		// convert object color to object ID
		inline static unsigned int IDDecoder(const Vector4 &Color) {
		
		}
	};
}