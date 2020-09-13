#pragma once
#include "Mesh.h"
#include "Model.h"
#include "Vector3.h"

namespace MOON {

	class Capsule : public Model {
	public:
		Vector2 size;
		Vector2 segment;

		void CreateProceduralMesh(const bool& interactive) override;
		void ListProceduralProperties() override;

		Capsule(const std::string &name, const bool& interactive = false,
			Vector2 size = Vector2::ONE(), Vector2 segment = Vector2(3, 3),
			const int id = MOON_AUTOID) :
			Model(name, id), size(size), segment(segment) {
			CreateProceduralMesh(interactive);
		}

		static void InteractiveCreate(void* arg);
		static Mesh* GenerateMesh(Vector2 size, Vector2 segment);
	};

}