#pragma once
#include <cmath>
#include <vector>
#include <iostream>

#include "Renderer.h"
#include "Vector3.h"
#include "Matrix4x4.h"
#include "MathUtils.h"
#include "ObjectBase.h"

namespace moon {
	// An Euler Angles Version
	class Camera : public MObject {
	private:
		Vector3 lower_left_corner;
		Vector3 horizontal;
		Vector3 vertical;
		Vector3 WorldUp;

	public:
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
		float MouseSensitivity;
		float fov;

		// Constructor with vectors
		Camera(const std::string &name, const Vector3 &position = Vector3::ZERO(), const float aperture = 0.0f, const int id = MOON_AUTOID) :
			Front(Vector3(0.0f, 0.0f, -1.0f)), MouseSensitivity(0.025f), fov(45.0f), lens_radius(aperture / 2), MObject(name, id) {
			transform.position = position;
			WorldUp = Vector3(0.0f, 1.0f, 0.0f);
			Yaw = -90.0f;
			Pitch = 0.0f;

			updateCameraVectors();
		}

		Camera(const Vector3 &position = Vector3::ZERO(), const float aperture = 0.0f, const int id = MOON_AUTOID) :
			Front(Vector3(0.0f, 0.0f, -1.0f)), MouseSensitivity(0.025f), fov(45.0f), lens_radius(aperture / 2), MObject(id) {
			transform.position = position;
			WorldUp = Vector3(0.0f, 1.0f, 0.0f);
			Yaw = -90.0f;
			Pitch = 0.0f;
			updateCameraVectors();
		}

		// Constructor with scalar values
		Camera(float posX, float posY, float posZ, float upX, float upY, float upZ, float yaw, float pitch, const float aperture = 0.0f) :
				Front(Vector3(0.0f, 0.0f, -1.0f)), MouseSensitivity(0.025f), fov(45.0f), lens_radius(aperture / 2) {
			transform.position = Vector3(posX, posY, posZ);
			WorldUp = Vector3(upX, upY, upZ);
			Yaw = yaw;
			Pitch = pitch;
			updateCameraVectors();
		}

		// Returns the view matrix calculated using Euler Angles and the LookAt Matrix
		Matrix4x4 GetViewMatrix() {
			return Matrix4x4::LookAt(transform.position, transform.position + Front, Up);
		}

		void PanCamera(Vector2 mouseOffset) {
			mouseOffset *= MouseSensitivity;
			transform.position -= Right * mouseOffset.x + Up * mouseOffset.y;
			updateCameraVectors();
		}

		void ZoomCamera(Vector2 &mouseOffset) {
			if (fov >= 5.0f && fov <= 60.0f) fov -= mouseOffset.y * MouseSensitivity;
			else if (fov < 5.0f) fov = 5.0f;
			else if (fov > 60.0f) fov = 60.0f;

			updateCameraVectors();
		}

		void RotateCamera(Vector2 mouseOffset, bool constrainPitch = true) {
			mouseOffset *= MouseSensitivity * 4.0f;

			Yaw += mouseOffset.x;
			Pitch += mouseOffset.y;

			mouseOffset *= 0.32f;
			transform.position -= Right * mouseOffset.x + Up * mouseOffset.y;

			// Make sure that when pitch is out of bounds, screen doesn't get flipped
			if (constrainPitch) {
				if (Pitch > 89.0f) Pitch = 89.0f;
				if (Pitch < -89.0f) Pitch = -89.0f;
			}

			// Update Front, Right and Up Vectors using the updated Euler angles
			updateCameraVectors();
		}

		void PushCamera(Vector2 &mouseScrollOffset) {
			transform.position += Front * mouseScrollOffset.y;

			updateCameraVectors();
		}

		Ray GetRay(float s, float t) {
			Vector3 rd = lens_radius * MoonMath::RandomInUnitDisk();
			Vector3 offset = Right * rd.x + Up * rd.y;
			return Ray(transform.position + offset,
				lower_left_corner + s * horizontal +
				t * vertical - transform.position - offset);
		}

	private:
		// Calculates the front vector from the Camera's (updated) Euler Angles
		void updateCameraVectors() {
			// Calculate the new Front vector
			Front.x = cos(Deg2Rad * Yaw) * cos(Deg2Rad * Pitch);
			Front.y = sin(Deg2Rad * Pitch);
			Front.z = sin(Deg2Rad * Yaw) * cos(Deg2Rad * Pitch);
			Front.normalize();

			// Also re-calculate the Right and Up vector
			// Normalize the vectors, because their length gets closer to 0 the more 
			// you look up or down which results in slower movement.
			Right = Vector3::Normalize(Vector3::Cross(Front, WorldUp));
			Up = Vector3::Normalize(Vector3::Cross(Right, Front));

			float theta = fov * PI / 180.0;
			float aspect = Renderer::OUTPUT_SIZE.x / Renderer::OUTPUT_SIZE.y;
			float half_height = tan(theta / aspect);
			float half_width = aspect * half_height;

			Vector3 w = -Front; // camera position - target position
			float focus_dist = w.magnitude();
			Vector3 u = Right;
			Vector3 v = Up;
			lower_left_corner = transform.position - focus_dist * (half_width * u + half_height * v + w);

			horizontal = aspect * half_width * focus_dist * u;
			vertical = aspect * half_height * focus_dist * v;
		}
	};
}