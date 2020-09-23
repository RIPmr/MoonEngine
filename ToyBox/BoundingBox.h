#pragma once
#include <cmath>
#include <algorithm>
#include <vector>

#include "MathUtils.h"
#include "Vector3.h"
#include "properties.h"
#include "Ray.h"

namespace MOON {
	class BoundingBox {
	public:
		Vector3 min;
		Vector3 max;
		//Vector3 center;
		//Vector3 extend;

		PROPERTY(Vector3, center);
		GET(center) {
			return (min + max) / 2.0f;
		}

		BoundingBox() {
			min = Vector3(INFINITY, INFINITY, INFINITY);
			max = Vector3(-INFINITY, -INFINITY, -INFINITY);
		}
		BoundingBox(const Vector3 &min_, const Vector3 &max_) {
			min = min_;
			max = max_;
		}
		BoundingBox(const BoundingBox &bbox) {
			min = bbox.min;
			max = bbox.max;
		}
		~BoundingBox() = default;

		Vector3& operator[](bool i) { return i == 0 ? min : max; }
		const Vector3 operator[](bool i) const { return i == 0 ? min : max; }

		/*
			  v7-----v4
             /|      /|
            v6-----v5 |
            | v1----|v2
            |/      |/
            v0-----v3
		*/
		inline void GetCorners(std::vector<Vector3>* vec) const {
			vec->push_back(min);
			vec->push_back(Vector3(min.x, min.y, max.z));
			vec->push_back(Vector3(max.x, min.y, max.z));
			vec->push_back(Vector3(max.x, min.y, min.z));

			vec->push_back(max);
			vec->push_back(Vector3(max.x, max.y, min.z));
			vec->push_back(Vector3(min.x, max.y, min.z));
			vec->push_back(Vector3(min.x, max.y, max.z));
		}

		inline void Reset() {
			min.setValue(INFINITY, INFINITY, INFINITY);
			max.setValue(-INFINITY, -INFINITY, -INFINITY);
		}

		inline BoundingBox& join(const Vector3& p) {
			min.x = std::min(p.x, min.x);
			min.y = std::min(p.y, min.y);
			min.z = std::min(p.z, min.z);

			max.x = std::max(p.x, max.x);
			max.y = std::max(p.y, max.y);
			max.z = std::max(p.z, max.z);

			return *this;
		}

		inline BoundingBox& join(const BoundingBox& bbox) {
			min.x = std::min(bbox.min.x, min.x);
			min.y = std::min(bbox.min.y, min.y);
			min.z = std::min(bbox.min.z, min.z);

			max.x = std::max(bbox.max.x, max.x);
			max.y = std::max(bbox.max.y, max.y);
			max.z = std::max(bbox.max.z, max.z);

			return *this;
		}

		inline bool intersect(const Ray& ray) const {
			const Vector3& orig = ray.pos;
			const Vector3  invDir = 1.0f / ray.dir;
			const Vector3  sign(ray.dir.x < 0, ray.dir.y < 0, ray.dir.z < 0);
			
			float tmin, tmax, tymin, tymax, tzmin, tzmax;

			tmin = ((*this)[sign[0]].x - orig.x) * invDir.x;
			tmax = ((*this)[1 - sign[0]].x - orig.x) * invDir.x;
			tymin = ((*this)[sign[1]].y - orig.y) * invDir.y;
			tymax = ((*this)[1 - sign[1]].y - orig.y) * invDir.y;

			if ((tmin > tymax) || (tymin > tmax)) return false;

			if (tymin > tmin) tmin = tymin;
			if (tymax < tmax) tmax = tymax;

			tzmin = ((*this)[sign[2]].z - orig.z) * invDir.z;
			tzmax = ((*this)[1 - sign[2]].z - orig.z) * invDir.z;

			if ((tmin > tzmax) || (tzmin > tmax)) return false;

			if (tzmin > tmin) tmin = tzmin;
			if (tzmax < tmax) tmax = tzmax;

			//tHit = tmin;

			return true;
		}
	};
}