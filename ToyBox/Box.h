#pragma once
#include "Mesh.h"
#include "Model.h"
#include "Vector3.h"

namespace MOON {

	class Box : public Model {
	public:
		Vector3 size;
		Vector3 segment;

		void CreateProceduralMesh(const bool& interactive) override;
		void ListProceduralProperties() override;

		Box(const std::string &name, const bool& interactive = false,
			Vector3 size = Vector3::ONE(), Vector3 segment = Vector3::ONE(),
			const int id = MOON_AUTOID) :
			Model(name, id), size(size), segment(segment) {
			CreateProceduralMesh(interactive);
		}

		static void InteractiveCreate(void* arg);
		static Mesh* GenerateMesh(Vector3 size, Vector3 segment);
	};

}