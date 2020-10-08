#include <iostream>
#include <imgui.h>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "Camera.h"
#include "Gizmo.h"
#include "Strutil.h"
#include "ButtonEx.h"
#include "SceneMgr.h"
#include "Renderer.h"

namespace MOON {
	std::vector<Vector3> Camera::cameraShape = std::vector<Vector3>{
		Vector3(-0.1f,  0.1f, -0.3f), Vector3(-0.1f, -0.1f, -0.3f),
		Vector3(-0.1f, -0.1f, -0.3f), Vector3(0.1f, -0.1f, -0.3f),
		Vector3(0.1f, -0.1f, -0.3f), Vector3(0.1f,  0.1f, -0.3f),
		Vector3(0.1f,  0.1f, -0.3f), Vector3(-0.1f,  0.1f, -0.3f),

		Vector3(-0.1f,  0.1f, -0.1f), Vector3(-0.1f, -0.1f, -0.1f),
		Vector3(-0.1f, -0.1f, -0.1f), Vector3(0.1f, -0.1f, -0.1f),
		Vector3(0.1f, -0.1f, -0.1f), Vector3(0.1f,  0.1f, -0.1f),
		Vector3(0.1f,  0.1f, -0.1f), Vector3(-0.1f,  0.1f, -0.1f),

		Vector3(-0.15f,  0.15f, 0), Vector3(-0.15f, -0.15f, 0),
		Vector3(-0.15f, -0.15f, 0), Vector3(0.15f, -0.15f, 0),
		Vector3(0.15f, -0.15f, 0), Vector3(0.15f,  0.15f, 0),
		Vector3(0.15f,  0.15f, 0), Vector3(-0.15f,  0.15f, 0),

		// connect
		Vector3(-0.1f, 0.1f, -0.3f), Vector3(-0.1f, 0.1f, -0.1f),
		Vector3(-0.1f, 0.1f, -0.1f), Vector3(-0.15f, 0.15f, 0),

		Vector3(-0.1f, -0.1f, -0.3f), Vector3(-0.1f, -0.1f, -0.1f),
		Vector3(-0.1f, -0.1f, -0.1f), Vector3(-0.15f, -0.15f, 0),

		Vector3(0.1f, -0.1f, -0.3f), Vector3(0.1f, -0.1f, -0.1f),
		Vector3(0.1f, -0.1f, -0.1f), Vector3(0.15f, -0.15f, 0),

		Vector3(0.1f, 0.1f, -0.3f), Vector3(0.1f, 0.1f, -0.1f),
		Vector3(0.1f, 0.1f, -0.1f), Vector3(0.15f, 0.15f, 0)
	};

