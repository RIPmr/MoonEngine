#include "Gizmo.h"
#include "SceneMgr.h"

namespace MOON {

	Vector3 Gizmo::Translate(const Ray& ray, const Direction& dir, Transform *trans, Vector3& cAxisPoint_O, bool& xActive, float maxCamRayLength) {
		float		scrDist = Vector3::Distance(ray.pos, trans->position) / 12.0f;
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
		DrawPrototype(Matrix4x4::Translate(model, trans->position), ColorByDir(dir, !isActive, xActive));

		// get delta vector
		if (isActive && xActive) deltaVec = Vector3::Projection(cAxisPoint - cAxisPoint_O, axis.dir);
		cAxisPoint_O.setValue(cAxisPoint);

		return deltaVec;
	}

	Quaternion Gizmo::Rotate(const Ray& ray, const Direction& dir, Matrix4x4& model, Transform *trans, Vector3& cAxisPoint_O, bool& xActive) {
		float		scrDist = Vector3::Distance(ray.pos, trans->position) / 15.0f;
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
		float		scrDist = Vector3::Distance(ray.pos, trans->position) / 12.0f;
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
		DrawPrototype(Matrix4x4::Translate(model, trans->position), ColorByDir(dir, !isActive, xActive));

		// get delta vector
		if (isActive && xActive) {
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

	Quaternion Gizmo::Rotate_SS(const Ray& ray, const Direction& dir, Matrix4x4& model, Transform *trans, Vector3& cAxisPoint_O, Vector2& screenPos_O, bool& xActive) {
		float		scrDist = Vector3::Distance(ray.pos, trans->position) / 15.0f;
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
		Vector2 pivotInScreen = MOON_CurrentCamera->WorldToScreenPos(axis.pos);
		int rotDir = Vector2::DirectionSign(screenPos_O - pivotInScreen, n - pivotInScreen);
		if (isActive && xActive) deltaRot = Quaternion::Rotate(axis.dir, (n - screenPos_O).magnitude() * 30.0f * rotDir);

		return deltaRot;
	}

	void Gizmo::Manipulate(void* transform, const float maxCamRayLength) {
		static Vector2 screenPos;
		static Vector3 cAxisPoint_X, cAxisPoint_Y, cAxisPoint_Z;
		static bool	   xActive, yActive, zActive;

		Transform	   *trans = (Transform*)transform;
		Quaternion	   deltaRot;
		Vector3		   deltaVec;
		Vector3		   deltaSca;

		Ray ray = MOON_CurrentCamera->GetMouseRayAccurate();

		hoverGizmo = xActive | yActive | zActive;

		auto translate = [&]() {
			deltaVec += Translate(ray, Direction::UP, trans, cAxisPoint_Y, yActive, maxCamRayLength);
			deltaVec += Translate(ray, Direction::LEFT, trans, cAxisPoint_X, xActive, maxCamRayLength);
			deltaVec += Translate(ray, Direction::FORWARD, trans, cAxisPoint_Z, zActive, maxCamRayLength);
		};

		auto rotate = [&](float threshold) {
			Matrix4x4 modelX, modelY, modelZ;
			deltaRot = Rotate_SS(ray, Direction::UP, modelY, trans, cAxisPoint_Y, screenPos, yActive);
			deltaRot = Rotate_SS(ray, Direction::LEFT, modelX, trans, cAxisPoint_X, screenPos, xActive) * deltaRot;
			deltaRot = Rotate_SS(ray, Direction::FORWARD, modelZ, trans, cAxisPoint_Z, screenPos, zActive) * deltaRot;

			screenPos.setValue(MOON_MousePosNormalized);
			float dY = yActive ? cAxisPoint_Y.fastDistance(ray.pos) : INFINITY;
			float dX = xActive ? cAxisPoint_X.fastDistance(ray.pos) : INFINITY;
			float dZ = zActive ? cAxisPoint_Z.fastDistance(ray.pos) : INFINITY;

			yActive &= dY < dX && dY < dZ; xActive &= dX < dY && dX < dZ; zActive &= dZ < dX && dZ < dY;
			DrawPrototype(Matrix4x4::Translate(modelY, trans->position), ColorByDir(Direction::UP, !isActive, yActive));
			DrawPrototype(Matrix4x4::Translate(modelX, trans->position), ColorByDir(Direction::LEFT, !isActive, xActive));
			DrawPrototype(Matrix4x4::Translate(modelZ, trans->position), ColorByDir(Direction::FORWARD, !isActive, zActive));
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

		if (isActive && MOON_InputManager::mouse_left_hold) {
			if (deltaVec.magnitude() > 0) trans->Translate(deltaVec);
			if (deltaRot.magnitude() > 0) trans->Rotate(deltaRot);
			if (deltaSca.magnitude() > 0) trans->Scale(trans->scale + deltaSca);
		}
	}

	void Gizmo::DrawPrototype(const Matrix4x4& mat, const Vector4& color) {
		if (gizmoMode == GizmoMode::none) return;
		// TODO : link line
		if (gizmoMode == GizmoMode::link) return;

		// get data
		std::vector<float> &data = (gizmoMode == GizmoMode::rotate ? circle : translate);
		int drawSize = (gizmoMode == GizmoMode::rotate ? data.size() / 3 : 2);

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

}