#include "Vector3.h"

Vector3::Vector3(const Vector2 &v) {
	x = v.x; y = v.y; z = 0;
}

std::istream& operator>>(std::istream &is, Vector3 &t) {
	is >> t.x >> t.y >> t.z;
	return is;
}

std::ostream& operator<<(std::ostream &os, const Vector3 &t) {
	os << "(" << t.x << ", " << t.y << ", " << t.z << ")";
	return os;
}

Vector3 operator+(const Vector3 &v1, const Vector3 &v2) {
	return Vector3(v1.x + v2.x, v1.y + v2.y, v1.z + v2.z);
}

Vector3 operator-(const Vector3 &v1, const Vector3 &v2) {
	return Vector3(v1.x - v2.x, v1.y - v2.y, v1.z - v2.z);
}

Vector3 operator*(const Vector3 &v1, const Vector3 &v2) {
	return Vector3(v1.x * v2.x, v1.y * v2.y, v1.z * v2.z);
}

Vector3 operator/(const Vector3 &v1, const Vector3 &v2) {
	return Vector3(v1.x / v2.x, v1.y / v2.y, v1.z / v2.z);
}

Vector3 operator*(const float &t, const Vector3 &v) {
	return Vector3(t * v.x, t * v.y, t * v.z);
}

Vector3 operator/(const float &t, const Vector3 &v) {
	return Vector3(v.x / t, v.y / t, v.z / t);
}

Vector3 operator*(const Vector3 &v, const float &t) {
	return Vector3(t * v.x, t * v.y, t * v.z);
}

Vector3 operator/(const Vector3 &v, const float &t) {
	return Vector3(v.x / t, v.y / t, v.z / t);
}

Vector3& Vector3::operator+=(const Vector3 &v) {
	x += v.x; y += v.y; z += v.z;
	return *this;
}

Vector3& Vector3::operator-=(const Vector3 &v) {
	x -= v.x; y -= v.y; z -= v.z;
	return *this;
}

Vector3& Vector3::operator*=(const Vector3 &v) {
	x *= v.x; y *= v.y; z *= v.z;
	return *this;
}

Vector3& Vector3::operator/=(const Vector3 &v) {
	x /= v.x; y /= v.y; z /= v.z;
	return *this;
}

Vector3& Vector3::operator*=(const float &t) {
	x *= t; y *= t; z *= t;
	return *this;
}

Vector3& Vector3::operator/=(const float &t) {
	x /= t; y /= t; z /= t;
	return *this;
}

Vector3 Vector3::Normalize(const Vector3 &v) {
	float len = 1 / v.length();
	len = 1 / len;

	return Vector3(v.x, v.y, v.z) * len;
}

float Vector3::Dot(const Vector3 &v1, const Vector3 &v2) {
	return v1.x * v2.x + v1.y * v2.y + v1.z * v2.z;
}

Vector3 Vector3::Cross(const Vector3 &v1, const Vector3 &v2) {
	return Vector3(v1.y * v2.z - v1.z * v2.y,
		v1.z * v2.x - v1.x * v2.z,
		v1.x * v2.y - v1.y * v2.x);
}

Vector3 Vector3::ONE() {
	return Vector3(1.0, 1.0, 1.0);
}

Vector3 Vector3::ZERO() {
	return Vector3(0.0, 0.0, 0.0);
}