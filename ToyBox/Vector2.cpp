#include "Vector2.h"
#include "Vector3.h"

namespace MOON {
	Vector2::Vector2(const Vector3 &v) {
		x = v.x; y = v.y;
	}

	bool operator==(const Vector2 &v1, const Vector2 &v2) {
		if (v1.x == v2.x && v1.y == v2.y) return true;
		else return false;
	}

	bool operator!=(const Vector2 &v1, const Vector2 &v2) {
		if (v1.x == v2.x && v1.y == v2.y) return false;
		else return true;
	}

	std::istream& operator>>(std::istream &is, Vector2 &t) {
		is >> t.x >> t.y;
		return is;
	}

	std::ostream& operator<<(std::ostream &os, const Vector2 &t) {
		os << "(" << t.x << ", " << t.y << ")";
		return os;
	}

	Vector2 operator+(const Vector2 &v1, const Vector2 &v2) {
		return Vector2(v1.x + v2.x, v1.y + v2.y);
	}

	Vector2 operator-(const Vector2 &v1, const Vector2 &v2) {
		return Vector2(v1.x - v2.x, v1.y - v2.y);
	}

	Vector2 operator*(const Vector2 &v1, const Vector2 &v2) {
		return Vector2(v1.x * v2.x, v1.y * v2.y);
	}

	Vector2 operator/(const Vector2 &v1, const Vector2 &v2) {
		return Vector2(v1.x / v2.x, v1.y / v2.y);
	}

	Vector2 operator*(const float &t, const Vector2 &v) {
		return Vector2(t * v.x, t * v.y);
	}

	Vector2 operator/(const float &t, const Vector2 &v) {
		return Vector2(v.x / t, v.y / t);
	}

	Vector2 operator*(const Vector2 &v, const float &t) {
		return Vector2(t * v.x, t * v.y);
	}

	Vector2 operator/(const Vector2 &v, const float &t) {
		return Vector2(v.x / t, v.y / t);
	}

	Vector2& Vector2::operator+=(const Vector2 &v) {
		x += v.x; y += v.y;
		return *this;
	}

	Vector2& Vector2::operator-=(const Vector2 &v) {
		x -= v.x; y -= v.y;
		return *this;
	}

	Vector2& Vector2::operator*=(const Vector2 &v) {
		x *= v.x; y *= v.y;
		return *this;
	}

	Vector2& Vector2::operator/=(const Vector2 &v) {
		x /= v.x; y /= v.y;
		return *this;
	}

	Vector2& Vector2::operator*=(const float &t) {
		x *= t; y *= t;
		return *this;
	}

	Vector2& Vector2::operator/=(const float &t) {
		x /= t; y /= t;
		return *this;
	}

	Vector2 Vector2::Normalize(const Vector2 &v) {
		float len = 1 / v.magnitude();

		return Vector2(v.x, v.y) * len;
	}

	float Vector2::Dot(const Vector2 &v1, const Vector2 &v2) {
		return v1.x * v2.x + v1.y * v2.y;
	}

	float Vector2::Cross(const Vector2 &v1, const Vector2 &v2) {
		return (v1.x * v2.y - v1.y * v2.x);
	}

	Vector2 Vector2::ONE() {
		return Vector2(1.0, 1.0);
	}

	Vector2 Vector2::ZERO() {
		return Vector2(0.0, 0.0);
	}
}