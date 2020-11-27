#pragma once
#include "Mesh.h"
#include "Model.h"

namespace MOON {

	class Sphere : public Model {
	public:
		float radius;
		int segment;

		void CreateProcedural(const bool& interactive) override;
		void ListProceduralProperties() override;

		Sphere(const std::string &name, const bool& interactive = false, 
			float radius = 1.0f, int segment = 24, const int id = MOON_AUTOID) : 
			Model(name, id), radius(radius), segment(segment) {
			CreateProcedural(interactive);
		}

		~Sphere() override {};

		static void InteractiveCreate(void* arg);
		static Mesh* GenerateMesh(float rad, int divs);
	};

}