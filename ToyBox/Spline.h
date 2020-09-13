#pragma once
#include <vector>
#include <iostream>
#include <cmath>
#include <string>

#include "Icons.h"
#include "ButtonEx.h"
#include "MoonEnums.h"
#include "ObjectBase.h"
#include "Vector3.h"
#include "Transform.h"
#include "Matrix4x4.h"
#include "MathUtils.h"
#include "Utility.h"
#include "Color.h"
#include "Curves.h"

namespace MOON {
	enum SplineType {
		CORNER,
		BEZIER,
		BSPLINE,
		NURBS,
		CATMULL
	};

	class Spline : public ObjectBase {
	public:
		SplineType type;
		int knotNum;
		int smooth;
		bool constraint;
		float width;

		std::vector<Vector3> knotList;
		std::vector<Vector3> interpKnots;
		std::vector<Vector3> controllerList;

		std::vector<Vector3> normalList;
		std::vector<Vector3> tangentList;
		std::vector<Vector3> bitangentList;

		Spline(const std::string &name, const SplineType &type) : ObjectBase(name, MOON_UNSPECIFIEDID), 
			width(1.0f), smooth(20), constraint(true), knotNum(2) {
			this->type = type;
		}

		Spline(const std::string &name, const SplineType &type, const std::vector<Vector3> &data) :
			ObjectBase(name, MOON_UNSPECIFIEDID), width(1.0f), smooth(20), constraint(true), knotNum(2) {
			this->knotList = data;
			this->type = type;
		}

		std::vector<Vector3>& GetKnots() {
			if (!type) return knotList;
			else return interpKnots;
		}
		std::vector<Vector3>& GetNormal() {
			return normalList;
		}
		std::vector<Vector3>& GetTangent() {
			return tangentList;
		}
		std::vector<Vector3>& GetBitangent() {
			return bitangentList;
		}

		void Update() {
			SynchronizeKnotList(knotNum - knotList.size());
			RecalculateVectors();
			SynchronizeControllerList();
			Interpolate(type);
		}

		void Interpolate(const SplineType &type) {
			if (!type) return;
			interpKnots.clear();
			if (type == BEZIER) {
				Curves::CubicBezier::ComputeCurve(
					knotList, 
					controllerList, 
					smooth, 
					interpKnots
				);
			} else if (type == BSPLINE) {

			} else if (type == NURBS) {

			} else if (type == CATMULL) {

			}
			RecalculateVectors(true);
		}

		void RecalculateVectors(const bool &isInterp = false) {
			normalList.clear(); tangentList.clear(); bitangentList.clear();
			MoonMath::CalculatePathVector(
				isInterp ? interpKnots : knotList,
				normalList, tangentList, bitangentList
			);
		}

		void SynchronizeKnotList(int deltaNum) {
			if (!deltaNum) return;
			else if (deltaNum > 0) {
				while (deltaNum--) knotList.push_back(
					knotList[knotList.size() - 1] + 
					Vector3::Normalize(
						GetKnots()[GetKnots().size() - 1] -
						GetKnots()[GetKnots().size() - 2]
					)
				);
			} else {
				deltaNum *= -1;
				while (deltaNum--) knotList.pop_back();
			}
		}

		void SynchronizeControllerList() {
			int deltaNum = (knotNum << 1) - 2 - controllerList.size();
			if (!deltaNum) return; deltaNum >>= 1;
			if (deltaNum > 0) {
				int currLast = controllerList.size() + 1;
				while (deltaNum--) {
					controllerList.push_back(knotList[currLast >> 1] * 2 - controllerList[currLast - 2]);
					controllerList.push_back(knotList[(currLast >> 1) + 1] - tangentList[(currLast >> 1) + 1] * 0.5f);
				}
			} else {
				deltaNum *= -1;
				while (deltaNum--) {
					controllerList.pop_back();
					controllerList.pop_back();
				}
			}
		}

		void ListParameters() {
			const char* items[] = { "Corner", "Bezier", "B-Spline", "NURBS" };
			ImGui::PushItemWidth(90.0f);
			if (ImGui::Combo("Type", (int*)&type, items, IM_ARRAYSIZE(items))) {
				Update();
			}

			if (ImGui::InputInt("knotNum", &knotNum, 1, 1)) {
				Constraint(knotNum, 2, INFINITY_INT);
				Update();
			}
			ImGui::DragFloat("width", &width, 0.1f, 1.0f, 10.0f);
			if (type && ImGui::DragInt("smooth", &smooth, 1.0f, 1, 255)) {
				Update();
			}
			ImGui::PopItemWidth();
		}

