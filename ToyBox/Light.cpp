#include <imgui.h>
#include "Light.h"
#include "Gizmo.h"
#include "ButtonEx.h"
#include "Matrix4x4.h"
#include "SceneMgr.h"

namespace MOON {

#pragma region light
	std::vector<Vector3>& Light::GetLightShape() { 
		static std::vector<Vector3> empty;
		return empty;
	}

	void Light::Draw(Shader* overrideShader) {
		Gizmo::DrawLinesDirect(
			GetLightShape(), wireColor,
			overrideShader == NULL ? 0.5f : 5.0f, true, 
			transform.localToWorldMat * ViewportDistanceMatrix, 
			overrideShader
		);
	}

	void Light::ListProperties() {
		// list name
		ListName();
		ImGui::Separator();

		// list transform
		ListTransform();
		ImGui::Separator();

		// list other props
		ListLightProperties();
	}

	void Light::ListLightProperties() {
		ImGui::Text("Light:");

		ImGui::Indent(10.0f);
		//ImGui::Text("Color"); ImGui::SameLine(80.0f);
		ImGui::ColorEdit3("Color", &color[0]);
		ImGui::Text("Power"); ImGui::SameLine(80.0f);
		ButtonEx::DragFloatNoLabel("power", &power, 0.1f);
		ImGui::Unindent(10.0f);
	}
#pragma endregion

#pragma region dir_light
	std::vector<Vector3>& DirLight::GetLightShape() {
		static std::vector<Vector3> lightShape = std::vector<Vector3>{
			Vector3(0.5f, 0.0f, 0.0f), Vector3(0.0f, 0.0f, 0.5f), Vector3(-0.5f, 0.0f, 0.0f),
			Vector3(0.0f, 0.0f, -0.5f), Vector3(0.5f, 0.0f, 0.0f), Vector3(0.0f, -0.75f, 0.0f),
			Vector3(0.0f, 0.0f, 0.5f), Vector3(0.0f, 0.75f, 0.0f), Vector3(-0.5f, 0.0f, 0.0f),
			Vector3(0.0f, -0.75f, 0.0f), Vector3(0.0f, 0.0f, -0.5f), Vector3(0.0f, 0.75f, 0.0f),
			Vector3(0.5f, 0.0f, 0.0f)
		};

		return lightShape;
	}

	void DirLight::ListLightProperties() {
		ImGui::Text("Light[Point]:");

		ImGui::Indent(10.0f);
		//ImGui::Text("Color"); ImGui::SameLine(80.0f);
		ImGui::ColorEdit3("Color", &color[0]);
		ImGui::Text("Power"); ImGui::SameLine(80.0f);
		ButtonEx::DragFloatNoLabel("power", &power, 0.1f);
		ImGui::Unindent(10.0f);
	}
#pragma endregion

#pragma region point_light
	std::vector<Vector3>& PointLight::GetLightShape() {
		static std::vector<Vector3> lightShape = std::vector<Vector3>{
			Vector3(0.5f, 0.0f, 0.0f), Vector3(0.0f, 0.0f, 0.5f), Vector3(-0.5f, 0.0f, 0.0f),
			Vector3(0.0f, 0.0f, -0.5f), Vector3(0.5f, 0.0f, 0.0f), Vector3(0.0f, -0.75f, 0.0f),
			Vector3(0.0f, 0.0f, 0.5f), Vector3(0.0f, 0.75f, 0.0f), Vector3(-0.5f, 0.0f, 0.0f),
			Vector3(0.0f, -0.75f, 0.0f), Vector3(0.0f, 0.0f, -0.5f), Vector3(0.0f, 0.75f, 0.0f),
			Vector3(0.5f, 0.0f, 0.0f)
		};

		return lightShape;
	}

	void PointLight::ListLightProperties() {
		ImGui::Text("Light[Point]:");

		ImGui::Indent(10.0f);
		//ImGui::Text("Color"); ImGui::SameLine(80.0f);
		ImGui::ColorEdit3("Color", &color[0]);
		ImGui::Text("Power"); ImGui::SameLine(80.0f);
		ButtonEx::DragFloatNoLabel("power", &power, 0.1f);
		ImGui::Unindent(10.0f);
	}
#pragma endregion

#pragma region spot_light

#pragma endregion

#pragma region moon_light

#pragma endregion

#pragma region dome_light

#pragma endregion

}