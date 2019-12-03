#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "Camera.h"
#include "SceneMgr.h"

namespace MOON {
	Matrix4x4 Camera::GetProjectionMatrix() const {
		return Matrix4x4::Perspective(fov, SceneManager::aspect, zNear, zFar);
	}

	Matrix4x4 Camera::GetViewMatrix() const {
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

	// screen pos to world ray fast version (used in renderer)
	Ray Camera::GetRay(float s, float t) const {
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

	// screen pos to world ray
	Ray Camera::GetMouseRay() const {
		// gives mouse pixel in NDC coordinates [-1, 1]
		Vector2 n(MOON_MousePos.x / MOON_WndSize.x * 2 - 1, 
				 (MOON_WndSize.y - MOON_MousePos.y - 1) / MOON_WndSize.y * 2 - 1);

		Vector3 ray_start, ray_end;
		Matrix4x4 view_proj_inverse = (projection * view).inverse();

		ray_start = view_proj_inverse.multVec(Vector3(n.x, n.y, 0.f));
		ray_end = view_proj_inverse.multVec(Vector3(n.x, n.y, 1.f));

		return Ray(ray_start, Vector3::Normalize(ray_end - ray_start));
		//return Ray(ray_start, ray_end - ray_start);
	}

	// screen pos to world pos
	// *NOTE: need to enable depth test
	Vector3 Camera::unProjectMouse() const {
		if (NULL == MOON_SceneCamera) {
			std::cout << "camera failed! failed to un-project mouse" << std::endl;
		} else {
			GLfloat winZ;
			Vector3 screenPos(MOON_MousePos.x, MOON_WndSize.y - MOON_MousePos.y - 1, 0.0f);

			//glBindFramebuffer(GL_FRAMEBUFFER, fbo);
			glReadPixels(screenPos.x, screenPos.y, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &winZ);
			//glBindFramebuffer(GL_FRAMEBUFFER, 0);
			screenPos.z = winZ;

			Vector4 viewport = Vector4(0.0f, 0.0f, MOON_WndSize.x, MOON_WndSize.y);
			Vector3 worldPos = Matrix4x4::UnProject(screenPos, MOON_CurrentCamera->view, MOON_CurrentCamera->projection, viewport);

			return worldPos;
		}
	}
}