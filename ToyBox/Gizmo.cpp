#include "Gizmo.h"
#include "Graphics.h"
#include "SceneMgr.h"
#include "BoundingBox.h"

namespace MOON {
#pragma region manipulator
	Vector3 Gizmo::Translate(const Ray& ray, const Direction& dir, Transform *trans, Vector3& cAxisPoint_O, bool& xActive, float maxCamRayLength) {
		float		scrDist = MOON_ActiveCamera->isortho ? 2.0f : Vector3::Distance(ray.pos, trans->position) / 15.0f;
		Vector3		cRayPoint, cAxisPoint, deltaVec;
		Matrix4x4	model;

		// Axis -------------------------------------------------------------------------
		Ray axis; axis.pos = trans->position;

		if (manipCoord == CoordSys::LOCAL) {
			axis.dir = trans->GetLocalAxis(dir) * trans->AxisSign(dir);
			model = Matrix4x4::Rotate(Matrix4x4::ScaleMat(scrDist), trans->rotation * RotAxisByDir(dir));
		} else {
			axis.dir = Vector3::WORLD(dir);
			model = Matrix4x4::Rotate(Matrix4x4::ScaleMat(scrDist), RotAxisByDir(dir));
		} axis.pos = axis.PointAtParameter(-maxCamRayLength);

		// calculate closest dist
		bool lineDist = MoonMath::closestDistanceBetweenLines(ray, axis, cRayPoint, cAxisPoint, maxCamRayLength, maxCamRayLength * 2) < threshold * scrDist;
		Vector3 axisProjection = cAxisPoint - trans->position;
		xActive = MOON_InputManager::mouse_left_hold ? xActive : lineDist && (axisProjection.magnitude() <= scrDist) && Vector3::DirectionSign(axisProjection, axis.dir) > 0;
		
		// draw gizmo
		DrawTransPrototype(Matrix4x4::Translate(model, trans->position), ColorByDir(dir, !isActive, xActive));

		// get delta vector
		if (isActive && xActive) deltaVec = Vector3::Projection(cAxisPoint - cAxisPoint_O, axis.dir);
		cAxisPoint_O.setValue(cAxisPoint);

		return deltaVec;
	}

	Quaternion Gizmo::Rotate(const Ray& ray, const Direction& dir, Matrix4x4& model, Transform *trans, Vector3& cAxisPoint_O, bool& xActive) {
		float		scrDist = MOON_ActiveCamera->isortho ? 2.0f : Vector3::Distance(ray.pos, trans->position) / 15.0f;
		Vector3		cIntersect;
		Quaternion	deltaRot;

		// Axis -------------------------------------------------------------------------
		Ray axis; axis.pos = trans->position;

		if (manipCoord == CoordSys::LOCAL) {
			axis.dir = trans->GetLocalAxis(dir) * trans->AxisSign(dir);
			model = Matrix4x4::Rotate(Matrix4x4::ScaleMat(scrDist), trans->rotation * RotAxisByDir(dir));
		} else {
			axis.dir = Vector3::WORLD(dir);
			model = Matrix4x4::Rotate(Matrix4x4::ScaleMat(scrDist), RotAxisByDir(dir));
		}

		// calculate closest dist
		bool lineDist = MoonMath::RayDiskIntersect(ray, axis.pos, axis.dir, scrDist, cIntersect);
		xActive = MOON_InputManager::mouse_left_hold ? xActive : lineDist;

		// get delta vector
		int rotDir = -Vector3::DirectionSign((cIntersect - axis.pos).cross(cAxisPoint_O - axis.pos), axis.dir);
		if (isActive && xActive) deltaRot = Quaternion::Rotate(axis.dir, (cIntersect - cAxisPoint_O).magnitude() * rotDir);
		cAxisPoint_O.setValue(cIntersect);

		return deltaRot;
	}

