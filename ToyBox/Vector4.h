#pragma once
#include <cmath>
#include <iostream>

namespace moon {
	extern class Vector3;
	class Vector4 {
	public:
		float x, y, z, w;
		Vector4() :x(0), y(0), z(0), w(0) {}
		Vector4(float x1, float y1, float z1, float w1) :x(x1), y(y1), z(z1), w(w1) {}
		Vector4(const Vector4 &v) :x(v.x), y(v.y), z(v.z), w(v.w) {}
		Vector4(const Vector3 &v);
		~Vector4() {}

		Vector4& operator+=(const Vector4 &v);
		Vector4& operator-=(const Vector4 &v);
		Vector4& operator*=(const Vector4 &v);
		Vector4& operator/=(const Vector4 &v);
		Vector4& operator*=(const float &t);
		Vector4& operator/=(const float &t);

		friend std::istream& operator>>(std::istream &is, Vector4 &t);
		friend std::ostream& operator<<(std::ostream &os, const Vector4 &t);
		friend bool operator==(const Vector4 &v1, const Vector4 &v2);
		friend bool operator!=(const Vector4 &v1, const Vector4 &v2);
		friend Vector4 operator+(const Vector4 &v1, const Vector4 &v2);
		friend Vector4 operator-(const Vector4 &v1, const Vector4 &v2);
		friend Vector4 operator*(const Vector4 &v1, const Vector4 &v2);
		friend Vector4 operator/(const Vector4 &v1, const Vector4 &v2);
		friend Vector4 operator*(const float &t, const Vector4 &v);
		friend Vector4 operator/(const float &t, const Vector4 &v);
		friend Vector4 operator*(const Vector4 &v, const float &t);
		friend Vector4 operator/(const Vector4 &v, const float &t);

		inline void operator=(const Vector4 &v) { x = v.x; y = v.y; z = v.z; w = v.w; }
		inline const Vector4 operator+() const { return *this; }
		inline Vector4 operator-() const { return Vector4(-x, -y, -z, -w); }
		inline float operator[](int i) const { return i == 0 ? x : (i == 1 ? y : (i == 2 ? z : w)); }
		inline float& operator[](int i) { return i == 0 ? x : (i == 1 ? y : (i == 2 ? z : w)); }

		inline void setValue(float _x, float _y, float _z, float _w) { x = _x; y = _y; z = _z; w = _w; }

		static Vector4 ONE();
		static Vector4 ZERO();
	};
}