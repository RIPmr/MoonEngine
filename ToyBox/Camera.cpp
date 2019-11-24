#include "Camera.h"
#include "SceneMgr.h"

namespace MOON {
	Matrix4x4 Camera::GetProjectionMatrix() {
		return Matrix4x4::Perspective(fov, SceneManager::SCR_SIZE.x / SceneManager::SCR_SIZE.y, zNear, zFar);
	}

	Matrix4x4 Camera::GetViewMatrix() {
		return Matrix4x4::LookAt(transform.position, transform.position + Front, Up);
	}

	void Camera::UpdateMatrix() {
		view = GetViewMatrix();
		projection = GetProjectionMatrix();
	}

	void Camera::PanCamera(Vector2 mouseOffset) {
		mouseOffset *= MouseSensitivity;
		transform.position -= Right * mouseOffset.x + Up * mouseOffset.y;
		updateCameraVectors();
	}

	void Camera::ZoomCamera(Vector2 &mouseOffset) {
		if (fov >= 5.0f && fov <= 60.0f) fov -= mouseOffset.y * MouseSensitivity;
		else if (fov < 5.0f) fov = 5.0f;
		else if (fov > 60.0f) fov = 60.0f;

		updateCameraVectors();
	}

	void Camera::RotateCamera(Vector2 mouseOffset, bool constrainPitch) {
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

	void Camera::PushCamera(Vector2 &mouseScrollOffset) {
		transform.position += Front * mouseScrollOffset.y;

		updateCameraVectors();
	}

	Ray Camera::GetRay(float s, float t) {
		Vector3 rd = lens_radius * MoonMath::RandomInUnitDisk();
		Vector3 offset = Right * rd.x + Up * rd.y;
		return Ray(transform.position + offset,
			lower_left_corner + s * horizontal +
			t * vertical - transform.position - offset);
	}

	void Camera::InitRenderCamera() {
		float theta = fov * Deg2Rad;
		float half_height = tan(theta / Renderer::aspect);
		float half_width = Renderer::aspect * half_height;

		float focus_dist = Front.magnitude();
		lower_left_corner = transform.position - focus_dist * (half_width * Right + half_height * Up - Front);

		horizontal = Renderer::aspect * half_width * focus_dist * Right;
		vertical = Renderer::aspect * half_height * focus_dist * Up;
	}

	void Camera::updateCameraVectors() {
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

		UpdateMatrix();
	}
}