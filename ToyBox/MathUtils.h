#pragma once
#include <cmath>
#include <vector>
#include <cstdlib>
#include <algorithm>

#include "Vector2.h"
#include "Vector3.h"
#include "Ray.h"

namespace MOON {
	#define MOON_m 0x100000000LL
	#define MOON_c 0xB16
	#define MOON_a 0x5DEECE66DLL
	/// actually it's pow(255, 4) = 0xfc05fc01
	/// but range of int is ¡À2147483647, smaller than 0xfc05fc01
	#define MOON_IDBOUNDARY std::numeric_limits<int>::max()
	#define PI acos(-1.0)
	#define Rad2Deg 180/PI
	#define Deg2Rad PI/180
	#define INFINITY std::numeric_limits<float>::max()
	#define EPSILON 0.001

	class MoonMath {
	private:
		static unsigned long long seed;
	public:

		inline static std::vector<int> TENtoNBase(int num, int n) {
			std::vector<int> v;
			while (num) {
				v.push_back(num % n);
				num /= n;
			}
			return v;
		}

		inline static void GammaCorrection(Vector3 &GammaSpaceCol) {
			GammaSpaceCol.x = sqrt(GammaSpaceCol.x);
			GammaSpaceCol.y = sqrt(GammaSpaceCol.y);
			GammaSpaceCol.z = sqrt(GammaSpaceCol.z);
		}

		inline static double drand48() {
			seed = (MOON_a * seed + MOON_c) & 0xFFFFFFFFFFFFLL;
			unsigned int x = seed >> 16;
			return  ((double)x / (double)MOON_m);
		}

		inline static void srand48(unsigned int i) {
			seed = (((long long int)i) << 16) | rand();
		}

		inline static float clamp(const float &num, const float &min, const float &max) {
			return std::max(min, std::min(max, num));
		}

		inline static Vector3 RandomInUnitSphere() { // get a random vector inside unit sphere
			Vector3 p;
			do {
				p = 2.0 * Vector3(drand48(), drand48(), drand48()) - Vector3::ONE();
			} while (p.MagSquared() >= 1.0);
			return p;
		}

		inline static Vector3 RandomInUnitDisk() {
			Vector3 p;
			do {
				p = 2.0 * Vector3(drand48(), drand48(), 0) - Vector3(1, 1, 0);
			} while (p.dot(p) >= 1.0);
			return p;
		}

		inline static Vector3 Reflect(const Vector3 &v, const Vector3 &n) {
			return v - 2 * v.dot(n) * n;
		}

		inline static bool Refract(const Vector3 &v, const Vector3 &n, const float &ni_over_nt, Vector3 &refracted) {
			Vector3 uv = Vector3::Normalize(v);
			float dt = uv.dot(n);
			float discriminant = 1.0 - pow(ni_over_nt, 2) * (1 - pow(dt, 2));
			if (discriminant > 0) {
				refracted = ni_over_nt * (uv - n * dt) - n * sqrt(discriminant);
				return true;
			} else return false;
		}

		inline static float Schlick(float cosine, float ref_idx) {
			float r0 = (1 - ref_idx) / (1 + ref_idx);
			r0 = r0 * r0;
			return r0 + (1 - r0) * pow((1 - cosine), 5);
		}

		inline static bool RayTriangleIntersect(const Ray &ray, const Vector3 &v0, const Vector3 &v1, const Vector3 &v2, float &t, float &u, float &v) {
			Vector3 v0v1 = v1 - v0;
			Vector3 v0v2 = v2 - v0;
			Vector3 pvec = ray.dir.cross(v0v2);
			float det = v0v1.dot(pvec);

			// ray and triangle are parallel if det is close to 0
			if (fabs(det) < EPSILON) return false;

			float invDet = 1.0 / det;
			Vector3 tvec = ray.pos - v0;
			u = tvec.dot(pvec) * invDet;
			if (u < 0 || u > 1) return false;

			Vector3 qvec = tvec.cross(v0v1);
			v = ray.dir.dot(qvec) * invDet;
			if (v < 0 || u + v > 1) return false;

			t = v0v2.dot(qvec) * invDet;

			//return (t > 0) ? true : false;
			return true;
		}

		// A test to see if P1 is on the same side as P2 of a line segment ab
		inline static bool SameSide(Vector3 p1, Vector3 p2, Vector3 a, Vector3 b) {
			Vector3 cp1 = Vector3::Cross(b - a, p1 - a);
			Vector3 cp2 = Vector3::Cross(b - a, p2 - a);

			if (Vector3::Dot(cp1, cp2) >= 0)
				return true;
			else
				return false;
		}

		// Generate a cross produect normal for a triangle
		inline static Vector3 GenTriNormal(Vector3 t1, Vector3 t2, Vector3 t3) {
			Vector3 u = t2 - t1;
			Vector3 v = t3 - t1;

			Vector3 normal = Vector3::Cross(u, v);

			return normal;
		}

		// Check to see if a Vector3 Point is within a 3 Vector3 Triangle
		inline static bool IsInTriangle(Vector3 point, Vector3 tri1, Vector3 tri2, Vector3 tri3) {
			// Test to see if it is within an infinite prism that the triangle outlines.
			bool within_tri_prisim = SameSide(point, tri1, tri2, tri3) &&
				SameSide(point, tri2, tri1, tri3) &&
				SameSide(point, tri3, tri1, tri2);

			// If it isn't it will never be on the triangle
			if (!within_tri_prisim) return false;

			// Calulate Triangle's Normal
			Vector3 n = GenTriNormal(tri1, tri2, tri3);

			// Project the point onto this normal
			Vector3 proj = Vector3::Projection(point, n);

			// If the distance from the triangle to the point is 0£¬it lies on the triangle
			if (proj.magnitude() == 0) return true;
			else return false;
		}

		inline static Vector2 RotateAround(const Vector2 &targetPoint, const Vector2 &rotCenter, const float theta) {
			// discard y val
			float cx = rotCenter.x, cy = rotCenter.y; // rotCenter
			float px = targetPoint.x, py = targetPoint.y; // point

			float s = sin(theta);
			float c = cos(theta);

			// translate point back to rotCenter:
			px -= cx;
			py -= cy;

			// rotate point
			float xnew = px * c + py * s;
			float ynew = -px * s + py * c;

			// translate point back:
			px = xnew + cx;
			py = ynew + cy;

			return Vector2(px, py);
		}

		// ---------------------------------------------------------------------------------
		inline static float Determinant(const Vector3 &a, const Vector3 &v1, const Vector3 &v2) {
			return a[0] * (v1[1] * v2[2] - v1[2] * v2[1]) + a[1] * (v1[2] * v2[0] - v1[0] * v2[2]) + a[2] * (v1[0] * v2[1] - v1[1] * v2[0]);
		}
		inline static float closestDistanceBetweenLines(const Ray &a0, const Ray &a1, Vector3 &p0, Vector3 &p1, const float &len0 = 1.0f, const float &len1 = 1.0f) {
			return closestDistanceBetweenLines(a0.pos, a0.PointAtParameter(len0), a1.pos, a1.PointAtParameter(len1), p0, p1);
		}
		// Given two lines defined by (a0,a1,b0,b1)
		// Return the closest points on each segment and their distance
		inline static float closestDistanceBetweenLines(const Vector3 &a0, const Vector3 &a1, const Vector3 &b0, const Vector3 &b1, Vector3 &Line1Closest, Vector3 &Line2Closest) {
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

	};
}