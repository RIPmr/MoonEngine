#pragma once
#include <cmath>
#include <ctime>
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
	#define PI acos(-1.0)
	#define Rad2Deg 180/PI
	#define Deg2Rad PI/180
	#define EPSILON 0.001
	#define INFINITY std::numeric_limits<float>::max()
	#define INFINITY_INT std::numeric_limits<int>::max()
	/// actually it's pow(255, 4) = 0xfc05fc01
	/// but range of int is ±2147483647, smaller than 0xfc05fc01
	#define MOON_IDBOUNDARY std::numeric_limits<int>::max()

	class MoonMath {
	private:
		static unsigned long long seed;
	public:

		inline static void CalculatePathVector(
			const std::vector<Vector3> &path,
			std::vector<Vector3> &normalList, 
			std::vector<Vector3> &tangentList,
			std::vector<Vector3> &bitangentList) {
			Vector3 prev, next, tangent, normal, bitangent;
			for (int i = 0; i < path.size(); i++) {
				if (i == 0) prev = next = path[i + 1] - path[i];
				else if (i == path.size() - 1) next = prev = path[i] - path[i - 1];
				else {
					prev = Vector3::Normalize(path[i] - path[i - 1]);
					next = Vector3::Normalize(path[i + 1] - path[i]);
				}
				tangent = (prev == -next ? next : Vector3::Normalize(prev + next));
				for (int ti = i + 2; ti < path.size() && tangent.magnitude() == 0; ti++) tangent = Vector3::Normalize(path[ti] - path[i]);
				normal = Vector3::Cross(tangent == Vector3::WORLD(UP) || tangent == Vector3::WORLD(DOWN) ? Vector3(0.0f, 1.0f, 0.001f) : Vector3::WORLD(UP), tangent);
				normal.normalize();
				bitangent = Vector3::Cross(tangent, normal);

				normalList.push_back(normal); tangentList.push_back(tangent); bitangentList.push_back(bitangent);
			}
		}

		// least-squre method fitting bezier curve
		inline static bool LeastSquareFitting(const std::vector<Vector3>& data,
			Vector3& inTangent, Vector3& outTangent) {
			if (data.size() < 2) return false;
			Vector3 P0(data[0]), P3(data[data.size() - 1]);

			// calculate P1 and P2
			float distSum = 0;
			for (int i = 0; i < data.size() - 1; i++) {
				float dist = Vector3::Distance(data[i], data[i + 1]);
				distSum += dist;
			}
			std::vector<float> ti; ti.push_back(0);
			for (int i = 1; i < data.size(); i++) {
				float dist = Vector3::Distance(data[i], data[i - 1]);
				ti.push_back(ti[ti.size() - 1] + dist / distSum);
			}

			float A1 = 0; float A2 = 0; float A12 = 0; int cnt = 0;
			Vector3 C1, C2;
			for(auto& t : ti) {
				A1 += t * t * std::pow((1 - t), 4);
				A2 += std::pow(t, 4) * std::pow((1 - t), 2);
				A12 += std::pow(t, 3) * std::pow((1 - t), 3);

				C1 += 3 * t * std::pow((1 - t), 2) * (data[cnt] - std::pow((1 - t), 3) * P0 - t * t * t * P3);
				C2 += 3 * t * t * (1 - t) * (data[cnt] - std::pow((1 - t), 3) * P0 - t * t * t * P3);

				cnt++;
			}
			A1 *= 9; A2 *= 9; A12 *= 9;

			Vector3 P1 = (A2 * C1 - A12 * C2) / (A1 * A2 - A12 * A12);
			Vector3 P2 = (A1 * C2 - A12 * C1) / (A1 * A2 - A12 * A12);

			inTangent = P1; outTangent = P2;
			return true;
		}

		/*
		函数模板与同名的非模板函数重载时候，调用顺序:
			1.寻找一个参数完全匹配的函数，如果找到了就调用它
			2.寻找一个函数模板，将其实例化，产生一个匹配的模板函数，若找到了，就调用它
			3.若1,2都失败，再试一试低一级的对函数的重载方法，例如通过类型转换可产生参数匹配，就调用它
			4.若1,2,3均未找到匹配的函数，则是一个错误的调用
		*/
		template<typename T>
		inline static bool Approximate(const T &a, const T &b, const float &epsilon = EPSILON) {
			if (std::abs(a - b) < epsilon) return true;
			else return false;
		}
		inline static bool Approximate(const Vector3 &a, const Vector3 &b, const float &epsilon = EPSILON) {
			return a.distance(b) < epsilon;
		}
		inline static bool Approximate(const Vector2 &a, const Vector2 &b, const float &epsilon = EPSILON) {
			return a.distance(b) < epsilon;
		}

		inline static double ExpTylor(double x, double precision = 1e-6) {
			double i = 1, num = 1;
			double sum = 1, temp = 1;
			do {
				num = num * i;
				temp = pow(x, i++) / num;
				sum += temp;
			} while (fabs(temp) > precision);
			return sum;
		}

		inline static std::vector<int> TENtoNBase(int num, int n) {
			std::vector<int> v;
			while (num) {
				v.push_back(num % n);
				num /= n;
			}
			return v;
		}

		inline static void GammaCorrection(Vector3 &GammaSpaceCol, const float& gamma = 2.2f) {
			float invGamma = 1.0f / gamma;
			GammaSpaceCol.x = std::pow(GammaSpaceCol.x, invGamma);
			GammaSpaceCol.y = std::pow(GammaSpaceCol.y, invGamma);
			GammaSpaceCol.z = std::pow(GammaSpaceCol.z, invGamma);
		}

		inline static double Random01() {
			return rand() / double(RAND_MAX);
		}

		inline static double RandomRange(const double &min, const double &max) {
			return lerp(min, max, Random01());
		}

		inline static int RandomRange(const int &min, const int &max) {
			//srand((unsigned)time(NULL));
			return (rand() % (max - min + 1)) + min;
		}

		/*
		Gaussian/normal distribution
		E = 0
		V = 1
		*/
		inline static double GaussianRand() {
			static double V1, V2, S;
			static int phase = 0;
			double X;

			if (phase == 0) {
				do {
					double U1 = (double)rand() / RAND_MAX;
					double U2 = (double)rand() / RAND_MAX;

					V1 = 2 * U1 - 1;
					V2 = 2 * U2 - 1;
					S = V1 * V1 + V2 * V2;
				} while (S >= 1 || S == 0);

				X = V1 * sqrt(-2 * log(S) / S);
			} else X = V2 * sqrt(-2 * log(S) / S);

			phase = 1 - phase;
			return X;
		}

		inline static double GaussianRand(const double &E,const double &V) {
			double X = GaussianRand();
			X = X * V + E;
			return X;
		}

		// 返回服从均匀分布的 [0.0, 1.0) 之间的 double 型随机数
		inline static double drand48() {
			seed = (MOON_a * seed + MOON_c) & 0xFFFFFFFFFFFFLL;
			unsigned int x = seed >> 16;
			return ((double)x / (double)MOON_m);
		}

		inline static void srand48(unsigned int i) {
			seed = (((long long int)i) << 16) | rand();
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

		template <typename T>
		inline static int GetSign(const T& number) {
			return number / std::fabs(number);
		}

		// left + (right - left) * percent
		// percent: [0,1]
		inline static double lerp(const double &left, const double &right, const double &percent) {
			return left + (right - left) * percent;
		}

		inline static float clamp(const float &num, const float &min, const float &max) {
			return std::max(min, std::min(max, num));
		}

		inline static Vector3 clamp(const Vector3 &num, const float &min, const float &max) {
			return Vector3(
				std::max(min, std::min(max, num.x)),
				std::max(min, std::min(max, num.y)),
				std::max(min, std::min(max, num.z))
			);
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
		inline static bool SameSide(const Vector3 &p1, const Vector3 &p2, const Vector3 &a, const Vector3 &b) {
			Vector3 cp1 = Vector3::Cross(b - a, p1 - a);
			Vector3 cp2 = Vector3::Cross(b - a, p2 - a);

			if (Vector3::Dot(cp1, cp2) >= 0)
				return true;
			else
				return false;
		}

		// Generate a cross produect normal for a triangle
		inline static Vector3 GenTriNormal(const Vector3 &t1, const Vector3 &t2, const Vector3 &t3) {
			Vector3 u = t2 - t1;
			Vector3 v = t3 - t1;

			Vector3 normal = Vector3::Cross(u, v);

			return normal;
		}

		// Check to see if a Vector3 Point is within a 3 Vector3 Triangle
		inline static bool IsInTriangle(const Vector3 &point, const Vector3 &tri1, const Vector3 &tri2, const Vector3 &tri3) {
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

			// If the distance from the triangle to the point is 0，it lies on the triangle
			if (proj.magnitude() == 0) return true;
			else return false;
		}

		// Rotate 2D point around origin point (0, 0)
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

		// Rotate 3D point around given axis
		// *NOTE: not tested yet!
		static Vector3 RotateAround(const Vector3 &position, const Vector3 &center, const Vector3 &axis, const float &angle);

		/// <summary>
		/// 计算射线与平面的空间状态判断
		/// 平面法线方向必须是射线起点所在的平面一侧方向
		/// 返回等于0时,射线与平面平行,无交点
		/// 返回大于0时,射线射向与平面相反的方向,无交点
		/// </summary>
		inline static float IsRayPlaneIntersect(const Ray &ray, Vector3 planeNormal, const Vector3 &planePoint, const bool cullBack = true) {
			if (!cullBack) planeNormal *= Vector3::Dot(planePoint - ray.pos, planeNormal) > 0 ? -1 : 1;
			return Vector3::Dot(ray.dir, planeNormal);
		}

		inline static Vector3 RayPlaneIntersect(const Ray &ray, const Vector3 &planeNormal, const Vector3 &planePoint) {
			float d = Vector3::Dot(planePoint - ray.pos, planeNormal) / Vector3::Dot(ray.dir, planeNormal);
			return ray.PointAtParameter(d);
		}

		/// <summary>
		/// 计算空间线段与平面的交点
		/// accuracy（容差）越小对于直线端点刚好为交点的情况求交越精确
		/// </summary>
		inline static bool LinePlaneIntersect(const Vector3 &pointA, const Vector3 &pointB, const Vector3 &planeNormal, const Vector3 &planePoint, Vector3 &intersect, const float &accuracy = 0.0001f) {
			// first, treat the line as an ray
			Vector3 direction = pointB - pointA;
			int determine = Vector3::Dot(planePoint - pointA, planeNormal) > 0 ? -1 : 1;

			// check for intersect
			if (IsRayPlaneIntersect(Ray(pointA, direction), planeNormal * determine, planePoint) < 0) {
				// calculate intersect point
				intersect = RayPlaneIntersect(Ray(pointA, Vector3::Normalize(direction)), planeNormal * determine, planePoint);
				// check if the intersect point is inside line
				if ((intersect - pointA).magnitude() <= (direction.magnitude() + accuracy)) return true;
			}

			return false;
		}

		inline static bool RayDiskIntersect(const Ray& ray, const Vector3 &diskPoint, const Vector3 &diskNormal, const float &diskRadius, Vector3 &intersect, const float &accuracy = 0.0001f) {
			int determine = Vector3::Dot(diskPoint - ray.pos, diskNormal) > 0 ? -1 : 1;

			if (IsRayPlaneIntersect(Ray(ray.pos, ray.dir), diskNormal * determine, diskPoint) < 0) {
				intersect = RayPlaneIntersect(Ray(ray.pos, Vector3::Normalize(ray.dir)), diskNormal * determine, diskPoint);
				if ((intersect - diskPoint).magnitude() <= diskRadius + accuracy) return true;
			}

			return false;
		}

		inline static float PointLineDistance(const Vector3 &point, const Vector3 &linePoint1, const Vector3 &linePoint2) {
			float fProj = Vector3::Dot(point - linePoint1, Vector3::Normalize(linePoint1 - linePoint2));
			return std::sqrtf((point - linePoint1).magnitude() - fProj * fProj);
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
		static float closestDistanceBetweenLines(const Vector3 &a0, const Vector3 &a1, const Vector3 &b0, const Vector3 &b1, Vector3 &Line1Closest, Vector3 &Line2Closest);

	};
}