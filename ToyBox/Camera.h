#pragma once
#include <cmath>
#include <vector>
#include <iostream>

#include "Renderer.h"
#include "Vector3.h"
#include "Matrix4x4.h"
#include "MathUtils.h"
#include "ObjectBase.h"

namespace MOON {
	// An Euler Angles Version
	class Camera : public MObject {
	private:
		Vector3 lower_left_corner;
		Vector3 horizontal;
		Vector3 vertical;
		Vector3 WorldUp;

	public:
		// global param
		static float MouseSensitivity;

		// Target object
		MObject* target;

		// Camera attributes
		Vector3 Front;
		Vector3 Right;
		Vector3 Up;

		float lens_radius;

		// Euler angles
		float Yaw;
		float Pitch;
		//float Roll;

		// Camera options
		float fov;
		float zNear;
		float zFar;

		// Camera matrix
		bool isortho;
		Matrix4x4 view;
		Matrix4x4 projection;

		// Constructor with vectors
		Camera(const std::string &name, const Vector3 &position = Vector3::ZERO(), const float aperture = 0.0f, const int id = MOON_AUTOID) :
			Front(Vector3(0.0f, 0.0f, -1.0f)), fov(45.0f), lens_radius(aperture / 2), MObject(name, id) {
			transform.position = position;
			WorldUp = Vector3(0.0f, 1.0f, 0.0f);
			Yaw = -90.0f;
			Pitch = 0.0f;
			zNear = 0.1f;
			zFar = 100.0f;

			updateCameraVectors();
		}

		Camera(const Vector3 &position = Vector3::ZERO(), const float aperture = 0.0f, const int id = MOON_AUTOID) :
			Front(Vector3(0.0f, 0.0f, -1.0f)), fov(45.0f), lens_radius(aperture / 2), MObject(id) {
			transform.position = position;
			WorldUp = Vector3(0.0f, 1.0f, 0.0f);
			Yaw = -90.0f;
			Pitch = 0.0f;
			zNear = 0.1f;
			zFar = 100.0f;

			updateCameraVectors();
		}

		// Constructor with scalar values
		Camera(float posX, float posY, float posZ, float upX, float upY, float upZ, float yaw, float pitch, const float aperture = 0.0f) :
				Front(Vector3(0.0f, 0.0f, -1.0f)), fov(45.0f), lens_radius(aperture / 2) {
			transform.position = Vector3(posX, posY, posZ);
			WorldUp = Vector3(upX, upY, upZ);
			Yaw = yaw;
			Pitch = pitch;
			zNear = 0.1f;
			zFar = 100.0f;

			updateCameraVectors();
		}

		void InitRenderCamera();
		void UpdateMatrix();
		// Returns the view matrix calculated using Euler Angles and the LookAt Matrix
		Matrix4x4 GetViewMatrix() const;
		Matrix4x4 GetProjectionMatrix() const;

		Ray GetRay(float s, float t) const;
		Ray GetMouseRay() const;
		Vector3 unProjectMouse() const;

		void PanCamera(Vector2 mouseOffset);
		void ZoomCamera(Vector2 &mouseOffset);
		void RotateCamera(Vector2 mouseOffset, bool constrainPitch = true);
		void PushCamera(Vector2 &mouseScrollOffset);

	private:
		// Calculates the front vector from the Camera's (updated) Euler Angles
		void updateCameraVectors();
	};
}