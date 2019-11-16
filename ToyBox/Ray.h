#pragma once

#include "Vector3.h"

namespace moon {
	class Ray {
	public:
		Vector3 pos;
		Vector3 dir;

		Ray() {}
		Ray(const Vector3 &position, const Vector3 &direction) {
			pos = position;
			dir = direction;
		}

		inline Vector3 PointAtParameter(float dist) {
			return pos + dist * dir;
		}
	};
}