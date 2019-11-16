#pragma once
#include <cmath>
#include <iostream>
#include <iomanip>

#include "Vector3.h"
#include "Quaternion.h"

namespace moon {
	class Matrix4x4 {
	public:
		float x[4][4] = { {1,0,0,0},
						  {0,1,0,0},
						  {0,0,1,0},
						  {0,0,0,1} };

		Matrix4x4() {}
		Matrix4x4(float i) {
			x[0][0] = i; x[0][1] = 0; x[0][2] = 0; x[0][3] = 0;
			x[1][0] = 0; x[1][1] = i; x[1][2] = 0; x[1][3] = 0;
			x[2][0] = 0; x[2][1] = 0; x[2][2] = i; x[2][3] = 0;
			x[3][0] = 0; x[3][1] = 0; x[3][2] = 0; x[3][3] = i;
		}
		Matrix4x4(float a, float b, float c, float d, float e, float f, float g, float h,
			float i, float j, float k, float l, float m, float n, float o, float p) {
			x[0][0] = a; x[0][1] = b; x[0][2] = c; x[0][3] = d;
			x[1][0] = e; x[1][1] = f; x[1][2] = g; x[1][3] = h;
			x[2][0] = i; x[2][1] = j; x[2][2] = k; x[2][3] = l;
			x[3][0] = m; x[3][1] = n; x[3][2] = o; x[3][3] = p;
		}

		const float* operator[](uint8_t i) const { return x[i]; }
		float* operator[](uint8_t i) { return x[i]; }

		// Multiply the current matrix with another matrix (rhs)
		Matrix4x4 operator*(const Matrix4x4& v) const {
			Matrix4x4 tmp;
			multiply(*this, v, tmp);

			return tmp;
		}

		// To make it easier to understand how a matrix multiplication works, 
		// the fragment of code included within the #if-#else statement, show 
		// how this works if you were to iterate over the coefficients of the 
		// resulting matrix (a). However you will often see this multiplication 
		// being done using the code contained within the #else-#end statement. 
		// It is exactly the same as the first fragment only we have litteraly 
		// written down as a series of operations what would actually result from 
		// executing the two for() loops contained in the first fragment. 
		// It is supposed to be faster, however considering matrix multiplicatin 
		// is not necessarily that common, this is probably not super useful nor 
		// really necessary (but nice to have -- and it gives you an example of 
		// how it can be done, as this how you will this operation implemented in 
		// most libraries).
		static void multiply(const Matrix4x4 &a, const Matrix4x4& b, Matrix4x4 &c) {
#if 0 
			for (uint8_t i = 0; i < 4; ++i) {
				for (uint8_t j = 0; j < 4; ++j) {
					c[i][j] = a[i][0] * b[0][j] + a[i][1] * b[1][j] +
						a[i][2] * b[2][j] + a[i][3] * b[3][j];
				}
			}
#else 
			// A restric qualified pointer (or reference) is basically a promise
			// to the compiler that for the scope of the pointer, the target of the
			// pointer will only be accessed through that pointer (and pointers
			// copied from it.
			const float * __restrict ap = &a.x[0][0];
			const float * __restrict bp = &b.x[0][0];
			float * __restrict cp = &c.x[0][0];

			float a0, a1, a2, a3;

			a0 = ap[0];
			a1 = ap[1];
			a2 = ap[2];
			a3 = ap[3];

			cp[0] = a0 * bp[0] + a1 * bp[4] + a2 * bp[8] + a3 * bp[12];
			cp[1] = a0 * bp[1] + a1 * bp[5] + a2 * bp[9] + a3 * bp[13];
			cp[2] = a0 * bp[2] + a1 * bp[6] + a2 * bp[10] + a3 * bp[14];
			cp[3] = a0 * bp[3] + a1 * bp[7] + a2 * bp[11] + a3 * bp[15];

			a0 = ap[4];
			a1 = ap[5];
			a2 = ap[6];
			a3 = ap[7];

			cp[4] = a0 * bp[0] + a1 * bp[4] + a2 * bp[8] + a3 * bp[12];
			cp[5] = a0 * bp[1] + a1 * bp[5] + a2 * bp[9] + a3 * bp[13];
			cp[6] = a0 * bp[2] + a1 * bp[6] + a2 * bp[10] + a3 * bp[14];
			cp[7] = a0 * bp[3] + a1 * bp[7] + a2 * bp[11] + a3 * bp[15];

			a0 = ap[8];
			a1 = ap[9];
			a2 = ap[10];
			a3 = ap[11];

			cp[8] = a0 * bp[0] + a1 * bp[4] + a2 * bp[8] + a3 * bp[12];
			cp[9] = a0 * bp[1] + a1 * bp[5] + a2 * bp[9] + a3 * bp[13];
			cp[10] = a0 * bp[2] + a1 * bp[6] + a2 * bp[10] + a3 * bp[14];
			cp[11] = a0 * bp[3] + a1 * bp[7] + a2 * bp[11] + a3 * bp[15];

			a0 = ap[12];
			a1 = ap[13];
			a2 = ap[14];
			a3 = ap[15];

			cp[12] = a0 * bp[0] + a1 * bp[4] + a2 * bp[8] + a3 * bp[12];
			cp[13] = a0 * bp[1] + a1 * bp[5] + a2 * bp[9] + a3 * bp[13];
			cp[14] = a0 * bp[2] + a1 * bp[6] + a2 * bp[10] + a3 * bp[14];
			cp[15] = a0 * bp[3] + a1 * bp[7] + a2 * bp[11] + a3 * bp[15];
#endif 
		}

