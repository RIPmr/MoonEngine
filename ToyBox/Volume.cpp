#include "Volume.h"
#include "Debugger.h"
#include "SceneMgr.h"
#include "HotkeyMgr.h"
#include "Coroutine.h"
#include "Utility.h"

namespace MOON {

	void Volume::InteractiveCreate(void* arg) {
		HotKeyManager::state = CREATE;
		Volume* sp = (Volume*)arg;

		bool pickFlag[2] = { true, true };
		while (pickFlag[0] && HotKeyManager::state == CREATE) {
			auto pickPoint = MOON_InputManager::PickGroundPoint(MOON_MousePos);
			Gizmo::DrawPoint(pickPoint, Color::RED(), 6.0f);

			if (MOON_InputManager::IsMouseDown(0)) {
				sp->transform.position = Vector3::ZERO();
				sp->bbox.min = pickPoint;
				sp->bbox.max = pickPoint;
				pickFlag[0] = false;
				break;
			}
			yield_return();
		}
		while (pickFlag[1] && HotKeyManager::state == CREATE) {
			if (MOON_InputManager::mouse_left_hold) {
				auto newPoint = MOON_InputManager::PickGroundPoint(MOON_MousePos);
				sp->bbox.min.setValue(
					std::min(sp->bbox.min.x, newPoint.x),
					std::min(sp->bbox.min.y, newPoint.y),
					std::min(sp->bbox.min.z, newPoint.z)
				);
				sp->bbox.max.setValue(
					std::max(sp->bbox.min.x, newPoint.x),
					std::max(sp->bbox.min.y, newPoint.y),
					std::max(sp->bbox.min.z, newPoint.z)
				);
			} else if (MOON_InputManager::IsMouseRelease(0)) {
				pickFlag[1] = false;
				break;
			}
			yield_return();
		}
		while (HotKeyManager::state == CREATE) {
			if (MOON_InputManager::IsMouseDown(0)) {
				return;
			} else {
				auto newPoint = MOON_InputManager::PickGroundPoint(MOON_MousePos);
				Vector3 checkPoint = sp->transform.position + Vector3(sp->bbox.size.x, 0, sp->bbox.size.z);
				checkPoint.x = newPoint.x;
				float len = newPoint.distance(checkPoint);
				float ang = Vector3::Angle(
					checkPoint - newPoint,
					MOON_ActiveCamera->transform.position - newPoint
				);
				sp->bbox.max.y = len * std::tanf(ang);
			}
			yield_return();
		}

		MOON_VolumeManager::DeleteItem(sp->ID);
	}

	void Volume::CreateProcedural(const bool& interactive) {
		if (interactive) {
			Coroutine::ptr co = Coroutine::create_coroutine(InteractiveCreate, this);
		}
	}

	void Volume::Draw(Shader* overrideShader) {
		auto flag = overrideShader ? (overrideShader->name._Equal("RayMarching") || 
			overrideShader->name._Equal("Galaxy")) : false;
		if (flag) SetupParameters(overrideShader);

		// start drawing
		if (drawBound) {
			std::vector<Vector3> corners; bbox.GetCorners(&corners);

			std::vector<Vector3> drawList{
				corners[0], corners[1], corners[1], corners[2],
				corners[2], corners[3], corners[3], corners[0],

				corners[4], corners[5], corners[5], corners[6],
				corners[6], corners[7], corners[7], corners[4],

				corners[0], corners[6], corners[5], corners[3],
				corners[2], corners[4], corners[7], corners[1]
			};

			Gizmo::DrawLinesDirect(
				drawList, wireColor, flag ? 1.0f : 4.0f, 
				false, transform.localToWorldMat, 
				flag ? nullptr : overrideShader
			);
		}
	}

