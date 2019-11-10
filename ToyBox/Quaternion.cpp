#include "Quaternion.h"
#include "MathUtils.h"

namespace moon {
	//Cos theta of two quaternion
	float Quaternion::Dot(const Quaternion &lhs, const Quaternion &rhs) {
		return lhs.x * rhs.x + lhs.y * rhs.y + lhs.z * rhs.z + lhs.w * rhs.w;
	}

	Quaternion Quaternion::Lerp(const Quaternion &a, const Quaternion &b, float t) {
		return (1 - t) * a + t * b;
	}

	Quaternion Quaternion::Slerp(const Quaternion &a, const Quaternion &b, float t) {
		float theta = Angle(a, b);
		float sintheta = sin(theta);
		float ta = sin((1 - t) * theta) / sintheta;
		float tb = sin(t * theta) / sintheta;
		return ta * a + tb * b;
	}

	float Quaternion::Angle(const Quaternion &lhs, const Quaternion &rhs) {
		float cos_theta = Dot(lhs, rhs);
		// if B is on opposite hemisphere from A, use -B instead
		if (cos_theta < 0.f) cos_theta = -cos_theta;
		float theta = acos(cos_theta);
		return 2 * Rad2Deg * theta;
	}


	void Quaternion::Set(float _x, float _y, float _z, float _w) {
		x = _x; y = _y; z = _z; w = _w;
	}

	void Quaternion::SetEulerAngle(float yaw, float pitch, float roll) {
		float angle;
		float sinRoll, sinPitch, sinYaw, cosRoll, cosPitch, cosYaw;

		angle = yaw * 0.5f;
		sinYaw = sin(angle);
		cosYaw = cos(angle);

		angle = pitch * 0.5f;
		sinPitch = sin(angle);
		cosPitch = cos(angle);

		angle = roll * 0.5f;
		sinRoll = sin(angle);
		cosRoll = cos(angle);

		float _y = cosRoll * sinPitch*cosYaw + sinRoll * cosPitch*sinYaw;
		float _x = cosRoll * cosPitch*sinYaw - sinRoll * sinPitch*cosYaw;
		float _z = sinRoll * cosPitch*cosYaw - cosRoll * sinPitch*sinYaw;
		float _w = cosRoll * cosPitch*cosYaw + sinRoll * sinPitch*sinYaw;

		float mag = _x * _x + _y * _y + _z * _z + _w * _w;
		x = _x / mag;
		y = _y / mag;
		z = _z / mag;
		w = _w / mag;
	}

	Quaternion& Quaternion::operator^=(const float &pow) {
		// calculate ¦È/2
		float alpha = acos(w);
		float newAlpha = alpha * pow;
		w = cos(newAlpha);
		float multi = sin(newAlpha) / sin(alpha);

		x *= multi;
		y *= multi;
		z *= multi;

		return *this;
	}

	Quaternion operator^(const Quaternion& q, const float& pow) {
		float alpha = acos(q.w);
		float newAlpha = alpha * pow;
		float ww = cos(newAlpha);
		float multi = sin(newAlpha) / sin(alpha);

		return Quaternion(q.x * multi, q.y * multi, q.z * multi, ww);
	}

	Quaternion Quaternion::Pow(const Quaternion& q, const float& t) {
		float alpha = acos(q.w);
		float newAlpha = alpha * t;
		float ww = cos(newAlpha);
		float multi = sin(newAlpha) / sin(alpha);

		return Quaternion(q.x * multi, q.y * multi, q.z * multi, ww);
	}

	Quaternion& Quaternion::operator+=(const Quaternion &q) {
		x += q.x; y += q.y; z += q.z; w += q.w;
		return *this;
	}

	Quaternion& Quaternion::operator-=(const Quaternion &q) {
		x -= q.x; y -= q.y; z -= q.z; w -= q.w;
		return *this;
	}

	Quaternion& Quaternion::operator*=(float s) {
		x *= s; y *= s; z *= s; w *= s;
		return *this;
	}

	Quaternion& Quaternion::operator/=(float s) {
		x /= s; y /= s; z /= s; w /= s;
		return *this;
	}

	Quaternion Quaternion::Conjugate() const {
		return Quaternion(-x, -y, -z, w);
	}

	Quaternion Quaternion::Inverse() const {
		return Quaternion(-x, -y, -z, w);
	}

	Quaternion operator+(const Quaternion& lhs, const Quaternion& rhs) {
		return Quaternion(lhs.x + rhs.x, lhs.y + rhs.y, lhs.z + rhs.z, lhs.w + rhs.w);
	}

	Quaternion operator-(const Quaternion& lhs, const Quaternion& rhs) {
		return Quaternion(lhs.x - rhs.x, lhs.y - rhs.y, lhs.z - rhs.z, lhs.w - rhs.w);
	}

	Quaternion operator*(const Quaternion& lhs, const Quaternion& rhs) {
		float w1 = lhs.w;
		float w2 = rhs.w;
		Vector3 v1(lhs.x, lhs.y, lhs.z);
		Vector3 v2(rhs.x, rhs.y, rhs.z);
		float w3 = w1 * w2 - v1.dot(v2);
		Vector3 v3 = v1.cross(v2) + v2 * w1 + v1 * w2;
		return Quaternion(v3.x, v3.y, v3.z, w3);
	}

	Quaternion operator*(const Quaternion& r, const float& s) {
		return Quaternion(r.x * s, r.y * s, r.z * s, r.w * s);
	}

	Quaternion operator/(const Quaternion& r, const float& s) {
		return Quaternion(r.x / s, r.y / s, r.z / s, r.w / s);
	}

	Quaternion operator*(const float& s, const Quaternion& r) {
		return r * s;
	}

	Quaternion operator/(const float& s, const Quaternion& r) {
		return r / s;
	}

	Vector3 operator*(const Quaternion& q, const Vector3& v) {
		// Extract the vector part of the quaternion
		Vector3 u(q.x, q.y, q.z);
		// Extract the scalar part of the quaternion
		float s = q.w;
		return u * 2.0f * u.dot(v) + v * (s * s - u.dot(u)) + u.cross(v) * 2.0f * s;
	}

	Vector3 Quaternion::EulerAngle() const {
		float yaw = atan2(2 * (w * x + z * y), 1 - 2 * (x * x + y * y));
		float pitch = asin(MoonMath::clamp(2 * (w * y - x * z), -1.0f, 1.0f));
		float roll = atan2(2 * (w * z + x * y), 1 - 2 * (z * z + y * y));
		return Vector3(Rad2Deg * yaw, Rad2Deg * pitch, Rad2Deg * roll);
	}
}