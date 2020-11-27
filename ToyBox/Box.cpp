#include "Gizmo.h"
#include "Box.h"
#include "SceneMgr.h"
#include "HotkeyMgr.h"
#include "UIController.h"

#include <iostream>

namespace MOON {
	void Box::InteractiveCreate(void* arg) {
		HotKeyManager::state = CREATE;
		Box* sp = (Box*)arg;

		bool pickFlag[2] = { true, true };
		while (pickFlag[0] && HotKeyManager::state == CREATE) {
			auto pickPoint = MOON_InputManager::PickGroundPoint(MOON_MousePos);
			Gizmo::DrawPoint(pickPoint, Color::RED(), 6.0f);

			if (MOON_InputManager::IsMouseDown(0)) {
				sp->transform.position = pickPoint;
				pickFlag[0] = false;
				break;
			}
			yield_return();
		}
		while (pickFlag[1] && HotKeyManager::state == CREATE) {
			if (MOON_InputManager::mouse_left_hold) {
				auto newPoint = MOON_InputManager::PickGroundPoint(MOON_MousePos);
				sp->size.setValue(
					std::fabs(sp->transform.position.x - newPoint.x),
					0,
					std::fabs(sp->transform.position.z - newPoint.z)
				);
				Utility::ReleaseVector(sp->meshList);
				sp->meshList.push_back(GenerateMesh(sp->size, sp->segment));
			} else if (MOON_InputManager::IsMouseRelease(0)) {
				pickFlag[1] = false;
				break;
			}
			yield_return();
		}
		while (HotKeyManager::state == CREATE) {
			if(MOON_InputManager::IsMouseDown(0)) {
				return;
			} else {
				auto newPoint = MOON_InputManager::PickGroundPoint(MOON_MousePos);
				Vector3 checkPoint = sp->transform.position + Vector3(sp->size.x, 0, sp->size.z);
				checkPoint.x = newPoint.x;
				float len = newPoint.distance(checkPoint);
				float ang = Vector3::Angle(
					checkPoint - newPoint,
					MOON_ActiveCamera->transform.position - newPoint
				);
				sp->size.y = len * std::tanf(ang);
				Utility::ReleaseVector(sp->meshList);
				sp->meshList.push_back(GenerateMesh(sp->size, sp->segment));
			}
			yield_return();
		}

		MOON_ModelManager::DeleteItem(sp->ID);
	}

	void Box::CreateProcedural(const bool& interactive) {
		if (interactive) {
			Coroutine::ptr co = Coroutine::create_coroutine(InteractiveCreate, this);
		} else {
			Utility::ReleaseVector(meshList);
			meshList.push_back(GenerateMesh(size, segment));
		}
	}

	void Box::ListProceduralProperties() {
		ImGui::Text("Box:");
		ImGui::Indent(10.0f);
		ImGui::Text("Size"); ImGui::SameLine(80);
		if (ImGui::DragFloat3(UniquePropName("size"), (float*)&size, 0.1f, 0, INFINITY, "%.3f", 1.0f, true)) {
			CreateProcedural(false);
		}
		ImGui::Text("Segment"); ImGui::SameLine(80);
		int seg[3] = { segment.x, segment.y, segment.z };
		if (ImGui::DragInt2(UniquePropName("segment"), seg, 1.0f, 1, 255, "%d", true)) {
			segment.setValue(seg[0], seg[1], seg[2]);
			CreateProcedural(false);
		}
		ImGui::Unindent(10.0f);
	}

	Mesh* Box::GenerateMesh(Vector3 size, Vector3 segment) {
		std::vector<Vertex> verts;
		std::vector<unsigned int> tris;

		verts.resize(24); tris.resize(36);

		// forward
		verts[0].Position.setValue(size.x, 0, size.z);
		verts[1].Position.setValue(-size.x, 0, size.z);
		verts[2].Position.setValue(size.x, size.y, size.z);
		verts[3].Position.setValue(-size.x, size.y, size.z);

		for (int i = 0; i < 4; i++) verts[i].Normal.setValue(Vector3::WORLD(FORWARD));

		// back
		verts[4].Position.setValue(verts[2].Position.x, verts[2].Position.y, -size.z);
		verts[5].Position.setValue(verts[3].Position.x, verts[3].Position.y, -size.z);
		verts[6].Position.setValue(verts[0].Position.x, verts[0].Position.y, -size.z);
		verts[7].Position.setValue(verts[1].Position.x, verts[1].Position.y, -size.z);

		for (int i = 4; i < 8; i++) verts[i].Normal.setValue(Vector3::WORLD(BACKWARD));

		// up
		verts[8] = verts[2];
		verts[9] = verts[3];
		verts[10] = verts[4];
		verts[11] = verts[5];

		for (int i = 8; i < 12; i++) verts[i].Normal.setValue(Vector3::WORLD(UP) * MoonMath::GetSign(size.y));

		// down
		verts[12].Position.setValue(verts[10].Position.x, 0, verts[10].Position.z);
		verts[13].Position.setValue(verts[11].Position.x, 0, verts[11].Position.z);
		verts[14].Position.setValue(verts[8].Position.x, 0, verts[8].Position.z);
		verts[15].Position.setValue(verts[9].Position.x, 0, verts[9].Position.z);

		for (int i = 12; i < 16; i++) verts[i].Normal.setValue(Vector3::WORLD(DOWN) * MoonMath::GetSign(size.y));

		// left
		verts[16] = verts[6]; 
		verts[17] = verts[0]; 
		verts[18] = verts[4]; 
		verts[19] = verts[2];

		for (int i = 16; i < 20; i++) verts[i].Normal.setValue(Vector3::WORLD(LEFT));

		// right
		verts[20].Position.setValue(-size.x, verts[18].Position.y, verts[18].Position.z);
		verts[21].Position.setValue(-size.x, verts[19].Position.y, verts[19].Position.z);
		verts[22].Position.setValue(-size.x, verts[16].Position.y, verts[16].Position.z);
		verts[23].Position.setValue(-size.x, verts[17].Position.y, verts[17].Position.z);

		for (int i = 20; i < 24; i++) verts[i].Normal.setValue(Vector3::WORLD(RIGHT));

		// tris
		for (int i = 0, cnt = 0; i < 24; i = i + 4) {
			tris[cnt++] = i; tris[cnt++] = i + 3; tris[cnt++] = i + 1;
			tris[cnt++] = i; tris[cnt++] = i + 2; tris[cnt++] = i + 3;
		}

		Mesh* mesh = new Mesh("sphere", verts, tris);
		mesh->material = MOON_MaterialManager::GetItem("default");
		mesh->UpdateBBox();
		return mesh;
	}

}