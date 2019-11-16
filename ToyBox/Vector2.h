#pragma once

#include <cmath>
#include <iostream>

namespace moon {
	extern class Vector3;
	class Vector2 {
	public:
		float x, y;
		Vector2() :x(0), y(0) {}
		Vector2(float x1, float y1) :x(x1), y(y1) {}
		Vector2(const Vector2 &v) :x(v.x), y(v.y) {}
		Vector2(const Vector3 &v);
		~Vector2() {}

		Vector2& operator+=(const Vector2 &v);
		Vector2& operator-=(const Vector2 &v);
		Vector2& operator*=(const Vector2 &v);
		Vector2& operator/=(const Vector2 &v);
		Vector2& operator*=(const float &t);
		Vector2& operator/=(const float &t);

		friend std::istream& operator>>(std::istream &is, Vector2 &t);
		friend std::ostream& operator<<(std::ostream &os, const Vector2 &t);
		friend bool operator==(const Vector2 &v1, const Vector2 &v2);
		friend Vector2 operator+(const Vector2 &v1, const Vector2 &v2);
		friend Vector2 operator-(const Vector2 &v1, const Vector2 &v2);
		friend Vector2 operator*(const Vector2 &v1, const Vector2 &v2);
		friend Vector2 operator/(const Vector2 &v1, const Vector2 &v2);
		friend Vector2 operator*(const float &t, const Vector2 &v);
		friend Vector2 operator/(const float &t, const Vector2 &v);
		friend Vector2 operator*(const Vector2 &v, const float &t);
		friend Vector2 operator/(const Vector2 &v, const float &t);

		inline void operator=(const Vector2 &v) { x = v.x; y = v.y; }
		inline const Vector2 operator+() const { return *this; }
		inline Vector2 operator-() const { return Vector2(-x, -y); }
		inline float operator[](int i) const { return i == 0 ? x : y; }
		inline float& operator[](int i) { return i == 0 ? x : y; }

		inline float dot(const Vector2 &v) const { return x * v.x + y * v.y; }
		inline float magnitude() const { return sqrtf(dot(*this)); }
		inline float MagSquared() const { return dot(*this); }


		// 另两个输入向量z轴为0，返回三维交叉积向量的z值（结果向量沿Z轴向上）
		inline float cross(const Vector2 &v) const {
			return (x * v.y - y * v.x);
		}

		inline void normalize() {
			float len = 1 / magnitude();
			x *= len; y *= len;
		}

		inline float distance(const Vector2 &v) const {
			float dist;
			dist = sqrt((this->x - v.x) * (this->x - v.x) + (this->y - v.y) * (this->y - v.y));
			return dist;
		}

		inline void setValue(float _x, float _y) { x = _x; y = _y; }

		static Vector2 Normalize(const Vector2 &v);
		static float Cross(const Vector2 &v1, const Vector2 &v2);
		static float Dot(const Vector2 &v1, const Vector2 &v2);
		static Vector2 ONE();
		static Vector2 ZERO();
	};
}