		// return a transposed copy of the current matrix as a new matrix
		Matrix4x4 transposed() const {
#if 0 
			Matrix4x4 t;
			for (uint8_t i = 0; i < 4; ++i) {
				for (uint8_t j = 0; j < 4; ++j) {
					t[i][j] = x[j][i];
				}
			}
			return t;
#else 
			return Matrix4x4(x[0][0], x[1][0], x[2][0], x[3][0],
							 x[0][1], x[1][1], x[2][1], x[3][1],
							 x[0][2], x[1][2], x[2][2], x[3][2],
							 x[0][3], x[1][3], x[2][3], x[3][3]);
#endif 
		}

		// transpose itself
		Matrix4x4& transpose() {
			Matrix4x4 tmp(x[0][0], x[1][0], x[2][0], x[3][0],
				x[0][1], x[1][1], x[2][1], x[3][1],
				x[0][2], x[1][2], x[2][2], x[3][2],
				x[0][3], x[1][3], x[2][3], x[3][3]);
			*this = tmp;

			return *this;
		}

		void multVecMatrix(const Vector3 &src, Vector3 &dst) const {
			float a, b, c, w;

			a = src[0] * x[0][0] + src[1] * x[1][0] + src[2] * x[2][0] + x[3][0];
			b = src[0] * x[0][1] + src[1] * x[1][1] + src[2] * x[2][1] + x[3][1];
			c = src[0] * x[0][2] + src[1] * x[1][2] + src[2] * x[2][2] + x[3][2];
			w = src[0] * x[0][3] + src[1] * x[1][3] + src[2] * x[2][3] + x[3][3];

			dst.x = a / w;
			dst.y = b / w;
			dst.z = c / w;
		}

		void multDirMatrix(const Vector3 &src, Vector3 &dst) const {
			float a, b, c;

			a = src[0] * x[0][0] + src[1] * x[1][0] + src[2] * x[2][0];
			b = src[0] * x[0][1] + src[1] * x[1][1] + src[2] * x[2][1];
			c = src[0] * x[0][2] + src[1] * x[1][2] + src[2] * x[2][2];

			dst.x = a;
			dst.y = b;
			dst.z = c;
		}