	void Volume::SetupParameters(Shader* rayMarchingShader) {
		rayMarchingShader->use();

		// volume params ----------------------------------------------------
		rayMarchingShader->setVec3("bboxMax", transform.localToWorldMat.multVec(bbox.min));
		rayMarchingShader->setVec3("bboxMin", transform.localToWorldMat.multVec(bbox.max));

		rayMarchingShader->setFloat("_step", step);
		rayMarchingShader->setFloat("_rayStep", rayStep);
		rayMarchingShader->setFloat("_lightStep", lightStep);
		rayMarchingShader->setInt("_maxMarchLoop", maxMarchLoop);

		rayMarchingShader->setFloat("_darknessThreshold", darknessThreshold);
		rayMarchingShader->setFloat("_midtoneOffset", midtoneOffset);
		rayMarchingShader->setFloat("_shadowOffset", shadowOffset);
		rayMarchingShader->setFloat("_lightAbsorptionTowardSun", lightAbsorptionTowardSun);
		rayMarchingShader->setFloat("_lightAbsorptionThroughCloud", lightAbsorptionThroughCloud);
		rayMarchingShader->setVec3("_mainColor", mainColor);
		rayMarchingShader->setVec3("_shadowColor", shadowColor);
		rayMarchingShader->setVec4("_phaseParams", phaseParams);

		rayMarchingShader->setFloat("_scatterMultiply", scatterMultiply);
		rayMarchingShader->setFloat("_densityOffset", densityOffset);
		rayMarchingShader->setFloat("_densityMultiply", densityMultiply);

		rayMarchingShader->setInt("_downSampling", downSampling);
		rayMarchingShader->setVec3("_offset", offset);
		rayMarchingShader->setFloat("_scale", scale);
		rayMarchingShader->setFloat("_time", time);
		rayMarchingShader->setVec2("_noiseMulti", multiply);
	}

