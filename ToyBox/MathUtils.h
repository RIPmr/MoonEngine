#pragma once

#include <cmath>
#include <cstdlib>
#include <algorithm>

#include "Vector3.h"
#include "Ray.h"

namespace moon {
	#define MOON_m 0x100000000LL
	#define MOON_c 0xB16
	#define MOON_a 0x5DEECE66DLL
	#define PI acos(-1.0)
	#define Rad2Deg 180/PI
	#define Deg2Rad PI/180
	#define INFINITY std::numeric_limits<float>::max()
	#define EPSILON 0.001

	class MoonMath {
	private:
		static unsigned long long seed;
	public:
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
			} while (p.squaredMagnitude() >= 1.0);
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

	};
}