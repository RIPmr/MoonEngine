#pragma once
#include <vector>
#include <iostream>

#include "Vector3.h"
#include "Matrix4x4.h"
#include "MathUtils.h"

namespace moon {
	// Defines several possible options for camera movement. 
	// Used as abstraction to stay away from window-system specific input methods
	enum Camera_Movement {
		FORWARD,
		BACKWARD,
		LEFT,
		RIGHT
	};

	// Default camera values
	const float YAW = -90.0f;
	const float PITCH = 0.0f;
	const float SPEED = 2.5f;
	const float SENSITIVITY = 0.1f;
	const float ZOOM = 45.0f;

	// An abstract camera class that processes input and calculates the corresponding Euler Angles, 
	// Vectors and Matrices for use in OpenGL
	class Camera {
	public:
		// Camera Attributes
		Vector3 Position;
		Vector3 Front;
		Vector3 Up;
		Vector3 Right;
		Vector3 WorldUp;
		// Euler Angles
		float Yaw;
		float Pitch;
		// Camera options
		float MovementSpeed;
		float MouseSensitivity;
		float Zoom;

		// Constructor with vectors
		Camera(Vector3 position = Vector3(0.0f, 0.0f, 0.0f), Vector3 up = Vector3(0.0f, 1.0f, 0.0f), float yaw = YAW, float pitch = PITCH) : Front(Vector3(0.0f, 0.0f, -1.0f)), MovementSpeed(SPEED), MouseSensitivity(SENSITIVITY), Zoom(ZOOM) {
			Position = position;
			WorldUp = up;
			Yaw = yaw;
			Pitch = pitch;
			updateCameraVectors();
		}

		// Constructor with scalar values
		Camera(float posX, float posY, float posZ, float upX, float upY, float upZ, float yaw, float pitch) : Front(Vector3(0.0f, 0.0f, -1.0f)), MovementSpeed(SPEED), MouseSensitivity(SENSITIVITY), Zoom(ZOOM) {
			Position = Vector3(posX, posY, posZ);
			WorldUp = Vector3(upX, upY, upZ);
			Yaw = yaw;
			Pitch = pitch;
			updateCameraVectors();
		}

		// Returns the view matrix calculated using Euler Angles and the LookAt Matrix
		Matrix4x4 GetViewMatrix() {
			return Matrix4x4::LookAt(Position, Position + Front, Up);
		}

		// Processes input received from any keyboard-like input system.
		// Accepts input parameter in the form of camera defined ENUM 
		// (to abstract it from windowing systems)
		void ProcessKeyboard(Camera_Movement direction, float deltaTime) {
			float velocity = MovementSpeed * deltaTime;
			if (direction == FORWARD)
				Position += Front * velocity;
			if (direction == BACKWARD)
				Position -= Front * velocity;
			if (direction == LEFT)
				Position -= Right * velocity;
			if (direction == RIGHT)
				Position += Right * velocity;
		}

		// Processes input received from a mouse input system. 
		// Expects the offset value in both the x and y direction.
		void ProcessMouseMovement(float xoffset, float yoffset, bool constrainPitch = true) {
			xoffset *= MouseSensitivity;
			yoffset *= MouseSensitivity;

			Yaw += xoffset;
			Pitch += yoffset;

			// Make sure that when pitch is out of bounds, screen doesn't get flipped
			if (constrainPitch) {
				if (Pitch > 89.0f)
					Pitch = 89.0f;
				if (Pitch < -89.0f)
					Pitch = -89.0f;
			}

			// Update Front, Right and Up Vectors using the updated Euler angles
			updateCameraVectors();
		}

		// Processes input received from a mouse scroll-wheel event. 
		// Only requires input on the vertical wheel-axis
		void ProcessMouseScroll(float yoffset) {
			if (Zoom >= 1.0f && Zoom <= 45.0f)
				Zoom -= yoffset;
			if (Zoom <= 1.0f)
				Zoom = 1.0f;
			if (Zoom >= 45.0f)
				Zoom = 45.0f;
		}

	private:
		// Calculates the front vector from the Camera's (updated) Euler Angles
		void updateCameraVectors() {
			// Calculate the new Front vector
			Vector3 front;
			front.x = cos(Deg2Rad * Yaw) * cos(Deg2Rad * Pitch);
			front.y = sin(Deg2Rad * Pitch);
			front.z = sin(Deg2Rad * Yaw) * cos(Deg2Rad * Pitch);
			Front = Vector3::Normalize(front);

			// Also re-calculate the Right and Up vector
			// Normalize the vectors, because their length gets closer to 0 the more 
			// you look up or down which results in slower movement.
			Right = Vector3::Normalize(Vector3::Cross(Front, WorldUp));
			Up = Vector3::Normalize(Vector3::Cross(Right, Front));
		}
	};
}