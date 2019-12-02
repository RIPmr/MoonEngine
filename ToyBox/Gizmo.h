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

namespace MOON {
	class Gizmo {
	public:
		static bool isActive;

		static CoordSys manipCoord;
		static GizmoPos gizmoPos;
		static GizmoMode gizmoMode;

		static std::vector<float> circle;
		static std::vector<float> translate;


		inline static void Manipulate(GizmoMode mode, Transform& transform, const Matrix4x4& camera_matrix);

		inline static void DrawPrototype(const Shader* shader, const Matrix4x4& mat, const Vector4& color) {
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

		inline static void Draw(const Shader* shader, const void* transform) {
			Transform* trans = (Transform*)transform;
			Matrix4x4  model;

			// Y-Axis
			if (manipCoord == CoordSys::LOCAL) model = Matrix4x4::Rotate(model, trans->rotation);
			DrawPrototype(shader, Matrix4x4::Translate(model, trans->position), Color::GREEN());

			// X-Axis
			if (manipCoord == CoordSys::LOCAL) model = Matrix4x4::Rotate(Matrix4x4::identity(), trans->rotation * Quaternion(0, 0, 90.0f));
			else model = Matrix4x4::Rotate(Matrix4x4::identity(), Quaternion(0, 0, 90.0f));
			DrawPrototype(shader, Matrix4x4::Translate(model, trans->position), Color::RED());

			// Z-Axis
			if (manipCoord == CoordSys::LOCAL) model = Matrix4x4::Rotate(Matrix4x4::identity(), trans->rotation * Quaternion(90.0f, 0, 0));
			else model = Matrix4x4::Rotate(Matrix4x4::identity(), Quaternion(90.0f, 0, 0));
			DrawPrototype(shader, Matrix4x4::Translate(model, trans->position), Color::BLUE());

			// disable anti-aliasing
			glDisable(GL_LINE_SMOOTH);
			glDisable(GL_BLEND);
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