	Vector3 Gizmo::Scale(const Ray& ray, const Direction& dir, Transform *trans, Vector3& cAxisPoint_O, bool& xActive, float maxCamRayLength) {
		float		scrDist = MOON_ActiveCamera->isortho ? 2.0f : Vector3::Distance(ray.pos, trans->position) / 15.0f;
		Vector3		cRayPoint, cAxisPoint, deltaVec;
		Matrix4x4	model;

		// Axis -------------------------------------------------------------------------
		Ray axis; axis.pos = trans->position;

		if (manipCoord == CoordSys::LOCAL) {
			axis.dir = trans->GetLocalAxis(dir) * trans->AxisSign(dir);
			model = Matrix4x4::Rotate(Matrix4x4::ScaleMat(scrDist), trans->rotation * RotAxisByDir(dir));
		} else {
			axis.dir = Vector3::WORLD(dir);
			model = Matrix4x4::Rotate(Matrix4x4::ScaleMat(scrDist), RotAxisByDir(dir));
		} axis.pos = axis.PointAtParameter(-maxCamRayLength);

		// calculate closest dist
		bool lineDist = MoonMath::closestDistanceBetweenLines(ray, axis, cRayPoint, cAxisPoint, maxCamRayLength, maxCamRayLength * 2) < threshold * scrDist;
		Vector3 axisProjection = cAxisPoint - trans->position;
		//std::cout << "lineDist:" << lineDist << " scrDist: " << (axisProjection.magnitude() <= scrDist) << " axis.dir: " << (Vector3::DirectionSign(axisProjection, axis.dir) > 0) << std::endl;
		xActive = MOON_InputManager::mouse_left_hold ? xActive : lineDist && (axisProjection.magnitude() <= scrDist) && Vector3::DirectionSign(axisProjection, axis.dir) > 0;

		// draw gizmo
		bool disableWorldScale = !manipCoord == CoordSys::WORLD;

		DrawTransPrototype(Matrix4x4::Translate(model, trans->position), ColorByDir(dir, !(isActive && disableWorldScale), xActive));

		// get delta vector
		if (isActive && xActive && disableWorldScale) {
			deltaVec = Vector3::WORLD(dir) * Vector3::Distance(cAxisPoint, cAxisPoint_O) * Vector3::DirectionSign(cAxisPoint - cAxisPoint_O, axis.dir);
			// NOT CORRECT
			/*if (manipCoord == CoordSys::WORLD) {
				deltaVec = Vector3(Vector3::ProjectionMag(deltaVec, trans->left()),
								   Vector3::ProjectionMag(deltaVec, trans->up()),
								   Vector3::ProjectionMag(deltaVec, trans->forward()));
			}*/
		}
		cAxisPoint_O.setValue(cAxisPoint);

		return deltaVec;
	}

	Quaternion Gizmo::Rotate_SS(const Ray& ray, const Direction& dir, Matrix4x4& model, Transform *trans, Vector3& cAxisPoint_O, Vector2& screenPos_O, bool& xActive, float& deltaAngle) {
		float		scrDist = MOON_ActiveCamera->isortho ? 2.0f : Vector3::Distance(ray.pos, trans->position) / 15.0f;
		Vector3		cIntersect;
		Quaternion	deltaRot;

		// Axis -------------------------------------------------------------------------
		Ray axis; axis.pos = trans->position;

		if (manipCoord == CoordSys::LOCAL) {
			axis.dir = trans->GetLocalAxis(dir) * trans->AxisSign(dir);
			model = Matrix4x4::Rotate(Matrix4x4::ScaleMat(scrDist), trans->rotation * RotAxisByDir(dir));
		} else {
			axis.dir = Vector3::WORLD(dir);
			model = Matrix4x4::Rotate(Matrix4x4::ScaleMat(scrDist), RotAxisByDir(dir));
		}

		// calculate closest dist
		bool lineDist = MoonMath::RayDiskIntersect(ray, axis.pos, axis.dir, scrDist, cIntersect);
		xActive = MOON_InputManager::mouse_left_hold ? xActive : lineDist;
		cAxisPoint_O.setValue(cIntersect);

		Vector2 n = MOON_MousePosNormalized;
		Vector2 pivotInScreen = MOON_ActiveCamera->WorldToScreenPos(axis.pos);
		//std::cout << Vector3::DirectionSign(axis.pos - ray.pos, axis.dir) << std::endl;
		int rotDir = Vector2::DirectionSign(screenPos_O - pivotInScreen, n - pivotInScreen) * -Vector3::DirectionSign(axis.pos - ray.pos, axis.dir);
		if (isActive && xActive) {
			deltaAngle = (n - screenPos_O).magnitude() * 30.0f * rotDir;
			deltaRot = Quaternion::Rotate(axis.dir, deltaAngle);
		}

		//DrawTransPrototype(Matrix4x4::Translate(model, trans->position), ColorByDir(dir, !isActive, xActive));

		return deltaRot;
	}

