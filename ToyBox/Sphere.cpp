#include "Gizmo.h"
#include "Sphere.h"
#include "SceneMgr.h"
#include "HotkeyMgr.h"
#include "UIController.h"

#include <iostream>

namespace MOON {
	void Sphere::InteractiveCreate(void* arg) {
		HotKeyManager::state = CREATE;
		Sphere* sp = (Sphere*)arg;

		bool pickFlag = true;
		while (pickFlag && HotKeyManager::state == CREATE) {
			auto pickPoint = MOON_InputManager::PickGroundPoint(MOON_MousePos);
			Gizmo::DrawPoint(pickPoint, Color::RED(), 6.0f);

			if (MOON_InputManager::IsMouseDown(0)) {
				sp->transform.position = pickPoint;
				pickFlag = false;
				break;
			}
			yield_return();
		}
		while (!pickFlag && HotKeyManager::state == CREATE) {
			if (MOON_InputManager::mouse_left_hold) {
				sp->radius = sp->transform.position.distance(MOON_InputManager::PickGroundPoint(MOON_MousePos));
				Utility::ReleaseVector(sp->meshList);
				sp->meshList.push_back(GenerateMesh(sp->radius, sp->segment));
			} else if (MOON_InputManager::IsMouseRelease(0)) {
				return;
			}
			yield_return();
		}

		MOON_ModelManager::DeleteItem(sp->ID);
	}

	void Sphere::CreateProcedural(const bool& interactive) {
		if (interactive) {
			Coroutine::ptr co = Coroutine::create_coroutine(InteractiveCreate, this);
		} else {
			Utility::ReleaseVector(meshList);
			meshList.push_back(GenerateMesh(radius, segment));
		}
	}

	void Sphere::ListProceduralProperties() {
		ImGui::Text("Sphere:");
		ImGui::Indent(10.0f);
		ImGui::Text("Radius"); ImGui::SameLine(80);
		if (ImGui::DragFloat(UniquePropName("radius"), &radius, 0.1f, 0, INFINITY, "%.3f", 1.0f, true)) {
			CreateProcedural(false);
			opstack.ExecuteAll();
		}
		ImGui::Text("Segment"); ImGui::SameLine(80);
		if (ImGui::DragInt(UniquePropName("segment"), &segment, 1.0f, 3, 255, "%d", true)) {
			CreateProcedural(false);
			opstack.ExecuteAll();
		}
		ImGui::Unindent(10.0f);
	}

	Mesh* Sphere::GenerateMesh(float rad, int divs) {
		std::vector<Vertex> verts;
		std::vector<unsigned int> tris;

		int m = divs, n = divs;
		float width = rad, height = rad;
		verts.resize((m + 1) * (n + 1));
		tris.resize(6 * m * n);

		for (int i = 0; i < verts.size(); i++) {
			float x = i % (n + 1);
			float y = i / (n + 1);
			float x_pos = x / n * width;
			float y_pos = y / m * height;
			verts[i].Position.setValue(x_pos, y_pos, 0);

			float u = x / n;
			float v = y / m;
			verts[i].UV.setValue(u, v);
		}

		for (int i = 0; i < 2 * m * n; i++) {
			int triIndex[3];
			if (i % 2 == 0) {
				triIndex[0] = i / 2 + i / (2 * n);
				triIndex[1] = triIndex[0] + 1;
				triIndex[2] = triIndex[0] + (n + 1);
			} else {
				triIndex[0] = (i + 1) / 2 + i / (2 * n);
				triIndex[1] = triIndex[0] + (n + 1);
				triIndex[2] = triIndex[1] - 1;
			}
			tris[i * 3] = triIndex[0];
			tris[i * 3 + 1] = triIndex[1];
			tris[i * 3 + 2] = triIndex[2];
		}

		for (int i = 0; i < verts.size(); i++) {
			verts[i].Position.setValue(
				rad * std::cosf(verts[i].Position.x / width * 2 * PI) * std::cosf(verts[i].Position.y / height * PI - PI / 2),
				rad * std::sinf(verts[i].Position.x / width * 2 * PI) * std::cosf(verts[i].Position.y / height * PI - PI / 2),
				rad * std::sinf(verts[i].Position.y / height * PI - PI / 2)
			);
			verts[i].Normal.setValue(verts[i].Position);
		}

		Mesh* mesh = new Mesh("sphere", verts, tris);
		mesh->material = MOON_MaterialManager::defaultMat;
		mesh->UpdateBBox();
		return mesh;
	}

}