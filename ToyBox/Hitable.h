#pragma once
#include "Ray.h"
#include "MathUtils.h"
#include "Vector3.h"
#include "Vector2.h"

namespace moon {
	extern class Material;

	struct HitRecord {
		float t; // distance between camera and target (hited by the ray)
		Vector3 p;
		Vector3 normal;
		Vector2 uv;
		Material *mat;

		HitRecord(): t(INFINITY) {}
	};

	class Hitable {
	public:
		virtual bool Hit(const Ray &t, HitRecord &rec) const = 0;
	};
}