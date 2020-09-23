#pragma once
#include "HelperBase.h"

namespace MOON {

	class Dummy : public Helper {
	public:
		bool drawBox;
		bool drawCross;

		float displaySize;

		Dummy(const std::string &name, const int id = MOON_AUTOID) : Helper(name, id) {
			this->drawBox = true;
			this->drawCross = false;
			this->displaySize = 1.0f;
			this->wireColor = Vector4(0, 0.8f, 0, 1);

			#pragma region generate_dummy_shape
			// box: 0~3
			splineList.push_back(Spline("box_base", 
				SplineType::CORNER, {
					Vector3( 0.5f, -0.5f,  0.5f),
					Vector3(-0.5f, -0.5f,  0.5f),
					Vector3(-0.5f, -0.5f, -0.5f),
					Vector3( 0.5f, -0.5f, -0.5f),
					Vector3( 0.5f, -0.5f,  0.5f),
					Vector3( 0.5f,  0.5f,  0.5f)
				}
			));
			splineList.push_back(Spline("box_top",
				SplineType::CORNER, {
					Vector3( 0.5f,  0.5f, -0.5f),
					Vector3(-0.5f,  0.5f, -0.5f),
					Vector3(-0.5f,  0.5f,  0.5f),
					Vector3( 0.5f,  0.5f,  0.5f),
					Vector3( 0.5f,  0.5f, -0.5f),
					Vector3( 0.5f, -0.5f, -0.5f)
				}
			));
			splineList.push_back(Spline("left_column",
				SplineType::CORNER, {
					Vector3(-0.5f, -0.5f,  0.5f),
					Vector3(-0.5f,  0.5f,  0.5f)
				}
			));
			splineList.push_back(Spline("right_column",
				SplineType::CORNER, {
					Vector3(-0.5f,  0.5f, -0.5f),
					Vector3(-0.5f, -0.5f, -0.5f)
				}
			));

			// cross: 4~6
			splineList.push_back(Spline("cross_forward",
				SplineType::CORNER, {
					Vector3(0, 0, -1.0f),
					Vector3(0, 0,  1.0f)
				}
			));
			splineList.push_back(Spline("cross_left",
				SplineType::CORNER, {
					Vector3(-1.0f, 0, 0),
					Vector3(1.0f, 0, 0)
				}
			));
			splineList.push_back(Spline("cross_up",
				SplineType::CORNER, {
					Vector3(0, -1.0f, 0),
					Vector3(0,  1.0f, 0)
				}
			));
			#pragma endregion
		}

		~Dummy() override = default;

		void Draw(Shader* overrideShader = NULL) override {
			if (drawBox) {
				for (int i = 0; i < 4; i++)
					splineList[i].Draw(
						transform.localToWorldMat, 
						wireColor, overrideShader
					);
			}
			if (drawCross) {
				for (int i = 4; i < 7; i++)
					splineList[i].Draw(
						transform.localToWorldMat,
						wireColor, overrideShader
					);
			}
		}

		void ListHelperPropties() override {
			ImGui::Text("Dummy:");
			ImGui::Indent(10.0f);

			ImGui::Text("WireBox:"); ImGui::SameLine(80.0f);
			ImGui::Checkbox("box", &drawBox, true);

			ImGui::Text("Cross:"); ImGui::SameLine(80.0f);
			ImGui::Checkbox("cross", &drawCross, true);
			ImGui::Unindent(10.0f);
		}
	};

}