		void ListKnots() {
			if (ImGui::TreeNode("Knots")) {
				ImGui::PushItemWidth(130.0f);
				for (int i = 0; i < knotList.size(); i++) {
					Vector3 oldPos = knotList[i];
					if (ImGui::DragFloat3(to_string_precision(i, 0, 3).c_str(), (float*)&knotList[i], 0.1f)) {
						Update();
						Vector3 deltaPos = knotList[i] - oldPos;
						if (i == 0) {
							controllerList[0] += deltaPos;
						} else if (i == knotList.size() - 1) {
							controllerList[controllerList.size() - 1] += deltaPos;
						} else {
							controllerList[(i << 1) - 1] += deltaPos;
							controllerList[i << 1] += deltaPos;
						}
					}
				}
				ImGui::PopItemWidth();
				ImGui::TreePop();
			}
		}

		void ListControllers() {
			if (!type) return;
			if (type == BEZIER) {
				ImGui::Indent(100.0f);
				SwitchButton("Constraint", "FreeHandle", constraint, ImVec2(75, 22));
				ImGui::Unindent(100.0f);
				ImGui::SameLine(50.0f);
				//ImGui::Checkbox("constraint", &constraint);
			}
			if (ImGui::TreeNode("Controllers")) {
				ImGui::PushItemWidth(130.0f);
				for (int i = 0; i < controllerList.size(); i++) {
					if (ImGui::DragFloat3(std::to_string(i).c_str(), (float*)&controllerList[i], 0.1f)) {
						if (constraint && i > 0 && i < controllerList.size() - 1) {
							int cid = i % 2 == 0 ? i - 1 : i + 1;
							int kid = i % 2 == 0 ? i / 2 : (i + 1) / 2;
							controllerList[cid] = knotList[kid] * 2.0f - controllerList[i];
						}
						Interpolate(type);
						RecalculateVectors(true);
					}
				}
				ImGui::PopItemWidth();
				ImGui::TreePop();
			}
		}

		void Draw(const Matrix4x4 &model, const Vector4 &color = Color::WHITE(), const Shader *overrideShader = NULL) {
			if (knotList.size() < 1) return;
			Gizmo::DrawLines(type ? interpKnots : knotList, color, overrideShader == NULL ? width : std::max(5.0f, width), true, model, overrideShader);
			/*if (overrideShader == NULL) {
				Gizmo::DrawPoints(knotList, Color::BLUE(), 5.0f, model);
				if (type == BEZIER) {
					std::vector<Vector3> controlLines;
					for (int i = 0, ctrl = 0; i < knotNum - 1; i++) {
						controlLines.push_back(knotList[i]);
						controlLines.push_back(controllerList[ctrl]);
						controlLines.push_back(knotList[i + 1]);
						controlLines.push_back(controllerList[ctrl + 1]);
						ctrl += 2;
					}
					Gizmo::DrawLines(controlLines, Color::RED(), 1.0f, false, model);
					Gizmo::DrawPoints(controllerList, Color::RED(), 5.0f, model);
				}
			}*/
		}

		void DrawHandles(const Matrix4x4 &model, const Vector4 &color = Color::WHITE()) {
			if (knotList.size() < 1) return;
			Gizmo::DrawPoints(knotList, Color::BLUE(), 5.0f, model);
			if (type == BEZIER) {
				std::vector<float> controlLines;
				for (int i = 0, ctrl = 0; i < knotNum - 1; i++) {
					controlLines.push_back(knotList[i].x);
					controlLines.push_back(knotList[i].y);
					controlLines.push_back(knotList[i].z);
					controlLines.push_back(controllerList[ctrl].x);
					controlLines.push_back(controllerList[ctrl].y);
					controlLines.push_back(controllerList[ctrl].z);

					controlLines.push_back(knotList[i + 1].x);
					controlLines.push_back(knotList[i + 1].y);
					controlLines.push_back(knotList[i + 1].z);
					controlLines.push_back(controllerList[ctrl + 1].x);
					controlLines.push_back(controllerList[ctrl + 1].y);
					controlLines.push_back(controllerList[ctrl + 1].z);
					ctrl += 2;
				}
				Gizmo::DrawLinePrototype(controlLines, Color::RED(), 1.0f, false, model);
				Gizmo::DrawPoints(controllerList, Color::RED(), 5.0f, model);
			}
		}

		void DebugVectors(const Matrix4x4 &model) {
			if (knotList.size() < 1) return;
			std::vector<Vector3>& list = GetKnots();
			std::vector<Vector3> norm, tan, bitan;
			for (int i = 0; i < list.size(); i++) {
				norm.push_back(list[i]); norm.push_back(list[i] + normalList[i] * 0.5f);
				tan.push_back(list[i]); tan.push_back(list[i] + tangentList[i] * 0.5f);
				bitan.push_back(list[i]); bitan.push_back(list[i] + bitangentList[i] * 0.5f);
			}
			Gizmo::DrawLines(norm, Color::BLUE(), 1.0f, false, model);
			Gizmo::DrawLines(tan, Color::RED(), 1.0f, false, model);
			Gizmo::DrawLines(bitan, Color::GREEN(), 1.0f, false, model);
		}
	};
}
