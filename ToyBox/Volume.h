#pragma once
#include "MShader.h"
#include "Hitable.h"
#include "ObjectBase.h"

namespace MOON {

	class Volume : public MObject, public Hitable {
	public:
		enum SourceType {
			Noise,
			Texture,
			OpenVDB,
			Alembic
		};

		SourceType source;

		bool drawBound;
		bool useLight;
		bool writeDepth;

		float step;
		float rayStep;
		int lightStep;
		int maxMarchLoop;

		float darknessThreshold;
		float midtoneOffset;
		float shadowOffset;
		float lightAbsorptionTowardSun;
		float lightAbsorptionThroughCloud;

		Vector3 mainColor;
		Vector3 shadowColor;
		Vector4 phaseParams;

		float scatterMultiply;
		float densityOffset;
		float densityMultiply;

		int downSampling;
		Vector3 offset;
		Vector2 multiply;
		float scale;

		Volume(const std::string &name, const bool& interactive = false, 
			const int id = MOON_AUTOID) : MObject(name, id) {
			source = Noise; drawBound = true; useLight = true; writeDepth = false;

			step = 0.1f; rayStep = 0.1f; lightStep = 8; maxMarchLoop = 128;

			darknessThreshold = 0.0f; midtoneOffset = 0.59f; shadowOffset = 1.02f;
			lightAbsorptionTowardSun = 0.1f;
			lightAbsorptionThroughCloud = 1.0f;

			mainColor.setValue(0.5f, 0.5f, 0.5f); shadowColor.setValue(0.1f, 0.1f, 0.1f);
			phaseParams.setValue(0.72f, 1.0f, 0.5f, 1.58f);

			scatterMultiply = 1.0f;
			densityOffset = 4.02f;
			densityMultiply = 2.3f;

			downSampling = 4;
			offset.setValue(0.0f, 0.0f, 0.0f);
			scale = 8.0f;
			multiply.setValue(2.0f, 4.0f);

			CreateProcedural(interactive);
		}
		virtual ~Volume() override = default;

		virtual void ListProperties() override {
			// list name ----------------------------------------------------------------------
			ListName();
			ImGui::Separator();

			// list transform -----------------------------------------------------------------
			ListTransform();
			ImGui::Separator();

			// list properties ----------------------------------------------------------------
			ListVolumePropties();
			ImGui::Separator();

			// list operators -----------------------------------------------------------------
			opstack.ListStacks();
			ImGui::Spacing();
		}

		static void InteractiveCreate(void* arg);
		virtual void Draw(Shader* overrideShader = NULL) override;

		void ListVolumePropties();
		void SetupParameters(Shader* rayMarchingShader);
		void CreateProcedural(const bool& interactive) override;
	};

}
