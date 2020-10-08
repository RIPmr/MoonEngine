#pragma once
#include "Ray.h"
#include "Vector2.h"
#include "Vector3.h"
#include "MathUtils.h"
#include "BoundingBox.h"

namespace MOON {
	extern class Material;

	struct HitRecord {
		float t; // distance between camera and target (hited by the ray)
		Vector3 p;
		Vector3 normal;
		Vector2 uv;
		Material *mat;

		HitRecord(): t(INFINITY) {}
		HitRecord(const HitRecord& rec) : t(rec.t), p(rec.p), normal(rec.normal), uv(rec.uv), mat(rec.mat) {}
	};

	class Hitable {
	public:
		BoundingBox bbox;

		Hitable() {}
		Hitable(const BoundingBox& box) : bbox(box) {}
		virtual ~Hitable() = default;

		virtual bool Hit(const Ray &t, HitRecord &rec) const { return true; };
		virtual bool bounding_box(Vector2 timeStamp, BoundingBox& box) const {
			box = bbox;
			return true;
		}
	};

}