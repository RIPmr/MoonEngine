#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <vector>
#include <iostream>
#include <cmath>

#include "MoonEnums.h"
#include "Transform.h"
#include "Vector2.h"
#include "Vector3.h"
#include "Matrix4x4.h"
#include "MathUtils.h"
#include "ObjectBase.h"
#include "MShader.h"
#include "Color.h"
#include "Utility.h"
#include "Ray.h"

namespace MOON {
	class Gizmo {
	public:
		static bool hoverGizmo;
		static bool isActive;

		static CoordSys  manipCoord;
		static GizmoPos  gizmoPos;
		static GizmoMode gizmoMode;

		// Set parameters
		inline static void SetMode(const GizmoMode& mode) { gizmoMode = mode; }
		inline static void SetPivot(const GizmoPos& pos) { gizmoPos = pos; }
		inline static void SetCoordSys(const CoordSys& coord) { manipCoord = coord; }
		inline static void SetThreshold(const float& newThreshold) { threshold = newThreshold; }
		inline static void RecalcCircle(float m_Theta = 12.0f, bool close = true) {
			m_Theta *= Deg2Rad;
			if (m_Theta < 0.0001f) m_Theta = 0.0001f;
			float m_Radius = 1.0f;

			circle.clear();
			for (float theta = 0; theta < 2 * PI; theta += m_Theta) {
				float x = m_Radius * cos(theta);
				float z = m_Radius * sin(theta);
				if (abs(theta - 2 * PI) < 0.01f) continue;
				circle.push_back(x); circle.push_back(0); circle.push_back(z);
			}

			if (close) { circle.push_back(circle[0]); circle.push_back(circle[1]); circle.push_back(circle[2]); }
		}
		inline static void RecalcTranslate() {
			translate.clear();

			translate.push_back(0); translate.push_back(0); translate.push_back(0);
			translate.push_back(0); translate.push_back(1); translate.push_back(0);
		}

	#pragma region Gizmos
		static Vector3 Translate(const Ray& ray, const Direction& dir, Transform *trans, Vector3& cAxisPoint_O, bool& xActive, float maxCamRayLength);
		static Quaternion Rotate(const Ray& ray, const Direction& dir, Matrix4x4& model, Transform *trans, Vector3& cAxisPoint_O, bool& xActive);
		static Vector3 Scale(const Ray& ray, const Direction& dir, Transform *trans, Vector3& cAxisPoint_O, bool& xActive, float maxCamRayLength);
		// screen space version
		static Quaternion Rotate_SS(const Ray& ray, const Direction& dir, Matrix4x4& model, Transform *trans, Vector3& cAxisPoint_O, Vector2& screenPos_O, bool& xActive, float& deltaAngle);
		static void Manipulate(void* transform, const float maxCamRayLength = 10000.0f);
	#pragma endregion

	#pragma region draw methods
		static void DrawPointDirect(const Vector3 &position, const Vector4 &color = Color::WHITE(), const float &pointSize = 1.0f, const Matrix4x4 model = Matrix4x4::identity());
		static void DrawPointsDirect(const std::vector<Vector3> &points, const Vector4 &color = Color::WHITE(), const float &pointSize = 1.0f, const Matrix4x4 model = Matrix4x4::identity());
		static void DrawLineDirect(const Vector3 &start, const Vector3 &end, const Vector4 &color = Color::WHITE(), const float &lineWidth = 1.0f, const Matrix4x4 model = Matrix4x4::identity());
		static void DrawLinesDirect(const std::vector<Vector3> &lines, const Vector4 &color = Color::WHITE(), const float &lineWidth = 1.0f, const bool &isStrip = true, const Matrix4x4 model = Matrix4x4::identity(), const Shader* overrideShader = NULL);
		static void DrawDashedLineDirect(const Vector3 &start, const Vector3 &end, const Vector4 &color = Color::WHITE(), const float &lineWidth = 1.0f, const Matrix4x4 model = Matrix4x4::identity());
	#pragma endregion

