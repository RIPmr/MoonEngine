#include "MathUtils.h"
#include "Matrix4x4.h"

namespace MOON {

	Vector3 MoonMath::RotateAround(const Vector3 &position, const Vector3 &center, const Vector3 &axis, const float &angle) {
		Vector3 point = Matrix4x4::RotateMat(angle, axis).multVec(position - center);
		Vector3 resultVec3 = center + point;
		return resultVec3;
	}

	float MoonMath::closestDistanceBetweenLines(const Vector3 &a0, const Vector3 &a1, const Vector3 &b0, const Vector3 &b1, Vector3 &Line1Closest, Vector3 &Line2Closest) {
		// Based on https://stackoverflow.com/questions/2824478/shortest-distance-between-two-line-segments
		float Distance;

		// Calculate denomitator
		auto A = a1 - a0;
		auto B = b1 - b0;
		float magA = A.magnitude();
		float magB = B.magnitude();

		auto _A = A / magA;
		auto _B = B / magB;

		auto cross = Vector3::Cross(_A, _B);
		auto denom = cross.magnitude() * cross.magnitude();

		// If lines are parallel (denom=0) test if lines overlap.
		// If they don't overlap then there is a closest point solution.
		// If they do overlap, there are infinite closest positions, but there is a closest distance
		if (denom == 0) {
			auto d0 = Vector3::Dot(_A, (b0 - a0));

			// Overlap only possible with clamping
			auto d1 = Vector3::Dot(_A, (b1 - a0));

			// Is segment B before A?
			if (d0 <= 0 && 0 >= d1) {
				if (abs(d0) < abs(d1)) {
					Line1Closest = a0;
					Line2Closest = b0;
					Distance = (a0 - b0).magnitude();

					return Distance;
				}
				Line1Closest = a0;
				Line2Closest = b1;
				Distance = (a0 - b1).magnitude();

				return Distance;
			}
			// Is segment B after A?
			else if (d0 >= magA && magA <= d1) {
				if (abs(d0) < abs(d1)) {
					Line1Closest = a1;
					Line2Closest = b0;
					Distance = (a1 - b0).magnitude();

					return Distance;
				}
				Line1Closest = a1;
				Line2Closest = b1;
				Distance = (a1 - b1).magnitude();

				return Distance;
			}

			// Segments overlap, return distance between parallel segments
			Line1Closest = Vector3::ZERO();
			Line2Closest = Vector3::ZERO();
			Distance = (((d0 * _A) + a0) - b0).magnitude();
			return Distance;
		}

		// Lines criss-cross: Calculate the projected closest points
		auto t = (b0 - a0);
		auto detA = Determinant(t, _B, cross);
		auto detB = Determinant(t, _A, cross);

		auto t0 = detA / denom;
		auto t1 = detB / denom;

		auto pA = a0 + (_A * t0); // Projected closest point on segment A
		auto pB = b0 + (_B * t1); // Projected closest point on segment B

		// Clamp projections
		if (t0 < 0) pA = a0;
		else if (t0 > magA) pA = a1;

		if (t1 < 0) pB = b0;
		else if (t1 > magB) pB = b1;

		float dot;
		// Clamp projection A
		if (t0 < 0 || t0 > magA) {
			dot = Vector3::Dot(_B, (pA - b0));
			if (dot < 0) dot = 0;
			else if (dot > magB) dot = magB;
			pB = b0 + (_B * dot);
		}
		// Clamp projection B
		if (t1 < 0 || t1 > magB) {
			dot = Vector3::Dot(_A, (pB - a0));
			if (dot < 0) dot = 0;
			else if (dot > magA) dot = magA;
			pA = a0 + (_A * dot);
		}

		Line1Closest = pA;
		Line2Closest = pB;
		Distance = (pA - pB).magnitude();

		return Distance;
	}

}