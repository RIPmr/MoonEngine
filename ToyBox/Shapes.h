#pragma once
#include "Icons.h"
#include "ObjectBase.h"
#include "Spline.h"

namespace MOON {
	class Shape : public MObject {
	public:
		std::vector<Spline> splineList;

		Shape(const std::string &name, const int id = MOON_AUTOID) : MObject(name, id) { }
		~Shape() override = default;

		void ListProperties() override {
			// list name ----------------------------------------------------------------------
			ListName();
			ImGui::Separator();

			// list transform -----------------------------------------------------------------
			ListTransform();
			ImGui::Separator();

			// list Splines -------------------------------------------------------------------
			ListSplines();
			ImGui::Separator();

			// list operators -----------------------------------------------------------------
			opstack.ListStacks();
			ImGui::Spacing();
		}

		void AddSpline(const std::string &name, const SplineType &type) {
			splineList.push_back(Spline(name, type));
		}

		void Draw(Shader* overrideShader = NULL) override {
			for (auto sp : splineList) {
				sp.Draw(transform.localToWorldMat, wireColor, overrideShader);
				if (selected && overrideShader == NULL) 
					sp.DrawHandles(transform.localToWorldMat, wireColor);
			}
		}

		void DebugVectors() {
			for (auto sp : splineList) {
				sp.DebugVectors(transform.localToWorldMat);
			}
		}

	private:
		void ListSplines() {
			ImGui::Text("Spline:");
			if (ImGui::TreeNodeEx((std::to_string(splineList.size()) + " spline(s)").c_str(), ImGuiTreeNodeFlags_DefaultOpen)) {
				for (auto &iter : splineList) {
					if (ImGui::TreeNodeEx(iter.name.c_str(), ImGuiTreeNodeFlags_DefaultOpen)) {
						iter.ListParameters();
						iter.ListKnots();
						iter.ListControllers();
						ImGui::TreePop();
					}
				}
				ImGui::TreePop();
			}
		}
	};

	class Line : public Shape {
	public:
		Line(const std::string &name, const int id = MOON_AUTOID) : Shape(name, id) {
			splineList.push_back(Spline(("spline_" + std::to_string(splineList.size())).c_str(), CORNER));
			splineList[0].knotList.push_back(Vector3::ZERO());
			splineList[0].knotList.push_back(Vector3(1, 0, 0));
			splineList[0].controllerList.push_back(Vector3(0.25f, 0, 0));
			splineList[0].controllerList.push_back(Vector3(0.75f, 0, 0));
			splineList[0].Update();
		}
		~Line() override = default;
	};

	class Circle : public Shape {
	public:
		float radius;

	};

	class Rectangle : public Shape {
	public:
		Vector2 size;
		float rounded;

	};

	class NGone : public Shape {
	public:
		unsigned int sides;
		float theta;

	};

}