		Matrix4x4 inverse() const {
			int i, j, k;
			Matrix4x4 s;
			Matrix4x4 t(*this);

			// Forward elimination
			for (i = 0; i < 3; i++) {
				int pivot = i;

				float pivotsize = t[i][i];

				if (pivotsize < 0)
					pivotsize = -pivotsize;

				for (j = i + 1; j < 4; j++) {
					float tmp = t[j][i];

					if (tmp < 0)
						tmp = -tmp;

					if (tmp > pivotsize) {
						pivot = j;
						pivotsize = tmp;
					}
				}

				if (pivotsize == 0) {
					// Cannot invert singular matrix
					return Matrix4x4();
				}

				if (pivot != i) {
					for (j = 0; j < 4; j++) {
						float tmp;

						tmp = t[i][j];
						t[i][j] = t[pivot][j];
						t[pivot][j] = tmp;

						tmp = s[i][j];
						s[i][j] = s[pivot][j];
						s[pivot][j] = tmp;
					}
				}

				for (j = i + 1; j < 4; j++) {
					float f = t[j][i] / t[i][i];

					for (k = 0; k < 4; k++) {
						t[j][k] -= f * t[i][k];
						s[j][k] -= f * s[i][k];
					}
				}
			}

			// Backward substitution
			for (i = 3; i >= 0; --i) {
				float f;

				if ((f = t[i][i]) == 0) {
					// Cannot invert singular matrix
					return Matrix4x4();
				}

				for (j = 0; j < 4; j++) {
					t[i][j] /= f;
					s[i][j] /= f;
				}

				for (j = 0; j < i; j++) {
					f = t[j][i];

					for (k = 0; k < 4; k++) {
						t[j][k] -= f * t[i][k];
						s[j][k] -= f * s[i][k];
					}
				}
			}

			return s;
		}

		// set current matrix to its inverse
		const Matrix4x4& invert() {
			*this = inverse();
			return *this;
		}

		friend std::ostream& operator << (std::ostream &s, const Matrix4x4 &m) {
			std::ios_base::fmtflags oldFlags = s.flags();
			int width = 12; // total with of the displayed number 
			s.precision(5); // control the number of displayed decimals 
			s.setf(std::ios_base::fixed);

			s << "|" << std::setw(width) << m[0][0] <<
				" " << std::setw(width) << m[0][1] <<
				" " << std::setw(width) << m[0][2] <<
				" " << std::setw(width) << m[0][3] << "|\n" <<

				"|" << std::setw(width) << m[1][0] <<
				" " << std::setw(width) << m[1][1] <<
				" " << std::setw(width) << m[1][2] <<
				" " << std::setw(width) << m[1][3] << "|\n" <<

				"|" << std::setw(width) << m[2][0] <<
				" " << std::setw(width) << m[2][1] <<
				" " << std::setw(width) << m[2][2] <<
				" " << std::setw(width) << m[2][3] << "|\n" <<

				"|" << std::setw(width) << m[3][0] <<
				" " << std::setw(width) << m[3][1] <<
				" " << std::setw(width) << m[3][2] <<
				" " << std::setw(width) << m[3][3] << "|";

			s.flags(oldFlags);
			return s;
		}

		static Matrix4x4 LookAt(const Vector3& from, const Vector3& to, const Vector3& up) {
			Vector3 front(Vector3::Normalize(to - from));
			Vector3 right(Vector3::Normalize(Vector3::Cross(front, up)));
			Vector3 upper(Vector3::Cross(right, front));

			Matrix4x4 Result;
			Result[0][0] =  right.x;
			Result[1][0] =  right.y;
			Result[2][0] =  right.z;
			Result[0][1] =  upper.x;
			Result[1][1] =  upper.y;
			Result[2][1] =  upper.z;
			Result[0][2] = -front.x;
			Result[1][2] = -front.y;
			Result[2][2] = -front.z;
			Result[3][0] = -Vector3::Dot(right, from);
			Result[3][1] = -Vector3::Dot(upper, from);
			Result[3][2] =  Vector3::Dot(front, from);

			return Result;
		}

		// right hand
		static Matrix4x4 Perspective(const float &fovy, const float &aspect, const float &zNear, const float &zFar) {
			float const tanHalfFovy = tan(Deg2Rad * fovy / 2.0);

			Matrix4x4 Result(0.0);
			Result[0][0] = 1.0 / (aspect * tanHalfFovy);
			Result[1][1] = 1.0 / (tanHalfFovy);
			Result[2][3] = -1.0;
			Result[2][2] = -(zFar + zNear) / (zFar - zNear);
			Result[3][2] = -(2.0 * zFar * zNear) / (zFar - zNear);

			return Result;
		}

		static Matrix4x4 Translate(const Matrix4x4 &model, const Vector3 moveVec);

		static Matrix4x4 Rotate(const Matrix4x4 &model, const Vector3 rotVec);
		static Matrix4x4 Rotate(const Matrix4x4 &model, const Quaternion q);

		static Matrix4x4 Scale(const Matrix4x4 &model, const float &factor);

	};
}