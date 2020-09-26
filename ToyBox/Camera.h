#pragma once
#include <cmath>
#include <vector>
#include <iostream>

#include "Renderer.h"
#include "Vector3.h"
#include "Matrix4x4.h"
#include "MathUtils.h"
#include "ObjectBase.h"
#include "Model.h"

namespace MOON {
	// An Euler Angles Version
	class Camera : public MObject {
	private:
		Vector3 lower_left_corner;
		Vector3 horizontal;
		Vector3 vertical;

	public:
		// global param
		static float MouseSensitivity;

		// Target object
		MObject* target;
		Vector3 tarPos;

		float lens_radius;

		// Camera options
		float fov;
		float zNear, zFar;
		float width, height;

		// Camera matrix
		bool isortho;
		Matrix4x4 view;
		Matrix4x4 projection;

		// Constructor with vectors
		Camera(const std::string &name, const Vector3 &position = Vector3::ZERO(), 
			const Vector3 &lookDir = Vector3::WORLD(FORWARD), const bool& isortho = false,
			const float aperture = 0.0f, const int id = MOON_AUTOID) :
			fov(45.0f), lens_radius(aperture / 2), MObject(name, id), isortho(isortho) {
			transform.position = position;
			transform.rotation = Quaternion::Rotate(Vector3::WORLD(FORWARD), lookDir);
			zNear = 0.1f; zFar = 1000.0f;
			width = 1.0f; height = 1.0f;
			tarPos = Vector3::ZERO();

			UpdateMatrix();
		}

		Camera(const Vector3 &position = Vector3::ZERO(), const Vector3 &Front = Vector3(0.0f, 0.0f, -1.0f),
			const bool& isortho = false, const float aperture = 0.0f, const int id = MOON_AUTOID) :
			fov(45.0f), lens_radius(aperture / 2), MObject(id), isortho(isortho) {
			transform.position = position;
			zNear = 0.1f; zFar = 1000.0f;
			width = 1.0f; height = 1.0f;
			tarPos = Vector3::ZERO();

			UpdateMatrix();
		}

		void InitRenderCamera();
		void UpdateMatrix();
		// Returns the view matrix calculated using Euler Angles and the LookAt Matrix
		Matrix4x4 GetViewMatrix();
		Matrix4x4 GetProjectionMatrix();

		Ray GetRay(float s, float t, float aspect);
		// rough way, treat camera position as start point of the ray
		Ray GetMouseRay() const;
		// accurate solution
		Ray GetMouseRayAccurate() const;
		Vector3 unProjectMouse() const;

		Vector3 WorldToScreenPos(const Vector3& worldPos) const;

		void CatchTarget(const Model* target);
		void PanCamera(Vector2 mouseOffset);
		void ZoomCamera(Vector2 &mouseOffset);
		void RotateCamera(Vector2 mouseOffset, bool constrainPitch = true);
		void PushCamera(Vector2 &mouseScrollOffset);
	};
}