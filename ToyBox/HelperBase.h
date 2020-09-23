#pragma once
#include "ObjectBase.h"
#include "Spline.h"

namespace MOON {
	class Helper : public MObject {
	public:
		std::vector<Spline> splineList;

		Helper(const std::string &name, const int id = MOON_AUTOID) : MObject(name, id) {}
		virtual ~Helper() override = default;

		void ListProperties() override {
			// list name ----------------------------------------------------------------------
			ListName();
			ImGui::Separator();

			// list transform -----------------------------------------------------------------
			ListTransform();
			ImGui::Separator();

			// list Splines -------------------------------------------------------------------
			ListHelperPropties();
			ImGui::Separator();

			// list operators -----------------------------------------------------------------
			opstack.ListStacks();
			ImGui::Spacing();
		}

		virtual void Draw(Shader* overrideShader = NULL) override {
			for (auto sp : splineList) {
				sp.Draw(transform.localToWorldMat, wireColor, overrideShader);
			}
		}

		virtual void ListHelperPropties() {
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
}