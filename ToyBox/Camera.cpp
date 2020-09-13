#include <iostream>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "Camera.h"
#include "SceneMgr.h"

namespace MOON {
	Matrix4x4 Camera::GetProjectionMatrix() {
		if (isortho) return Matrix4x4::Orthographic(width, height, zNear, zFar);
		else return Matrix4x4::Perspective(fov, SceneManager::aspect, zNear, zFar);
	}

	Matrix4x4 Camera::GetViewMatrix() {
		return Matrix4x4::LookAt(
			transform.position, 
			transform.position + 
			transform.GetLocalAxis(FORWARD), 
			transform.GetLocalAxis(UP)
		);
	}

	void Camera::UpdateMatrix() {
		view = GetViewMatrix();
		projection = GetProjectionMatrix();
	}

	void Camera::PanCamera(Vector2 mouseOffset) {
		mouseOffset *= MouseSensitivity;
		Vector3 offset = transform.GetLocalAxis(RIGHT) * mouseOffset.x + 
			transform.GetLocalAxis(UP) * mouseOffset.y;
		transform.position -= offset;
		tarPos -= offset;
		UpdateMatrix();
	}

	void Camera::ZoomCamera(Vector2 &mouseOffset) {
		if (fov >= 5.0f && fov <= 60.0f) fov -= mouseOffset.y * MouseSensitivity;
		else if (fov < 5.0f) fov = 5.0f;
		else if (fov > 60.0f) fov = 60.0f;

		UpdateMatrix();
	}

	void Camera::RotateCamera(Vector2 mouseOffset, bool constrainPitch) {
		mouseOffset *= MouseSensitivity * 8.0f;
		auto delta = transform.position - tarPos;
		auto pitch = Quaternion::Rotate(transform.GetLocalAxis(RIGHT), Deg2Rad * mouseOffset.y);
		// Roll can creep in from combinations of yaw and pitch 
		// if you're rotating around the camera's local axes each time
		// https://gamedev.stackexchange.com/questions/103242/why-is-the-camera-tilting-around-the-z-axis-when-i-only-specified-x-and-y/103243#103243
		// One way to fix this is to always pitch along your local x axis, 
		// but always yaw around the world y axis (if you always want the camera "up" vector facing up)
		auto yaw = Quaternion::Rotate(Vector3::WORLD(UP), -Deg2Rad * mouseOffset.x);

		transform.position = tarPos + yaw * pitch * delta;
		transform.Rotate(yaw * pitch);

		UpdateMatrix();
	}

	void Camera::PushCamera(Vector2 &mouseScrollOffset) {
		if (isortho) {
			auto ratio = width / height;
			auto delta = mouseScrollOffset.y * MouseSensitivity * 10.0f;
			width -= ratio * delta; height -= delta;
		} else transform.position += transform.GetLocalAxis(FORWARD) * mouseScrollOffset.y;

		UpdateMatrix();
	}

	// screen pos to world ray fast version (used in renderer)
	Ray Camera::GetRay(float s, float t, float aspect) {
		s = (s - 0.5f) / 2.0f * aspect + 0.5f;
		Vector3 rd = lens_radius * MoonMath::RandomInUnitDisk();
		Vector3 offset = transform.GetLocalAxis(RIGHT) * rd.x + transform.GetLocalAxis(UP) * rd.y;
		auto pos = transform.position;
		return Ray(pos + offset, lower_left_corner + s * horizontal + t * vertical - pos - offset);
	}

	void Camera::InitRenderCamera() {
		float aspect = 2.0f;
		float theta = fov * Deg2Rad;
		float half_height = tan(theta / aspect);
		float half_width = aspect * half_height;

		auto forward = transform.GetLocalAxis(FORWARD);
		auto right = transform.GetLocalAxis(RIGHT);
		auto up = transform.GetLocalAxis(UP);

		float focus_dist = forward.magnitude();
		lower_left_corner = transform.position - focus_dist * (half_width * right + half_height * up - forward);

		horizontal = aspect * half_width * focus_dist * right;
		vertical = aspect * half_height * focus_dist * up;
	}

	void Camera::CatchTarget(const Model* target) {
		tarPos = target ? const_cast<Transform&>(target->transform).position : Vector3::ZERO();

		transform.position = tarPos - transform.GetLocalAxis(FORWARD) * 
			(target ? (target->bbox_world.max - target->bbox_world.min).magnitude() * 2.0f : 20.0f);
		UpdateMatrix();
	}

	Vector3 Camera::WorldToScreenPos(const Vector3& worldPos) const {
		Matrix4x4 view_proj_inverse = projection * view;
		return view_proj_inverse.multVec(worldPos);
	}

	// screen pos to world ray
	Ray Camera::GetMouseRay() const {
		Vector2 n(MOON_MousePos.x / MOON_ScrSize.x * 2 - 1, (MOON_ScrSize.y - MOON_MousePos.y - 1) / MOON_ScrSize.y * 2 - 1);

		Matrix4x4 view_proj_inverse = (projection * view).inverse();
		Vector3 ray_end = view_proj_inverse.multVec(Vector3(n.x, n.y, 1.f));

		return Ray(const_cast<Transform&>(transform).position, Vector3::Normalize(ray_end - view_proj_inverse.multVec(Vector3(n.x, n.y, 0.f))));
	}

	Ray Camera::GetMouseRayAccurate() const {
		Vector2 n = MOON_MousePosNormalized;

		Vector3 ray_start, ray_end;
		Matrix4x4 view_proj_inverse = (projection * view).inverse();

		ray_start = view_proj_inverse.multVec(Vector3(n.x, n.y, -1.f));
		ray_end = view_proj_inverse.multVec(Vector3(n.x, n.y, 1.f));

		return Ray(ray_start, Vector3::Normalize(ray_end - ray_start));
	}

	// screen pos to world pos
	// *NOTE: need to enable depth test
	Vector3 Camera::unProjectMouse() const {
		GLfloat winZ;
		Vector3 screenPos(MOON_MousePos.x, MOON_ScrSize.y - MOON_MousePos.y - 1, 0.0f);

		//glBindFramebuffer(GL_FRAMEBUFFER, fbo);
		glReadPixels(screenPos.x, screenPos.y, 1, 1, GL_DEPTH_COMPONENT, GL_FLOAT, &winZ);
		//glBindFramebuffer(GL_FRAMEBUFFER, 0);
		screenPos.z = winZ;

		Vector4 viewport = Vector4(0.0f, 0.0f, MOON_ScrSize.x, MOON_ScrSize.y);
		Vector3 worldPos = Matrix4x4::UnProject(screenPos, MOON_ActiveCamera->view, MOON_ActiveCamera->projection, viewport);

		return worldPos;
	}
}