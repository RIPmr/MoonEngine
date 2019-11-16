#include "Vector4.h"
#include "Vector3.h"

namespace moon {
	Vector4::Vector4(const Vector3 &v) : x(v.x), y(v.y), z(v.z), w(1) {}

	std::istream& operator>>(std::istream &is, Vector4 &t) {
		is >> t.x >> t.y >> t.z >> t.w;
		return is;
	}

	std::ostream& operator<<(std::ostream &os, const Vector4 &t) {
		os << "(" << t.x << ", " << t.y << ", " << t.z << ", " << t.w << ")";
		return os;
	}

	Vector4& Vector4::operator+=(const Vector4 &v) {
		x += v.x; y += v.y; z += v.z; w += v.w;
		return *this;
	}
	Vector4& Vector4::operator-=(const Vector4 &v) {
		x -= v.x; y -= v.y; z -= v.z; w -= v.w;
		return *this;
	}
	Vector4& Vector4::operator*=(const Vector4 &v) {
		x *= v.x; y *= v.y; z *= v.z; w *= v.w;
		return *this;
	}
	Vector4& Vector4::operator/=(const Vector4 &v) {
		x /= v.x; y /= v.y; z /= v.z; w /= v.w;
		return *this;
	}
	Vector4& Vector4::operator*=(const float &t) {
		x *= t; y *= t; z *= t; w *= t;
		return *this;
	}
	Vector4& Vector4::operator/=(const float &t) {
		x /= t; y /= t; z /= t; w /= t;
		return *this;
	}

	Vector4 operator+(const Vector4 &v1, const Vector4 &v2) {
		return Vector4(v1.x + v2.x, v1.y + v2.y, v1.z + v2.z, v1.w + v2.w);
	}

	Vector4 operator-(const Vector4 &v1, const Vector4 &v2) {
		return Vector4(v1.x - v2.x, v1.y - v2.y, v1.z - v2.z, v1.w - v2.w);
	}

	Vector4 operator*(const Vector4 &v1, const Vector4 &v2) {
		return Vector4(v1.x * v2.x, v1.y * v2.y, v1.z * v2.z, v1.w * v2.w);
	}

	Vector4 operator/(const Vector4 &v1, const Vector4 &v2) {
		return Vector4(v1.x / v2.x, v1.y / v2.y, v1.z / v2.z, v1.w / v2.w);
	}

	Vector4 operator*(const float &t, const Vector4 &v) {
		return Vector4(t * v.x, t * v.y, t * v.z, t * v.w);
	}

	Vector4 operator/(const float &t, const Vector4 &v) {
		return Vector4(v.x / t, v.y / t, v.z / t, v.w / t);
	}

	Vector4 operator*(const Vector4 &v, const float &t) {
		return Vector4(t * v.x, t * v.y, t * v.z, t * v.w);
	}

	Vector4 operator/(const Vector4 &v, const float &t) {
		return Vector4(v.x / t, v.y / t, v.z / t, v.w / t);
	}

	bool operator!=(const Vector4 &v1, const Vector4 &v2) {
		return !(v1 == v2);
	}

	bool operator==(const Vector4 &v1, const Vector4 &v2) {
		if (v1.x == v2.x && v1.y == v2.y && v1.z == v2.z && v1.w == v2.w) return true;
		else return false;
	}

	Vector4 Vector4::ONE() {
		return Vector4(1.0, 1.0, 1.0, 1.0);
	}

	Vector4 Vector4::ZERO() {
		return Vector4(0.0, 0.0, 0.0, 0.0);
	}
}