	Matrix4x4 Gizmo::Manipulate(void* transform, const float maxCamRayLength) {
		static Vector2 screenPos;
		static Vector3 cAxisPoint_X, cAxisPoint_Y, cAxisPoint_Z;
		static bool	   xActive, yActive, zActive;

		Transform	   *trans = (Transform*)transform;
		Quaternion	   deltaRot, deltaRotLocal;
		Vector3		   deltaVec;
		Vector3		   deltaSca;

		Ray ray = MOON_ActiveCamera->GetMouseRayAccurate();

		hoverGizmo = xActive | yActive | zActive;

		// offset if using center mode
		/*auto offset = Vector3::ZERO();
		if (Gizmo::gizmoPos == center && CheckType(trans->mobject, "Model"))
			offset = dynamic_cast<Model*>(trans->mobject)->bbox.center - trans->position;
		Transform* tmpTrans = new Transform(*trans); tmpTrans->position += offset;*/

		auto translate = [&]() {
			deltaVec += Translate(ray, Direction::UP, trans, cAxisPoint_Y, yActive, maxCamRayLength);
			deltaVec += Translate(ray, Direction::LEFT, trans, cAxisPoint_X, xActive, maxCamRayLength);
			deltaVec += Translate(ray, Direction::FORWARD, trans, cAxisPoint_Z, zActive, maxCamRayLength);
		};

		auto rotate = [&](float threshold) {
			Matrix4x4 modelX, modelY, modelZ; Vector3 deltaAngle;
			deltaRot = Rotate_SS(ray, Direction::UP, modelY, trans, cAxisPoint_Y, screenPos, yActive, deltaAngle.y);
			deltaRot = Rotate_SS(ray, Direction::LEFT, modelX, trans, cAxisPoint_X, screenPos, xActive, deltaAngle.x) * deltaRot;
			deltaRot = Rotate_SS(ray, Direction::FORWARD, modelZ, trans, cAxisPoint_Z, screenPos, zActive, deltaAngle.z) * deltaRot;

			deltaRotLocal = Quaternion::Rotate(trans->GetNativeAxis(UP), deltaAngle.y);
			deltaRotLocal = Quaternion::Rotate(trans->GetNativeAxis(LEFT), deltaAngle.x) * deltaRotLocal;
			deltaRotLocal = Quaternion::Rotate(trans->GetNativeAxis(FORWARD), deltaAngle.z) * deltaRotLocal;

			screenPos.setValue(MOON_MousePosNormalized);
			float dY = yActive ? cAxisPoint_Y.fastDistance(ray.pos) : INFINITY;
			float dX = xActive ? cAxisPoint_X.fastDistance(ray.pos) : INFINITY;
			float dZ = zActive ? cAxisPoint_Z.fastDistance(ray.pos) : INFINITY;

			yActive &= dY < dX && dY < dZ; xActive &= dX < dY && dX < dZ; zActive &= dZ < dX && dZ < dY;
			DrawTransPrototype(Matrix4x4::Translate(modelY, trans->position), ColorByDir(Direction::UP, !isActive, yActive));
			DrawTransPrototype(Matrix4x4::Translate(modelX, trans->position), ColorByDir(Direction::LEFT, !isActive, xActive));
			DrawTransPrototype(Matrix4x4::Translate(modelZ, trans->position), ColorByDir(Direction::FORWARD, !isActive, zActive));
		};

		auto scale = [&]() {
			deltaSca += Scale(ray, Direction::UP, trans, cAxisPoint_Y, yActive, maxCamRayLength);
			deltaSca += Scale(ray, Direction::LEFT, trans, cAxisPoint_X, xActive, maxCamRayLength);
			deltaSca += Scale(ray, Direction::FORWARD, trans, cAxisPoint_Z, zActive, maxCamRayLength);
		};

		switch (gizmoMode) {
			case GizmoMode::translate:	translate(); break;
			case GizmoMode::rotate:		rotate(0.0001f); break;
			case GizmoMode::scale:		scale(); break;
		}

		Matrix4x4 deltaMat;
		if (isActive && MOON_InputManager::mouse_left_hold) {
			if (deltaVec.magnitude() > 0) {
				trans->Translate(deltaVec);
				deltaMat = Matrix4x4::Translate(deltaMat, deltaVec);
			}
			if (deltaRot.eulerAngles.magnitude() || deltaRotLocal.eulerAngles.magnitude()) {
				trans->Rotate(Gizmo::manipCoord == WORLD ? deltaRot : deltaRotLocal, Gizmo::manipCoord);
				deltaMat = Matrix4x4::Rotate(deltaMat, Gizmo::manipCoord == WORLD ? deltaRot : deltaRotLocal);
			}
			if (deltaSca.magnitude() > 0) {
				deltaMat = Matrix4x4::Scale(deltaMat, (deltaSca + trans->localScale) / trans->localScale);
				trans->Scale(deltaSca + trans->localScale);
			}
		}
		return deltaMat;
	}
#pragma endregion

#pragma region prototypes
	void Gizmo::DrawTransPrototype(const Matrix4x4& mat, const Vector4& color) {
		if (gizmoMode == GizmoMode::none) return;
		// TODO : link line
		if (gizmoMode == GizmoMode::link) return;

		// get data
		std::vector<float> &data = (gizmoMode == GizmoMode::rotate ? circle : translate);
		int drawSize = data.size() / 3;

		// configure shader
		MOON_ShaderManager::lineShader->use();
		MOON_ShaderManager::lineShader->setVec4("lineColor", color);
		MOON_ShaderManager::lineShader->setMat4("model", mat);

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
		glLineWidth(2.0);
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

	void Gizmo::DrawLinePrototype(const std::vector<float> &data, const Vector4 &color, const float &lineWidth, const bool &isStrip, const Matrix4x4 model, const Shader* overrideShader) {
		// configure shader
		if (overrideShader == NULL) {
			overrideShader = MOON_ShaderManager::lineShader;
			overrideShader->use();
			overrideShader->setVec4("lineColor", color);
		} else overrideShader->use();
		overrideShader->setMat4("model", model);
		overrideShader->setMat4("view", MOON_ActiveCamera->view);
		overrideShader->setMat4("projection", MOON_ActiveCamera->projection);
		
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
		glLineWidth(lineWidth);
		// copy data
		glBufferData(GL_ARRAY_BUFFER, data.size() * sizeof(float), &data[0], GL_STATIC_DRAW);
		// vertex data format
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
		glEnableVertexAttribArray(0);
		// unbind buffers
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glDrawArrays(isStrip ? GL_LINE_STRIP : GL_LINES, 0, data.size() / 3);
		glBindVertexArray(0);
		// delete buffer object
		glDeleteVertexArrays(1, &VAO);
		glDeleteBuffers(1, &VBO);
	}

	void Gizmo::DrawLinePrototype(const std::vector<Vector3> &data, const Vector4 &color, const float &lineWidth, const bool &isStrip, const Matrix4x4 model, const Shader* overrideShader) {
		// configure shader
		if (overrideShader == NULL) {
			overrideShader = MOON_ShaderManager::lineShader;
			overrideShader->use();
			overrideShader->setVec4("lineColor", color);
		} else overrideShader->use();
		overrideShader->setMat4("model", model);
		overrideShader->setMat4("view", MOON_ActiveCamera->view);
		overrideShader->setMat4("projection", MOON_ActiveCamera->projection);

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
		glLineWidth(lineWidth);
		// copy data
		glBufferData(GL_ARRAY_BUFFER, data.size() * sizeof(Vector3), &data[0], GL_STATIC_DRAW);
		// vertex data format
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vector3), (void*)0);
		glEnableVertexAttribArray(0);
		// unbind buffers
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glDrawArrays(isStrip ? GL_LINE_STRIP : GL_LINES, 0, data.size());
		glBindVertexArray(0);
		// delete buffer object
		glDeleteVertexArrays(1, &VAO);
		glDeleteBuffers(1, &VBO);
	}

	void Gizmo::DrawPointPrototype(const std::vector<float> &data, const Vector4 &color, const float &pointSize, const Matrix4x4 model) {
		// configure shader
		MOON_ShaderManager::lineShader->use();
		MOON_ShaderManager::lineShader->setVec4("lineColor", color);
		MOON_ShaderManager::lineShader->setMat4("model", model);
		MOON_ShaderManager::lineShader->setMat4("view", MOON_ActiveCamera->view);
		MOON_ShaderManager::lineShader->setMat4("projection", MOON_ActiveCamera->projection);

		// vertex array object
		unsigned int VAO;
		glGenVertexArrays(1, &VAO);
		// vertex buffer object
		unsigned int VBO;
		glGenBuffers(1, &VBO);
		// bind buffers
		glBindVertexArray(VAO);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		// point size
		glPointSize(pointSize);
		// copy data
		glBufferData(GL_ARRAY_BUFFER, data.size() * sizeof(float), &data[0], GL_STATIC_DRAW);
		// vertex data format
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (void*)0);
		glEnableVertexAttribArray(0);
		// unbind buffers
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glDrawArrays(GL_POINTS, 0, data.size() / 3);
		glBindVertexArray(0);
		// delete buffer object
		glDeleteVertexArrays(1, &VAO);
		glDeleteBuffers(1, &VBO);
	}

	void Gizmo::DrawPointPrototype(const std::vector<Vector3> &data, const Vector4 &color, const float &pointSize, const Matrix4x4 model) {
		// configure shader
		MOON_ShaderManager::lineShader->use();
		MOON_ShaderManager::lineShader->setVec4("lineColor", color);
		MOON_ShaderManager::lineShader->setMat4("model", model);
		MOON_ShaderManager::lineShader->setMat4("view", MOON_ActiveCamera->view);
		MOON_ShaderManager::lineShader->setMat4("projection", MOON_ActiveCamera->projection);

		// vertex array object
		unsigned int VAO;
		glGenVertexArrays(1, &VAO);
		// vertex buffer object
		unsigned int VBO;
		glGenBuffers(1, &VBO);
		// bind buffers
		glBindVertexArray(VAO);
		glBindBuffer(GL_ARRAY_BUFFER, VBO);
		// point size
		glPointSize(pointSize);
		// copy data
		glBufferData(GL_ARRAY_BUFFER, data.size() * sizeof(Vector3), &data[0], GL_STATIC_DRAW);
		// vertex data format
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vector3), (void*)0);
		glEnableVertexAttribArray(0);
		// unbind buffers
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		glDrawArrays(GL_POINTS, 0, data.size());
		glBindVertexArray(0);
		// delete buffer object
		glDeleteVertexArrays(1, &VAO);
		glDeleteBuffers(1, &VBO);
	}

	void Gizmo::DrawPointPrototype(const unsigned int& VAO, const size_t& size, const Vector4 &color, const float &pointSize, const Matrix4x4 model) {
		// configure shader
		MOON_ShaderManager::lineShader->use();
		MOON_ShaderManager::lineShader->setVec4("lineColor", color);
		MOON_ShaderManager::lineShader->setMat4("model", model);
		MOON_ShaderManager::lineShader->setMat4("view", MOON_ActiveCamera->view);
		MOON_ShaderManager::lineShader->setMat4("projection", MOON_ActiveCamera->projection);

		// bind buffers
		glBindVertexArray(VAO);
		// point size
		glPointSize(pointSize);
		// unbind buffers
		glDrawArrays(GL_POINTS, 0, size);
		glBindVertexArray(0);
	}
