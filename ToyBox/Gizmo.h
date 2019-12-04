#pragma once
#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <vector>
#include <iostream>

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
			if (m_Theta < 0.0001f)m_Theta = 0.0001f;
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

		// Gizmos
		inline static Vector3 Translate(const Shader* shader, const Ray& ray, const Direction& dir,
										Transform *trans, Vector3& cAxisPoint_O, bool& xActive, 
										float maxCamRayLength, const bool &mousePressed) {
			float		scrDist = Vector3::Distance(ray.pos, trans->position) / 10.0f;
			Vector3		cRayPoint, cAxisPoint, deltaVec;
			Matrix4x4	model;

			// Axis -------------------------------------------------------------------------
			Ray axis; axis.pos = trans->position;			

			if (manipCoord == CoordSys::LOCAL) {
				axis.dir = trans->GetLocalAxis(dir);
				model = Matrix4x4::Rotate(Matrix4x4::ScaleMat(scrDist), trans->rotation * RotAxisByDir(dir));
			} else {
				axis.dir = Vector3::WORLD(dir);
				model = Matrix4x4::Rotate(Matrix4x4::ScaleMat(scrDist), RotAxisByDir(dir));
			} axis.pos = axis.PointAtParameter(-maxCamRayLength);

			// calculate closest dist
			bool lineDist = MoonMath::closestDistanceBetweenLines(ray, axis, cRayPoint, cAxisPoint, maxCamRayLength, maxCamRayLength * 2) < threshold * scrDist;
			Vector3 axisProjection = cAxisPoint - trans->position;
			xActive = mousePressed ? xActive : lineDist && (axisProjection.magnitude() <= scrDist) && Vector3::Angle(axisProjection, axis.dir) < 1.0f;
			
			// draw gizmo
			DrawPrototype(shader, Matrix4x4::Translate(model, trans->position), isActive ? (xActive ? Color::YELLOW() : ColorByDir(dir)) : Color::Gray());
			
			// get delta vector
			if (isActive && xActive) deltaVec += Vector3::Projection(cAxisPoint - cAxisPoint_O, axis.dir);
			cAxisPoint_O.setValue(cAxisPoint);

			return deltaVec;
		}
		inline static void Rotate() {

		}
		inline static void Scale() {

		}

		inline static void Manipulate(const Ray& ray, void* transform, const Shader* shader, const bool& mousePressed, const float maxCamRayLength = 10000.0f) {
			static Vector3 cAxisPoint_X, cAxisPoint_Y, cAxisPoint_Z;
			static bool	   xActive, yActive, zActive;

			Transform	   *trans = (Transform*)transform;
			Vector3		   deltaEuler;
			Vector3		   deltaVec;

			switch (gizmoMode) {
				case GizmoMode::translate:  deltaVec += Translate(shader, ray, Direction::UP, trans, cAxisPoint_Y, yActive, maxCamRayLength, mousePressed);
											deltaVec += Translate(shader, ray, Direction::LEFT, trans, cAxisPoint_X, xActive, maxCamRayLength, mousePressed);
											deltaVec += Translate(shader, ray, Direction::FORWARD, trans, cAxisPoint_Z, zActive, maxCamRayLength, mousePressed);
											break;
				case GizmoMode::rotate	 :  
			}

			if (isActive && mousePressed) trans->Translate(deltaVec);
		}

	private:
		static float threshold;

		static std::vector<float> circle;
		static std::vector<float> translate;

		inline static void DrawPrototype(const Shader* shader, const Matrix4x4& mat, const Vector4& color) {
			if (gizmoMode == GizmoMode::none) return;
			// TODO : link line
			if (gizmoMode == GizmoMode::link) return;

			// get data
			std::vector<float> &data = (gizmoMode == GizmoMode::rotate ? circle : translate);
			int drawSize = (gizmoMode == GizmoMode::rotate ? data.size() / 3 : 2);

			// configure shader
			shader->setVec4("lineColor", color);
			shader->setMat4("model", mat);

			// vertex array object
			unsigned int VAO;
			glGenVertexArrays(1, &VAO);
			// vertex buffer object
			unsigned int VBO;
			glGenBuffers(1, &VBO);
			// bind buffers
			glBindVertexArray(VAO);
			glBindBuffer(GL_ARRAY_BUFFER, VBO);
			// line width
			glLineWidth(1.0);
			// copy data
			glBufferData(GL_ARRAY_BUFFER, data.size() * sizeof(float), &data[0], GL_STATIC_DRAW);
			// vertex data format
			glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
			glEnableVertexAttribArray(0);
			// unbind buffers
			glBindBuffer(GL_ARRAY_BUFFER, 0);
			glDrawArrays(GL_LINE_STRIP, 0, drawSize);
			glBindVertexArray(0);
			// delete buffer object
			glDeleteVertexArrays(1, &VAO);
			glDeleteBuffers(1, &VBO);
		}

		inline static Quaternion RotAxisByDir(const Direction& dir) {
			switch (dir) {
			case Direction::UP: return Quaternion::identity();
			case Direction::LEFT: return Quaternion(0, 0, -90.0f);
			case Direction::FORWARD: return Quaternion(90.0f, 0, 0);
			}
		}
		inline static Vector4 ColorByDir(const Direction& dir) {
			switch (dir) {
			case Direction::UP: return Color::GREEN();
			case Direction::LEFT: return Color::RED();
			case Direction::FORWARD: return Color::BLUE();
			}
		}
	};
}