	Matrix4x4 Camera::GetProjectionMatrix() {
		if (isortho) return Matrix4x4::Orthographic(width, height, zNear, zFar);
		else return Matrix4x4::Perspective(fov, aspect, zNear, zFar);
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
		s = (s - 0.5f) / 2.0f * aspect + 0.5f;  auto pos = transform.position;
		Vector3 rd = Renderer::depth ? MoonMath::RandomInUnitDisk() * lens_radius : Vector3::ZERO();
		Vector3 offset = transform.GetLocalAxis(RIGHT) * rd.x + transform.GetLocalAxis(UP) * rd.y;
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
			(target ? (target->bbox.max - target->bbox.min).magnitude() * 2.0f : 20.0f);
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
	// *NOTE: depth test should be enabled
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

	void Camera::Draw(Shader* overrideShader) {
		//Gizmo::DrawPointDirect(transform.position, Color::RED(), 6.0f);
		std::vector<Vector3> camShape(cameraShape);

		// draw field of view
		auto farCenter = Vector3::WORLD(FORWARD) * zFar;
		auto nearCenter = Vector3::WORLD(FORWARD) * zNear;
		if (isortho) {
			float y = height / 2.0f, x = width / 2.0f;

			camShape.push_back(farCenter  + Vector3( x,  y, 0));
			camShape.push_back(nearCenter + Vector3( x,  y, 0));
			camShape.push_back(farCenter  + Vector3(-x,  y, 0));
			camShape.push_back(nearCenter + Vector3(-x,  y, 0));
			camShape.push_back(farCenter  + Vector3(-x, -y, 0));
			camShape.push_back(nearCenter + Vector3(-x, -y, 0));
			camShape.push_back(farCenter  + Vector3( x, -y, 0));
			camShape.push_back(nearCenter + Vector3( x, -y, 0));

			camShape.push_back(farCenter + Vector3( x,  y, 0));
			camShape.push_back(farCenter + Vector3(-x,  y, 0));
			camShape.push_back(farCenter + Vector3(-x,  y, 0));
			camShape.push_back(farCenter + Vector3(-x, -y, 0));
			camShape.push_back(farCenter + Vector3(-x, -y, 0));
			camShape.push_back(farCenter + Vector3( x, -y, 0));
			camShape.push_back(farCenter + Vector3( x, -y, 0));
			camShape.push_back(farCenter + Vector3( x,  y, 0));

			camShape.push_back(nearCenter + Vector3( x,  y, 0));
			camShape.push_back(nearCenter + Vector3(-x,  y, 0));
			camShape.push_back(nearCenter + Vector3(-x,  y, 0));
			camShape.push_back(nearCenter + Vector3(-x, -y, 0));
			camShape.push_back(nearCenter + Vector3(-x, -y, 0));
			camShape.push_back(nearCenter + Vector3( x, -y, 0));
			camShape.push_back(nearCenter + Vector3( x, -y, 0));
			camShape.push_back(nearCenter + Vector3( x,  y, 0));
		} else {
			float y = zFar * std::tanf(fov * Deg2Rad / 2.0f);
			float ny = zNear * std::tanf(fov * Deg2Rad / 2.0f);
			float x = y * aspect, nx = ny * aspect;

			camShape.push_back(Vector3::ZERO());
			camShape.push_back(farCenter + Vector3( x,  y, 0));
			camShape.push_back(Vector3::ZERO());
			camShape.push_back(farCenter + Vector3(-x,  y, 0));
			camShape.push_back(Vector3::ZERO());
			camShape.push_back(farCenter + Vector3( x, -y, 0));
			camShape.push_back(Vector3::ZERO());
			camShape.push_back(farCenter + Vector3(-x, -y, 0));
			
			camShape.push_back(nearCenter + Vector3( nx,  ny, 0));
			camShape.push_back(nearCenter + Vector3(-nx,  ny, 0));
			camShape.push_back(nearCenter + Vector3(-nx,  ny, 0));
			camShape.push_back(nearCenter + Vector3(-nx, -ny, 0));
			camShape.push_back(nearCenter + Vector3(-nx, -ny, 0));
			camShape.push_back(nearCenter + Vector3( nx, -ny, 0));
			camShape.push_back(nearCenter + Vector3( nx, -ny, 0));
			camShape.push_back(nearCenter + Vector3( nx,  ny, 0));

			camShape.push_back(farCenter + Vector3( x,  y, 0));
			camShape.push_back(farCenter + Vector3(-x,  y, 0));
			camShape.push_back(farCenter + Vector3(-x,  y, 0));
			camShape.push_back(farCenter + Vector3(-x, -y, 0));
			camShape.push_back(farCenter + Vector3(-x, -y, 0));
			camShape.push_back(farCenter + Vector3( x, -y, 0));
			camShape.push_back(farCenter + Vector3( x, -y, 0));
			camShape.push_back(farCenter + Vector3( x,  y, 0));
		}

		Gizmo::DrawLinesDirect(camShape, wireColor, overrideShader == NULL ? 1.0f : 5.0f, false, transform.localToWorldMat, overrideShader);
	}

	void Camera::ListProperties() {
		// list name
		ListName();
		ImGui::Separator();

		// list transform
		ListTransform();
		if (transform.changeFlag) UpdateMatrix();
		ImGui::Separator();

		// list camera properties
		ImGui::Text("Aperture"); ImGui::SameLine(80.0f);
		float aperture = lens_radius * 2.0f;
		if (ImGui::DragFloat("aperture", &aperture, 0.1f, 0, 0, "%.3f", 1.0f, true)) {
			lens_radius = aperture / 2.0f;
		}
		ImGui::Text("Near/Far"); ImGui::SameLine(80.0f);
		float halfWidth = ImGui::GetContentRegionAvailWidth() / 2.0f - 5.0f;
		ImGui::SetNextItemWidth(halfWidth);
		if (ImGui::DragFloat("near", &zNear, 0.1f, 0.0f, zFar, "%.3f", 1.0f, true)) {
			UpdateMatrix();
		} ImGui::SameLine(90.0f + halfWidth);
		ImGui::SetNextItemWidth(halfWidth);
		if (ImGui::DragFloat("far", &zFar, 0.1f, zNear, INFINITY, "%.3f", 1.0f, true)) {
			UpdateMatrix();
		}
		ImGui::Text("Ortho"); ImGui::SameLine(80.0f);
		if (ImGui::Checkbox("iso", &isortho, true)) {
			UpdateMatrix();
		}
		if (isortho) {
			ImGui::SameLine(); SwitchButton(ICON_FA_LOCK, ICON_FA_LOCK, lockSize);

			ImGui::Text("ScrSize"); ImGui::SameLine(80.0f);
			if (lockSize) {
				ImGui::Text((Strutil::to_string_precision(width, 2) + u8" ¡Á " +
					Strutil::to_string_precision(height, 2)).c_str());
			} else {
				ImGui::SetNextItemWidth(halfWidth);
				if (ImGui::DragFloat("width", &width, 0.1f, 0, 0, "%.3f", 1.0f, true)) {
					UpdateMatrix();
				} ImGui::SameLine(90.0f + halfWidth);
				ImGui::SetNextItemWidth(halfWidth);
				if (ImGui::DragFloat("height", &height, 0.1f, 0, 0, "%.3f", 1.0f, true)) {
					UpdateMatrix();
				}
			}
		} else {
			ImGui::Text("FOV"); ImGui::SameLine(80.0f);
			if (ImGui::DragFloat("fov", &fov, 0.1f, 0.0f, 90.0f, "%.3f", 1.0f, true)) {
				UpdateMatrix();
			}
		}
	}
}