#pragma endregion

#pragma region drawers
	void Gizmo::DrawPointDirect(const Vector3 &position, const Vector4 &color, const float &pointSize, const Matrix4x4 model) {
		std::vector<float> data;
		data.push_back(position[0]); data.push_back(position[1]); data.push_back(position[2]);

		DrawPointPrototype(data, color, pointSize, model);
	}

	void Gizmo::DrawPointsDirect(const std::vector<Vector3> &points, const Vector4 &color, const float &pointSize, const Matrix4x4 model) {
		if (points.size() < 1) return;

		DrawPointPrototype(points, color, pointSize, model);
	}

	void Gizmo::DrawPointsDirect(const unsigned int& VAO, const size_t& pointNum, const Vector4 &color, const float &pointSize, const Matrix4x4 model) {
		if (pointNum < 1) return;

		DrawPointPrototype(VAO, pointNum, color, pointSize, model);
	}

	void Gizmo::DrawLineDirect(const Vector3 &start, const Vector3 &end, const Vector4 &color, const float &lineWidth, const Matrix4x4 model) {
		std::vector<float> data;
		data.push_back(start[0]); data.push_back(start[1]); data.push_back(start[2]);
		data.push_back(end[0]); data.push_back(end[1]); data.push_back(end[2]);

		DrawLinePrototype(data, color, lineWidth, false, model);
	}

	void Gizmo::DrawLinesDirect(const std::vector<Vector3> &lines, const Vector4 &color, const float &lineWidth, const bool &isStrip, const Matrix4x4 model, const Shader* overrideShader) {
		if (lines.size() < 1) return;

		DrawLinePrototype(lines, color, lineWidth, isStrip, model, overrideShader);
	}
