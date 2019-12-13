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
		Vector3 offset = Right * mouseOffset.x + Up * mouseOffset.y;
		transform.position -= offset;
		tarPos -= offset;
		UpdateCameraVectors();
	}

	void Camera::ZoomCamera(Vector2 &mouseOffset) {
		if (fov >= 5.0f && fov <= 60.0f) fov -= mouseOffset.y * MouseSensitivity;
		else if (fov < 5.0f) fov = 5.0f;
		else if (fov > 60.0f) fov = 60.0f;

		UpdateCameraVectors();
	}

	void Camera::RotateCamera(Vector2 mouseOffset, bool constrainPitch) {
		mouseOffset *= MouseSensitivity * 8.0f;
		transform.position = MoonMath::RotateAround(transform.position, tarPos, Up, -Deg2Rad * mouseOffset.x);
		transform.position = MoonMath::RotateAround(transform.position, tarPos, Right, Deg2Rad * mouseOffset.y);

		// Update Front, Right and Up Vectors using the updated Euler angles
		UpdateCameraVectors();
	}

	void Camera::PushCamera(Vector2 &mouseScrollOffset) {
		transform.position += Front * mouseScrollOffset.y;

		UpdateCameraVectors();
	}

	// screen pos to world ray fast version (used in renderer)
	Ray Camera::GetRay(float s, float t, float aspect) const {
		s = (s - 0.5f) / 2.0f * aspect + 0.5f;
		Vector3 rd = lens_radius * MoonMath::RandomInUnitDisk();
		Vector3 offset = Right * rd.x + Up * rd.y;
		return Ray(transform.position + offset, lower_left_corner + s * horizontal + t * vertical - transform.position - offset);
	}

	void Camera::InitRenderCamera() {
		float aspect = 2.0f;
		float theta = fov * Deg2Rad;
		float half_height = tan(theta / aspect);
		float half_width = aspect * half_height;

		float focus_dist = Front.magnitude();
		lower_left_corner = transform.position - focus_dist * (half_width * Right + half_height * Up - Front);

		horizontal = aspect * half_width * focus_dist * Right;
		vertical = aspect * half_height * focus_dist * Up;
	}

	void Camera::UpdateCameraVectors() {
		// Calculate the new Front vector
		Front = tarPos - transform.position;
		Front.normalize();

		// Also re-calculate the Right and Up vector
		// Normalize the vectors, because their length gets closer to 0 the more 
		// you look up or down which results in slower movement.
		Right = Vector3::Normalize(Vector3::Cross(Front, WorldUp));
		Up = Vector3::Normalize(Vector3::Cross(Right, Front));

		UpdateMatrix();
	}

	void Camera::CatchTarget(const Model* target) {
		tarPos = target ? target->transform.position : Vector3::ZERO();

		transform.position = tarPos - Front * 
							(target ? (target->bbox_world.max - 
							 target->bbox_world.min).magnitude() * 2.0f : 20.0f);
		UpdateCameraVectors();
	}

	Vector3 Camera::WorldToScreenPos(const Vector3& worldPos) const {
		Matrix4x4 view_proj_inverse = projection * view;
		return view_proj_inverse.multVec(worldPos);
	}

	// screen pos to world ray
	Ray Camera::GetMouseRay() const {
		Vector2 n(MOON_MousePos.x / MOON_WndSize.x * 2 - 1, (MOON_WndSize.y - MOON_MousePos.y - 1) / MOON_WndSize.y * 2 - 1);

		Matrix4x4 view_proj_inverse = (projection * view).inverse();
		Vector3 ray_end = view_proj_inverse.multVec(Vector3(n.x, n.y, 1.f));

		return Ray(transform.position, Vector3::Normalize(ray_end - view_proj_inverse.multVec(Vector3(n.x, n.y, 0.f))));
	}

	Ray Camera::GetMouseRayAccurate() const {
		Vector2 n = MOON_MousePosNormalized;

		Vector3 ray_start, ray_end;
		Matrix4x4 view_proj_inverse = (projection * view).inverse();

		ray_start = view_proj_inverse.multVec(Vector3(n.x, n.y, 0.f));
		ray_end = view_proj_inverse.multVec(Vector3(n.x, n.y, 1.f));

		return Ray(ray_start, Vector3::Normalize(ray_end - ray_start));
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