	#pragma region draw methods Ex
		static void DrawPoint(const Vector3 &position, const Vector4 &color = Color::WHITE(), const float &pointSize = 1.0f, const bool& depthTest = true, const Matrix4x4 model = Matrix4x4::identity(), const bool& drawActiveViewOnly = true);
		static void DrawPoints(const std::vector<Vector3> &points, const Vector4 &color = Color::WHITE(), const float &pointSize = 1.0f, const bool& depthTest = true, const Matrix4x4 model = Matrix4x4::identity(), const bool& drawActiveViewOnly = true);
		static void DrawPoints(const unsigned int& VAO, const size_t& pointNum, const Vector4 &color = Color::WHITE(), const float &pointSize = 1.0f, const bool& depthTest = true, const Matrix4x4 model = Matrix4x4::identity(), const bool& drawActiveViewOnly = true);
		static void DrawLine(const Vector3 &start, const Vector3 &end, const Vector4 &color = Color::WHITE(), const float &lineWidth = 1.0f, const bool& depthTest = true, const Matrix4x4 model = Matrix4x4::identity(), const bool& drawActiveViewOnly = true);
		static void DrawLines(const std::vector<Vector3> &lines, const Vector4 &color = Color::WHITE(), const float &lineWidth = 1.0f, const bool& depthTest = true, const bool &isStrip = true, const Matrix4x4 model = Matrix4x4::identity(), const Shader* overrideShader = NULL, const bool& drawActiveViewOnly = true);
		static void DrawDashedLine(const Vector3 &start, const Vector3 &end, const Vector4 &color = Color::WHITE(), const float &lineWidth = 1.0f, const bool& depthTest = true, const Matrix4x4 model = Matrix4x4::identity(), const bool& drawActiveViewOnly = true);
	#pragma endregion

	#pragma region draw primitives
		static void DrawCube(const Vector3& position, const Quaternion& rotation, const Vector3& scale);
		static void DrawSphere(const Vector3& position, const Quaternion& rotation, const float& radius);
		static void DrawCircle(const Vector3& position, const Quaternion& rotation, const float& radius);
		static void DrawRectangle(const Vector3& position, const Quaternion& rotation, const Vector2& size);
	#pragma endregion

	#pragma region Prototypes
		static void DrawPointPrototype(const std::vector<float> &data, const Vector4 &color = Color::WHITE(), const float &pointSize = 1.0f, const Matrix4x4 model = Matrix4x4::identity());
		static void DrawPointPrototype(const std::vector<Vector3> &data, const Vector4 &color = Color::WHITE(), const float &pointSize = 1.0f, const Matrix4x4 model = Matrix4x4::identity());
		static void DrawPointPrototype(const unsigned int& VAO, const size_t& pointNum, const Vector4 &color = Color::WHITE(), const float &pointSize = 1.0f, const Matrix4x4 model = Matrix4x4::identity());
		static void DrawLinePrototype(const std::vector<float> &data, const Vector4 &color = Color::WHITE(), const float &lineWidth = 1.0f, const bool &isStrip = true, const Matrix4x4 model = Matrix4x4::identity(), const Shader* overrideShader = NULL);
		static void DrawLinePrototype(const std::vector<Vector3> &data, const Vector4 &color = Color::WHITE(), const float &lineWidth = 1.0f, const bool &isStrip = true, const Matrix4x4 model = Matrix4x4::identity(), const Shader* overrideShader = NULL);
	#pragma endregion

	private:
		static float threshold;

		static std::vector<float> circle;
		static std::vector<float> translate;

		static std::vector<float> GenerateDashedLine(const Vector3 &start, const Vector3 &end, const float &gap);
		static void DrawTransPrototype(const Matrix4x4& mat, const Vector4& color);

		inline static Quaternion RotAxisByDir(const Direction& dir) {
			switch (dir) {
				case Direction::UP:		return Quaternion::identity();
				case Direction::LEFT:	return Quaternion(0, 0, -90.0f);
				case Direction::FORWARD:return Quaternion(90.0f, 0, 0);
			}
		}
		
		inline static Vector4 ColorByDir(const Direction& dir, const bool& disable, const bool& active) {
			if (disable) return Color::Gray();
			else if (active) return Color::YELLOW();

			switch (dir) {
				case Direction::UP:		return Color::GREEN();
				case Direction::LEFT:	return Color::RED();
				case Direction::FORWARD:return Color::BLUE();
			}
		}
	};
}