#pragma endregion

#pragma region drawers_adv
	void Gizmo::DrawPoint(const Vector3 &position, const Vector4 &color, const float &pointSize, const bool& depthTest, const Matrix4x4 model, const bool& drawActiveViewOnly) {
		std::vector<float> data;
		data.push_back(position[0]); data.push_back(position[1]); data.push_back(position[2]);

		if (!drawActiveViewOnly) {
			for (int i = 0; i < 4; i++) {
				if (i != MOON_ActiveView) {
					Graphics::SetDrawTarget((SceneView)i, depthTest);
					DrawPointPrototype(data, color, pointSize, model);
				}
			}
		}
		Graphics::SetDrawTarget(MOON_ActiveView, depthTest);
		DrawPointPrototype(data, color, pointSize, model);

		if (Graphics::process == sys_draw_ui) glBindFramebuffer(GL_FRAMEBUFFER, 0);
		else if (Graphics::process == sys_draw_scene) Graphics::SetDrawTarget(MOON_DrawTarget, depthTest);
	}

	void Gizmo::DrawPoints(const std::vector<Vector3> &points, const Vector4 &color, const float &pointSize, const bool& depthTest, const Matrix4x4 model, const bool& drawActiveViewOnly) {
		if (points.size() < 1) return;

		if (!drawActiveViewOnly) {
			for (int i = 0; i < 4; i++) {
				if (i != MOON_ActiveView) {
					Graphics::SetDrawTarget((SceneView)i, depthTest);
					DrawPointPrototype(points, color, pointSize, model);
				}
			}
		}
		Graphics::SetDrawTarget(MOON_ActiveView, depthTest);
		DrawPointPrototype(points, color, pointSize, model);

		if (Graphics::process == sys_draw_ui) glBindFramebuffer(GL_FRAMEBUFFER, 0);
		else if (Graphics::process == sys_draw_scene) Graphics::SetDrawTarget(MOON_DrawTarget, depthTest);
	}

	void Gizmo::DrawPoints(const unsigned int& VAO, const size_t& pointNum, const Vector4 &color, const float &pointSize, const bool& depthTest, const Matrix4x4 model, const bool& drawActiveViewOnly) {
		if (pointNum < 1) return;

		if (!drawActiveViewOnly) {
			for (int i = 0; i < 4; i++) {
				if (i != MOON_ActiveView) {
					Graphics::SetDrawTarget((SceneView)i, depthTest);
					DrawPointPrototype(VAO, pointNum, color, pointSize, model);
				}
			}
		}
		Graphics::SetDrawTarget(MOON_ActiveView, depthTest);
		DrawPointPrototype(VAO, pointNum, color, pointSize, model);

		if (Graphics::process == sys_draw_ui) glBindFramebuffer(GL_FRAMEBUFFER, 0);
		else if (Graphics::process == sys_draw_scene) Graphics::SetDrawTarget(MOON_DrawTarget, depthTest);
	}

	void Gizmo::DrawLine(const Vector3 &start, const Vector3 &end, const Vector4 &color, const float &lineWidth, const bool& depthTest, const Matrix4x4 model, const bool& drawActiveViewOnly) {
		std::vector<float> data;
		data.push_back(start[0]); data.push_back(start[1]); data.push_back(start[2]);
		data.push_back(end[0]); data.push_back(end[1]); data.push_back(end[2]);

		if (!drawActiveViewOnly) {
			for (int i = 0; i < 4; i++) {
				if (i != MOON_ActiveView) {
					Graphics::SetDrawTarget((SceneView)i, depthTest);
					DrawLinePrototype(data, color, lineWidth, false, model);
				}
			}
		}
		Graphics::SetDrawTarget(MOON_ActiveView, depthTest);
		DrawLinePrototype(data, color, lineWidth, false, model);

		if (Graphics::process == sys_draw_ui) glBindFramebuffer(GL_FRAMEBUFFER, 0);
		else if (Graphics::process == sys_draw_scene) Graphics::SetDrawTarget(MOON_DrawTarget, depthTest);
	}

	void Gizmo::DrawLines(const std::vector<Vector3> &lines, const Vector4 &color, const float &lineWidth, const bool& depthTest, const bool &isStrip, const Matrix4x4 model, const Shader* overrideShader, const bool& drawActiveViewOnly) {
		if (lines.size() < 1) return;

		if (!drawActiveViewOnly) {
			for (int i = 0; i < 4; i++) {
				if (i != MOON_ActiveView) {
					Graphics::SetDrawTarget((SceneView)i, depthTest);
					DrawLinePrototype(lines, color, lineWidth, isStrip, model, overrideShader);
				}
			}
		}
		Graphics::SetDrawTarget(MOON_ActiveView, depthTest);
		DrawLinePrototype(lines, color, lineWidth, isStrip, model, overrideShader);

		if (Graphics::process == sys_draw_ui) glBindFramebuffer(GL_FRAMEBUFFER, 0);
		else if (Graphics::process == sys_draw_scene) Graphics::SetDrawTarget(MOON_DrawTarget, depthTest);
	}