	void Volume::ListVolumePropties() {
		ImGui::Text("Volume:");

		ImGui::Indent(10.0f);
		float interval = 120.0f;

		ImGui::Text("Draw Bound"); ImGui::SameLine(interval);
		auto widgetWidth = ImGui::GetContentRegionAvailWidth();
		ImGui::SetNextItemWidth(widgetWidth);
		ButtonEx::CheckboxNoLabel("bound", &drawBound);

		ImGui::Text("Use Light"); ImGui::SameLine(interval);
		ImGui::SetNextItemWidth(widgetWidth);
		ButtonEx::CheckboxNoLabel("useLight", &useLight);

		ImGui::Text("Write Depth"); ImGui::SameLine(interval);
		ImGui::SetNextItemWidth(widgetWidth);
		ButtonEx::CheckboxNoLabel("writeDepth", &writeDepth);

		const char* items[] = { "Noise", "Galaxy", "Tex3D", "OpenVDB", "Alembic" };
		ImGui::AlignTextToFramePadding();
		ImGui::Text("Source"); ImGui::SameLine(interval);
		ImGui::SetNextItemWidth(widgetWidth);
		ButtonEx::ComboNoLabel("sourceType", (int*)&source, items, IM_ARRAYSIZE(items));

		if (source <= Galaxy) {
			ImGui::Text("Time"); ImGui::SameLine(interval);
			ImGui::SetNextItemWidth(widgetWidth);
			ButtonEx::DragFloatNoLabel("time", &time, 0.01f, 0.0f, 0.0f, "%.3f");

			ImGui::Text("Noise Offset"); ImGui::SameLine(interval);
			ImGui::SetNextItemWidth(widgetWidth);
			ButtonEx::DragVec3NoLabel("offset", &offset[0], 0.01f, 0.0f, 0.0f, "%.2f");

			ImGui::Text("Noise Scale"); ImGui::SameLine(interval);
			ImGui::SetNextItemWidth(widgetWidth);
			ButtonEx::DragFloatNoLabel("scale", &scale, 0.01f, 0.0f, 0.0f, "%.3f");

			ImGui::Text("Noise Multi"); ImGui::SameLine(interval);
			ImGui::SetNextItemWidth(widgetWidth);
			ButtonEx::DragVec2NoLabel("noiseMul", &multiply[0], 0.01f, 0.0f, 0.0f, "%.2f");
		}

		ImGui::Text("Max March"); ImGui::SameLine(interval);
		ImGui::SetNextItemWidth(widgetWidth);
		ButtonEx::DragIntNoLabel("march", &maxMarchLoop, 1, 1, 256);

		ImGui::Text("March Step"); ImGui::SameLine(interval);
		ImGui::SetNextItemWidth(widgetWidth);
		ButtonEx::DragFloatNoLabel("step", &step, 0.001f, 0.001f, 1.0f, "%.3f");

		ImGui::Text("Ray Step"); ImGui::SameLine(interval);
		ImGui::SetNextItemWidth(widgetWidth);
		ButtonEx::DragFloatNoLabel("rayStep", &rayStep, 0.001f, 0.001f, 1.0f, "%.3f");

		ImGui::Text("Light Step"); ImGui::SameLine(interval);
		ImGui::SetNextItemWidth(widgetWidth);
		ButtonEx::DragIntNoLabel("liStep", &lightStep, 1, 1, 256);

		ImGui::Text("Dark Threshold"); ImGui::SameLine(interval);
		ImGui::SetNextItemWidth(widgetWidth);
		ButtonEx::DragFloatNoLabel("dark", &darknessThreshold, 0.001f, 0.0f, 1.0f, "%.3f");

		ImGui::Text("Midtone Offset"); ImGui::SameLine(interval);
		ImGui::SetNextItemWidth(widgetWidth);
		ButtonEx::DragFloatNoLabel("mid", &midtoneOffset, 0.001f, 0.0f, 0.0f, "%.3f");

		ImGui::Text("Shadow Offset"); ImGui::SameLine(interval);
		ImGui::SetNextItemWidth(widgetWidth);
		ButtonEx::DragFloatNoLabel("shadow", &shadowOffset, 0.001f, 0.0f, 0.0f, "%.3f");

		ImGui::Text("Sun Absorb"); ImGui::SameLine(interval);
		ImGui::SetNextItemWidth(widgetWidth);
		ButtonEx::DragFloatNoLabel("sunAb", &lightAbsorptionTowardSun, 0.001f, 0.0f, 0.0f, "%.3f");

		ImGui::Text("Light Absorb"); ImGui::SameLine(interval);
		ImGui::SetNextItemWidth(widgetWidth);
		ButtonEx::DragFloatNoLabel("liAb", &lightAbsorptionThroughCloud, 0.001f, 0.0f, 0.0f, "%.3f");

		ImGui::Text("Main Color"); ImGui::SameLine(interval);
		ImGui::SetNextItemWidth(widgetWidth);
		ImGui::ColorEdit3(
			UniquePropName("mainCol"), &mainColor[0],
			ImGuiColorEditFlags_NoLabel
		);

		ImGui::Text("Shadow Color"); ImGui::SameLine(interval);
		ImGui::SetNextItemWidth(widgetWidth);
		ImGui::ColorEdit3(
			UniquePropName("shadCol"), &shadowColor[0],
			ImGuiColorEditFlags_NoLabel
		);

		ImGui::Text("Scatter Phase"); ImGui::SameLine(interval);
		ImGui::SetNextItemWidth(widgetWidth);
		ButtonEx::DragVec4NoLabel("phase", &phaseParams[0], 0.001f, 0.0f, 0.0f, "%.2f");

		ImGui::Text("Scatter Multi"); ImGui::SameLine(interval);
		ImGui::SetNextItemWidth(widgetWidth);
		ButtonEx::DragFloatNoLabel("scaMul", &scatterMultiply, 0.001f, 0.0f, 0.0f, "%.3f");

		ImGui::Text("Density Offset"); ImGui::SameLine(interval);
		ImGui::SetNextItemWidth(widgetWidth);
		ButtonEx::DragFloatNoLabel("densOff", &densityOffset, 0.001f, 0.0f, 0.0f, "%.3f");

		ImGui::Text("Density Multi"); ImGui::SameLine(interval);
		ImGui::SetNextItemWidth(widgetWidth);
		ButtonEx::DragFloatNoLabel("densMul", &densityMultiply, 0.01f, 0.0f, 0.0f, "%.3f");

		ImGui::Text("Downsampling"); ImGui::SameLine(interval);
		ImGui::SetNextItemWidth(widgetWidth);
		ButtonEx::DragIntNoLabel("downsampling", &downSampling, 1, 1, 16);

		ImGui::Unindent(10.0f);
	}

}