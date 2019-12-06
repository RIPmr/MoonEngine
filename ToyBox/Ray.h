#pragma once

#include "Vector3.h"

namespace MOON {
	class Ray {
	public:
		Vector3 pos;
		Vector3 dir;

		Ray() {}
		Ray(const Ray &r) : pos(r.pos), dir(r.dir) {}
		Ray(const Vector3 &position, const Vector3 &direction) {
			pos = position;
			dir = direction;
		}

		friend std::ostream& operator<<(std::ostream &os, const Ray &r) {
			os << "pos: " << r.pos << ", dir: " << r.dir;
			return os;
		}

		inline Vector3 PointAtParameter(float dist) const {
			return pos + dist * dir;
		}
	};
}