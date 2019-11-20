#include "Quaternion.h"
#include "MathUtils.h"
#include "Matrix4x4.h"

namespace moon {
	// Not tested yet
	Quaternion::Quaternion(const Matrix4x4 &mat) {
		float m11 = mat.x[1][1], m12 = mat.x[2][1], m13 = mat.x[3][1];
		float m21 = mat.x[1][2], m22 = mat.x[2][2], m23 = mat.x[3][2];
		float m31 = mat.x[1][3], m32 = mat.x[2][3], m33 = mat.x[3][3];


		//探测四元数中最大的项 
		float fourWSquaredMinusl = m11 + m22 + m33;
		float fourXSquaredMinusl = m11 - m22 - m33;
		float fourYSquaredMinusl = m22 - m11 - m33;
		float fourZSquaredMinusl = m33 - m11 - m22;

		int biggestIndex = 0;
		float fourBiggestSqureMinus1 = fourWSquaredMinusl;
		if (fourXSquaredMinusl > fourBiggestSqureMinus1) {
			fourBiggestSqureMinus1 = fourXSquaredMinusl;
			biggestIndex = 1;
		}
		if (fourYSquaredMinusl > fourBiggestSqureMinus1) {
			fourBiggestSqureMinus1 = fourYSquaredMinusl;
			biggestIndex = 2;
		}
		if (fourZSquaredMinusl > fourBiggestSqureMinus1) {
			fourBiggestSqureMinus1 = fourZSquaredMinusl;
			biggestIndex = 3;
		}

		//计算平方根和除法 
		float biggestVal = sqrt(fourBiggestSqureMinus1 + 1.0f)*0.5f;
		float mult = 0.25f / biggestVal;

		//计算四元数的值
		switch (biggestIndex) {
		case 0:
			w = biggestVal;
			x = (m23 - m32)*mult;
			y = (m31 - m13)*mult;
			z = (m12 - m21)*mult;
			break;
		case 1:
			x = biggestVal;
			w = (m23 - m32)*mult;
			y = (m12 + m21)*mult;
			z = (m31 + m13)*mult;
			break;
		case 2:
			y = biggestVal;
			w = (m31 - m13)*mult;
			x = (m12 + m21)*mult;
			z = (m23 + m32)*mult;
			break;
		case 3:
			z = biggestVal;
			w = (m12 - m21)*mult;
			x = (m31 + m13)*mult;
			y = (m23 + m32)*mult;
			break;
		}
	}

	float Quaternion::MagSquared() const { return (x * x + y * y + z * z + w * w); }

	float Quaternion::magnitude() const { return sqrtf(x * x + y * y + z * z + w * w); }

	void Quaternion::normalize() {
		float mag = magnitude();
		x /= mag; y /= mag;
		z /= mag; w /= mag;
	}

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


	void Quaternion::SetValue(float _x, float _y, float _z, float _w) {
		x = _x; y = _y; z = _z; w = _w;
		UpdateEulerAngle();
	}

	void Quaternion::SetValue(const Quaternion &q) {
		x = q.x; y = q.y; z = q.z; w = q.w;
		UpdateEulerAngle();
	}

	void Quaternion::SetEulerAngle(float yaw, float pitch, float roll) {
		eulerAngles.setValue(yaw, pitch, roll);
		yaw *= Deg2Rad; pitch *= Deg2Rad; roll *= Deg2Rad;

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
		// calculate θ/2
		float alpha = acos(w);
		float newAlpha = alpha * pow;
		w = cos(newAlpha);
		float multi = sin(newAlpha) / sin(alpha);

		x *= multi;
		y *= multi;
		z *= multi;

		UpdateEulerAngle();
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

	Quaternion Quaternion::Normalize(Quaternion &q) {
		return Quaternion(q.x, q.y, q.z, q.w);
	}

	Quaternion& Quaternion::operator+=(const Quaternion &q) {
		x += q.x; y += q.y; z += q.z; w += q.w;

		UpdateEulerAngle();
		return *this;
	}

	Quaternion& Quaternion::operator-=(const Quaternion &q) {
		x -= q.x; y -= q.y; z -= q.z; w -= q.w;

		UpdateEulerAngle();
		return *this;
	}

	Quaternion& Quaternion::operator*=(const Quaternion &q) {
		x = w * q.x + x * q.w + y * q.z - z * q.y;
		y = w * q.y - x * q.z + y * q.w + z * q.x;
		z = w * q.z + x * q.y - y * q.x + z * q.w;
		w = w * q.w - x * q.x - y * q.y - z * q.z;

		UpdateEulerAngle();
		return *this;
	}

	Quaternion& Quaternion::operator*=(float s) {
		x *= s; y *= s; z *= s; w *= s;

		UpdateEulerAngle();
		return *this;
	}

	Quaternion& Quaternion::operator/=(float s) {
		x /= s; y /= s; z /= s; w /= s;

		UpdateEulerAngle();
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
		return eulerAngles;
	}

	void Quaternion::UpdateEulerAngle() {
		float yaw = atan2(2 * (w * x + z * y), 1 - 2 * (x * x + y * y));
		float pitch = asin(MoonMath::clamp(2 * (w * y - x * z), -1.0f, 1.0f));
		float roll = atan2(2 * (w * z + x * y), 1 - 2 * (z * z + y * y));
		eulerAngles.setValue(Rad2Deg * yaw, Rad2Deg * pitch, Rad2Deg * roll);
	}
}