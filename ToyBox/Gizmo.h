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

		static CoordSys manipCoord;
		static GizmoPos gizmoPos;
		static GizmoMode gizmoMode;

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

		// TODO
		inline static void Translate() {

		}
		inline static void Rotate() {

		}
		inline static void Scale() {

		}

		inline static void Manipulate(const Ray& ray, void* transform, const Shader* shader, const bool& mousePressed, const float maxCamRayLength = 10000.0f, float threshold = 0.1f) {
			Matrix4x4 model;
			Transform *trans = (Transform*)transform;
			Vector3   deltaVec;
			float	  scrDist = Vector3::Distance(ray.pos, trans->position) / 10.0f;
			float	  lineDist;

			static bool	   xActive, yActive, zActive;
			static Vector3 cRayPoint, cNewAxisPoint;
			static Vector3 cAxisPoint_X, cAxisPoint_Y, cAxisPoint_Z;
			threshold *= scrDist;

			// Y-Axis -----------------------------------------------------------------------
			Ray axis; axis.pos = trans->position;
			if (manipCoord == CoordSys::LOCAL) axis.dir = trans->up();
			else axis.dir = Vector3::WORLD(Direction::UP);
			
			if (manipCoord == CoordSys::LOCAL) model = Matrix4x4::Rotate(Matrix4x4::ScaleMat(scrDist), trans->rotation);
			lineDist = MoonMath::closestDistanceBetweenLines(ray, axis, cRayPoint, cNewAxisPoint, maxCamRayLength, scrDist);
			xActive = mousePressed ? xActive : lineDist < threshold;
			DrawPrototype(shader, Matrix4x4::Translate(Matrix4x4::ScaleMat(scrDist), trans->position), isActive ?  (xActive ? Color::YELLOW() : Color::GREEN()) : Color::Gray());
			// get delta vector
			if (isActive && xActive) deltaVec += cNewAxisPoint - cAxisPoint_X;
			cAxisPoint_X.setValue(cNewAxisPoint);

			// X-Axis -----------------------------------------------------------------------
			if (manipCoord == CoordSys::LOCAL) axis.dir = trans->left();
			else axis.dir = Vector3::WORLD(Direction::LEFT);

			if (manipCoord == CoordSys::LOCAL) model = Matrix4x4::Rotate(Matrix4x4::ScaleMat(scrDist), trans->rotation * Quaternion(0, 0, -90.0f));
			else model = Matrix4x4::Rotate(Matrix4x4::ScaleMat(scrDist), Quaternion(0, 0, -90.0f));
			lineDist = MoonMath::closestDistanceBetweenLines(ray, axis, cRayPoint, cNewAxisPoint, maxCamRayLength, scrDist);
			DebugLine(cRayPoint, cNewAxisPoint);
			yActive = mousePressed ? yActive : lineDist < threshold;
			DrawPrototype(shader, Matrix4x4::Translate(model, trans->position), isActive ? (yActive ? Color::YELLOW() : Color::RED()) : Color::Gray());
			if (isActive && yActive) deltaVec += cNewAxisPoint - cAxisPoint_Y;
			cAxisPoint_Y.setValue(cNewAxisPoint);

			// Z-Axis -----------------------------------------------------------------------
			if (manipCoord == CoordSys::LOCAL) axis.dir = trans->forward();
			else axis.dir = Vector3::WORLD(Direction::FORWARD);

			if (manipCoord == CoordSys::LOCAL) model = Matrix4x4::Rotate(Matrix4x4::ScaleMat(scrDist), trans->rotation * Quaternion(90.0f, 0, 0));
			else model = Matrix4x4::Rotate(Matrix4x4::ScaleMat(scrDist), Quaternion(90.0f, 0, 0));
			lineDist = MoonMath::closestDistanceBetweenLines(ray, axis, cRayPoint, cNewAxisPoint, maxCamRayLength, scrDist);
			zActive = mousePressed ? zActive : lineDist < threshold;
			DrawPrototype(shader, Matrix4x4::Translate(model, trans->position), isActive ? (zActive ? Color::YELLOW() : Color::BLUE()) : Color::Gray());
			if (isActive && zActive) deltaVec += cNewAxisPoint - cAxisPoint_Z;
			cAxisPoint_Z.setValue(cNewAxisPoint);

			if (isActive && mousePressed) trans->set(&(trans->position + deltaVec));
		}
		
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
	};
}