#pragma endregion

#pragma region manipulate_massive
	void Gizmo::CreateVirtualDummy() {
		DeleteVirtualDummy();
		globalVirtualDummy = new Dummy("vDum", MOON_UNSPECIFIEDID);
		//globalVirtualDummy = (Dummy*)MOON_HelperManager::CreateHelper(dummy, "vDum");
	}

	void Gizmo::InitDummyMap() {
		// reset dummy
		globalVirtualDummy->transform.position = MOON_EditTarget->transform.position;
		globalVirtualDummy->transform.rotation = Quaternion::identity();
		globalVirtualDummy->transform.scale = Vector3::ONE();

		globalDummyMap.clear();
		if (MOON_ViewportState == EDIT) {
			if (MOON_EditTarget == nullptr) return;
			if (CheckType(MOON_EditTarget, "Model")) {
				Model* md = dynamic_cast<Model*>(MOON_EditTarget);
				BoundingBox bbox;
				// calculate center
				for (int i = 0; i < md->meshList.size(); i++) {
					auto select = dynamic_cast<HalfMesh*>(md->meshList[i])->selected_verts;
					for (auto& p : select) bbox.join(md->meshList[i]->vertices[p].Position);
				}
				// update dummy position
				globalVirtualDummy->transform.position = md->transform.localToWorldMat.multVec(bbox.center);

				// genereate dummy map
				/*for (int i = 0, base = 0; i < md->meshList.size(); i++) {
					auto select = dynamic_cast<HalfMesh*>(md->meshList[i])->selected_verts;
					for (auto& p : select) {
						globalDummyMap.insert(
							std::pair<unsigned int, Matrix4x4>(
								md->meshList[i]->vertices[p].ID + base, 
								Matrix4x4::TranslateMat(bbox.center - 
									md->transform.localToWorldMat.multVec(
										md->meshList[i]->vertices[p].Position
									)
								)
							)
						);
					}
					base += md->meshList[i]->vertices.size();
				}*/
			} else if (CheckType(MOON_EditTarget, "Shape")) {

			}
		} else {

		}

	}

	void Gizmo::ManipulateMassive(const float maxCamRayLength) {
		if (globalVirtualDummy == nullptr) return;
		if (MOON_SelectionChanged) {
			InitDummyMap();
			MOON_SelectionChanged = false;
		}
		Matrix4x4 dum_worldToLocalMat(globalVirtualDummy->transform.worldToLocalMat);
		Matrix4x4 dum_localToWorldMat(globalVirtualDummy->transform.localToWorldMat);
		auto trans = Manipulate(&globalVirtualDummy->transform, maxCamRayLength);
		if (trans == Matrix4x4::identity()) return;

		if (MOON_ViewportState == EDIT) {
			if (MOON_EditTarget == nullptr) return;
			if (CheckType(MOON_EditTarget, "Model")) {
				Model* md = dynamic_cast<Model*>(MOON_EditTarget);
				// update vertex position
				for (int i = 0; i < md->meshList.size(); i++) {
					auto select = dynamic_cast<HalfMesh*>(md->meshList[i])->selected_verts;
					for (auto& p : select) {
						auto localP = dum_worldToLocalMat.multVec(
							md->transform.localToWorldMat.multVec(
								md->meshList[i]->vertices[p].Position
							)
						);
						md->meshList[i]->vertices[p].Position = md->transform.worldToLocalMat.multVec(
							dum_localToWorldMat.multVec(trans.multVec(localP))
						);
					}
					md->meshList[i]->UpdateMesh();
				}
			} else if (CheckType(MOON_EditTarget, "Shape")) {

			}
		} else {

		}
	}

	void Gizmo::ReleaseDummyMap() {
		globalDummyMap.clear();
	}

	void Gizmo::DeleteVirtualDummy() {
		if (globalVirtualDummy != nullptr) {
			//MOON_HelperManager::DeleteItem(globalVirtualDummy);
			delete globalVirtualDummy;
			globalVirtualDummy = nullptr;
		}
	}
#pragma endregion
}