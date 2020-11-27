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
#pragma region parameters
	const int Camera::A = 0; const int Camera::B = 1;
	const int Camera::C = 2; const int Camera::D = 3;
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
#pragma endregion

#pragma region matrix_operators
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
#pragma endregion

#pragma camera_interactions
	void Camera::PanCamera(Vector2 mouseOffset) {
		mouseOffset *= MouseSensitivity;
		Vector3 offset = transform.GetLocalAxis(RIGHT) * mouseOffset.x + 
			transform.GetLocalAxis(UP) * mouseOffset.y;
		if (isortho) offset *= width / 10.0f;
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
			auto delta = mouseScrollOffset.y * MouseSensitivity * width;
			orthoDelta += delta; width -= aspect * delta; height -= delta;
		} else transform.position += transform.GetLocalAxis(FORWARD) * mouseScrollOffset.y;

		UpdateMatrix();
	}

	void Camera::InitRenderCamera() {
		float aspect = 2.0f;
		float theta = fov * Deg2Rad;
		float half_height = tan(theta / aspect);
		float half_width = aspect * half_height;

		auto forward = transform.GetLocalAxis(FORWARD);
		auto right = transform.GetLocalAxis(RIGHT);
		auto up = transform.GetLocalAxis(UP);

		float focus_dist = (tarPos - transform.position).magnitude();
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
#pragma endregion

#pragma region math_functions
	// screen pos to world ray fast version (used in renderer)
	Ray Camera::GetRay(float s, float t, float aspect) {
		s = (s - 0.5f) / 2.0f * aspect + 0.5f;  auto pos = transform.position;
		Vector3 rd = Renderer::depth ? MoonMath::RandomInUnitDisk() * lens_radius : Vector3::ZERO();
		Vector3 offset = transform.GetLocalAxis(RIGHT) * rd.x + transform.GetLocalAxis(UP) * rd.y;
		return Ray(pos + offset, lower_left_corner + s * horizontal + t * vertical - pos - offset);
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
#pragma endregion

#pragma region graphics
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
			ImGui::SameLine(); ButtonEx::SwitchButton(ICON_FA_LOCK, ICON_FA_LOCK, lockSize);

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
#pragma endregion

#pragma view_frustum_culling
	// Normalizes a plane (A side) from a given frustum.
	void Camera::NormalizePlane(std::vector<Vector4>& _frustum, int _side) {
		float magnitude = std::sqrtf(
			_frustum[_side][A] * _frustum[_side][A] +
			_frustum[_side][B] * _frustum[_side][B] +
			_frustum[_side][C] * _frustum[_side][C]
		);

		_frustum[_side][A] /= magnitude;
		_frustum[_side][B] /= magnitude;
		_frustum[_side][C] /= magnitude;
		_frustum[_side][D] /= magnitude;
	}

	// Extracts camera frustum from the projection and modelview matrix.
	void Camera::UpdateFrustum() {
		Matrix4x4 clip = transform.localToWorldMat * view * projection; 
		frustum.clear(); for (int i = 0; i < 6; i++) frustum.push_back(Vector4::ZERO());

		frustum[RIGHT][A] = clip[0][3] - clip[0][0];
		frustum[RIGHT][B] = clip[1][3] - clip[1][0];
		frustum[RIGHT][C] = clip[2][3] - clip[2][0];
		frustum[RIGHT][D] = clip[3][3] - clip[3][0];
		NormalizePlane(frustum, RIGHT);

		frustum[LEFT][A] = clip[0][3] + clip[0][0];
		frustum[LEFT][B] = clip[1][3] + clip[1][0];
		frustum[LEFT][C] = clip[2][3] + clip[2][0];
		frustum[LEFT][D] = clip[3][3] + clip[3][0];
		NormalizePlane(frustum, LEFT);

		frustum[DOWN][A] = clip[0][3] + clip[0][1];
		frustum[DOWN][B] = clip[1][3] + clip[1][1];
		frustum[DOWN][C] = clip[2][3] + clip[2][1];
		frustum[DOWN][D] = clip[3][3] + clip[3][1];
		NormalizePlane(frustum, DOWN);

		frustum[UP][A] = clip[0][3] - clip[0][1];
		frustum[UP][B] = clip[1][3] - clip[1][1];
		frustum[UP][C] = clip[2][3] - clip[2][1];
		frustum[UP][D] = clip[3][3] - clip[3][1];
		NormalizePlane(frustum, UP);

		frustum[BACKWARD][A] = clip[0][3] - clip[0][2];
		frustum[BACKWARD][B] = clip[1][3] - clip[1][2];
		frustum[BACKWARD][C] = clip[2][3] - clip[2][2];
		frustum[BACKWARD][D] = clip[3][3] - clip[3][2];
		NormalizePlane(frustum, BACKWARD);

		frustum[FORWARD][A] = clip[0][3] + clip[0][2];
		frustum[FORWARD][B] = clip[1][3] + clip[1][2];
		frustum[FORWARD][C] = clip[2][3] + clip[2][2];
		frustum[FORWARD][D] = clip[3][3] + clip[3][2];
		NormalizePlane(frustum, FORWARD);
	}

	bool Camera::IsPointInFrustum(const Vector3& p) {
		UpdateFrustum();
		// If you remember the plane equation (A*x + B*y + C*z + D = 0), then the rest
		// of this code should be quite obvious and easy to figure out yourself.
		for (int i = 0; i < 6; i++) {
			// Calculate the plane equation and check if the point is behind a side of the frustum
			if (frustum[i][A] * p.x + frustum[i][B] * p.y + frustum[i][C] * p.z + frustum[i][D] <= 0) {
				// The point was behind a side, so it ISN'T in the frustum
				return false;
			}
		}
		return true;
	}

	bool Camera::IsSphereInFrustum(const Vector3& p, const float& radius) {
		UpdateFrustum();
		for (int i = 0; i < 6; i++) {
			// If the center of the sphere is farther away from the plane than the radius
			if (frustum[i][A] * p.x + frustum[i][B] * p.y + frustum[i][C] * p.z + frustum[i][D] <= -radius) {
				// The distance was greater than the radius so the sphere is outside of the frustum
				return false;
			}
		}

		return true;
	}

	// *NOTE: just detect 8 corners of the bbox, so may not correct some times.
	bool Camera::IsBBoxInFrustum(const Vector3& center, const Vector3& extend) {
		UpdateFrustum();

		for (int i = 0; i < 6; i++) {
			if (frustum[i][A] * (center.x - extend.x) + frustum[i][B] * (center.y - extend.y) +
				frustum[i][C] * (center.z - extend.z) + frustum[i][D] > 0)
				continue;
			if (frustum[i][A] * (center.x + extend.x) + frustum[i][B] * (center.y - extend.y) +
				frustum[i][C] * (center.z - extend.z) + frustum[i][D] > 0)
				continue;
			if (frustum[i][A] * (center.x - extend.x) + frustum[i][B] * (center.y + extend.y) +
				frustum[i][C] * (center.z - extend.z) + frustum[i][D] > 0)
				continue;
			if (frustum[i][A] * (center.x + extend.x) + frustum[i][B] * (center.y + extend.y) +
				frustum[i][C] * (center.z - extend.z) + frustum[i][D] > 0)
				continue;
			if (frustum[i][A] * (center.x - extend.x) + frustum[i][B] * (center.y - extend.y) +
				frustum[i][C] * (center.z + extend.z) + frustum[i][D] > 0)
				continue;
			if (frustum[i][A] * (center.x + extend.x) + frustum[i][B] * (center.y - extend.y) +
				frustum[i][C] * (center.z + extend.z) + frustum[i][D] > 0)
				continue;
			if (frustum[i][A] * (center.x - extend.x) + frustum[i][B] * (center.y + extend.y) +
				frustum[i][C] * (center.z + extend.z) + frustum[i][D] > 0)
				continue;
			if (frustum[i][A] * (center.x + extend.x) + frustum[i][B] * (center.y + extend.y) +
				frustum[i][C] * (center.z + extend.z) + frustum[i][D] > 0)
				continue;

			// If we get here, it isn't in the frustum
			return false;
		}

		return true;
	}
#pragma endregion

}