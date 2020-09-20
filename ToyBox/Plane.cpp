#include "Gizmo.h"
#include "Plane.h"
#include "SceneMgr.h"
#include "HotkeyMgr.h"
#include "UIController.h"

#include <iostream>

namespace MOON {
	void Plane::InteractiveCreate(void* arg) {
		HotKeyManager::state = CREATE;
		Plane* sp = (Plane*)arg;

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
				auto newPoint = MOON_InputManager::PickGroundPoint(MOON_MousePos);
				sp->size.setValue(
					std::fabs(sp->transform.position.x - newPoint.x),
					std::fabs(sp->transform.position.z - newPoint.z)
				);
				Utility::ReleaseVector(sp->meshList);
				sp->meshList.push_back(GenerateMesh(sp->size, sp->segment));
			} else if (MOON_InputManager::IsMouseRelease(0)) {
				return;
			}
			yield_return();
		}

		MOON_ModelManager::DeleteItem(sp->ID);
	}

	void Plane::CreateProceduralMesh(const bool& interactive) {
		if (interactive) {
			Coroutine::create_coroutine(InteractiveCreate, this);
		} else {
			Utility::ReleaseVector(meshList);
			meshList.push_back(GenerateMesh(size, segment));
		}
	}

	void Plane::ListProceduralProperties() {
		ImGui::Text("Plane:");
		ImGui::Indent(10.0f);
		ImGui::Text("Size"); ImGui::SameLine(80);
		if (ImGui::DragFloat2(UniquePropName("size"), (float*)&size, 0.1f, 0, INFINITY, "%.3f", 1.0f, true)) {
			CreateProceduralMesh(false);
		}
		ImGui::Text("Segment"); ImGui::SameLine(80);
		int seg[2] = { segment.x, segment.y };
		if (ImGui::DragInt2(UniquePropName("segment"), seg, 1.0f, 1, 255, "%d", true)) {
			segment.setValue(seg[0], seg[1]);
			CreateProceduralMesh(false);
		}
		ImGui::Unindent(10.0f);
	}

	Mesh* Plane::GenerateMesh(Vector2 size, Vector2 segment) {
		std::vector<Vertex> verts;
		std::vector<unsigned int> tris;

		//verts.resize((segment.x + 1) * (segment.y + 1));
		tris.resize(segment.x * segment.y * 6);

		for (int z = 0; z <= segment.y; z++) {
			float v = (float)z / segment.y;
			for (int x = 0; x <= segment.x; x++) {
				float u = (float)x / segment.x;
				verts.push_back(Vertex());
				verts[verts.size() - 1].Position.setValue(u * size.x, 0, v * size.y);
				verts[verts.size() - 1].UV.setValue(u, v);
				verts[verts.size() - 1].Normal.setValue(0, 1, 0);
			}
		}

		int startIndex = 0;
		int a = 0, b = 0, c = 0, d = 0;
		for (int y = 0; y < segment.y; y++) {
			for (int x = 0; x < segment.x; x++) {
				a = y * (segment.x + 1) + x;
				b = (y + 1) * (segment.x + 1) + x;
				c = b + 1;
				d = a + 1;

				startIndex = y * segment.x * 2 * 3 + x * 2 * 3;

				tris[startIndex] = a;
				tris[startIndex + 1] = b;
				tris[startIndex + 2] = c;

				tris[startIndex + 3] = c;
				tris[startIndex + 4] = d;
				tris[startIndex + 5] = a;
			}
		}

		Mesh* mesh = new Mesh("plane", verts, tris);
		mesh->material = MOON_MaterialManager::GetItem("default");
		mesh->UpdateBBox();
		return mesh;
	}

}