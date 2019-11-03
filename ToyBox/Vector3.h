#pragma once

#include "Vector2.h"
extern class Vector2;

#include <cmath>
#include <iostream>

class Vector3 {
public:
	float x, y, z;
	Vector3() :x(0), y(0), z(0) {}
	Vector3(float x1, float y1, float z1) :x(x1), y(y1), z(z1) {}
	Vector3(const Vector3 &v) :x(v.x), y(v.y), z(v.z) {}
	Vector3(const Vector2 &v);
	~Vector3() {}

	Vector3& operator+=(const Vector3 &v);
	Vector3& operator-=(const Vector3 &v);
	Vector3& operator*=(const Vector3 &v);
	Vector3& operator/=(const Vector3 &v);
	Vector3& operator*=(const float &t);
	Vector3& operator/=(const float &t);

	friend std::istream& operator>>(std::istream &is, Vector3 &t);
	friend std::ostream& operator<<(std::ostream &os, const Vector3 &t);
	friend Vector3 operator+(const Vector3 &v1, const Vector3 &v2);
	friend Vector3 operator-(const Vector3 &v1, const Vector3 &v2);
	friend Vector3 operator*(const Vector3 &v1, const Vector3 &v2);
	friend Vector3 operator/(const Vector3 &v1, const Vector3 &v2);
	friend Vector3 operator*(const float &t, const Vector3 &v);
	friend Vector3 operator/(const float &t, const Vector3 &v);
	friend Vector3 operator*(const Vector3 &v, const float &t);
	friend Vector3 operator/(const Vector3 &v, const float &t);

	inline void operator=(const Vector3 &v) { x = v.x; y = v.y; z = v.z; }
	inline const Vector3 operator+() const { return *this; }
	inline Vector3 operator-() const { return Vector3(-x, -y, -z); }
	inline float operator[](int i) const { return i == 0 ? x : (i == 1 ? y : z); }
	inline float& operator[](int i) { return i == 0 ? x : (i == 1 ? y : z); }

	inline float dot(const Vector3 &v) const { return x * v.x + y * v.y + z * v.z; }
	inline float length() const { return sqrtf(dot(*this)); }
	inline float squaredLength() const { return dot(*this); }

	/*
	Cross Product
	aXb = | i,  j,  k  |
		  | a.x a.y a.z|
		  | b.x b.y b.z| =
		  (a.y*b.z - a.z*b.y)i +
		  (a.z*b.x - a.x*b.z)j +
		  (a.x*b.y - a.y*b.x)k
	*/
	inline Vector3 cross(const Vector3 &v) const {
		return Vector3(y * v.z - z * v.y,
					   z * v.x - x * v.z,
					   x * v.y - y * v.x);
	}

	inline void normalize() {
		float len = 1 / length();
		x *= len; y *= len; z *= len;
	}

	inline float distance(const Vector3 &v) const {
		float dist;
		dist = sqrt((this->x - v.x) * (this->x - v.x) + (this->y - v.y) * (this->y - v.y) + (this->z - v.z) * (this->z - v.z));
		return dist;
	}

	static Vector3 Normalize(const Vector3 &v);
	static Vector3 Cross(const Vector3 &v1, const Vector3 &v2);
	static float Dot(const Vector3 &v1, const Vector3 &v2);
	static Vector3 ONE();
	static Vector3 ZERO();
};