#pragma once
#include <cmath>
#include <cstdlib>
#include <iomanip>
#include <iostream>
#include <algorithm>
#include <assert.h>

#include "MathUtils.h"
#include "Vector3.h"

namespace MOON {
	class Matrix {
	private:
		static void GetID(const int &cnt, const int &tarCol, int &row, int &col) {
			row = cnt / tarCol;
			col = cnt - row * tarCol;
			if (col > 0) row++;
			else col = tarCol;
			row--; col--;
		}

		void SetVal(const float &val) {
			x = new float*[size.x];
			for (int i = 0; i < size.x; i++) {
				x[i] = new float[size.y];
				for (int j = 0; j < size.y; j++) x[i][j] = 0;
				if (i < size.y) x[i][i] = val;
			}
		}

		void SetVal(const std::vector<float> &val) {
			x = new float*[size.x];
			for (int i = 0; i < size.x; i++) {
				x[i] = new float[size.y];
				for (int j = 0; j < size.y; j++) x[i][j] = val[i * size.y + j];
			}
		}

		void Clear() {
			if (x != NULL && size != Vector2::ZERO()) {
				for (int i = 0; i < size.x; i++) delete[] x[i];
				delete[] x;
				x = NULL;
			}
		}

	public:
		float** x;
		Vector2 size;

		Matrix() {
			x = NULL;
			size = Vector2::ZERO();
		}
		Matrix(const float &rank, const float &i) {
			size.setValue(rank, rank);
			SetVal(i);
		}
		Matrix(const Vector2 &size, const float &i) {
			this->size.setValue(size);
			SetVal(i);
		}
		Matrix(const int &row, const int &col, const float &i) {
			this->size.setValue(row, col);
			SetVal(i);
		}
		Matrix(const std::vector<float> &data) {
			this->size.setValue(1, data.size());
			SetVal(data);
		}
		Matrix(const Vector2 &size, const std::vector<float> &data) {
			this->size.setValue(size);
			SetVal(data);
		}
		Matrix(const int &row, const int &col, const std::vector<float> &data) {
			this->size.setValue(row, col);
			SetVal(data);
		}
		Matrix(const Matrix &m) {
			//Clear();
			size = m.size;
			x = new float*[size.x];
			for (int i = 0; i < size.x; i++) {
				x[i] = new float[size.y];
				for (int j = 0; j < size.y; j++)
					x[i][j] = m[i][j];
			}
		}
		~Matrix() {
			Clear();
		}

		void NewMat(const int &row, const int &col, const float &i) {
			Clear();
			this->size.setValue(row, col);
			SetVal(i);
		}

		void NewMat(const Vector2 &size, const float &i) {
			Clear();
			this->size.setValue(size);
			SetVal(i);
		}

		static Matrix identity(float rank) { return Matrix(rank, 1); }
		static Matrix fill(const int &row, const int &col, const float &val) {
			Matrix m; m.size.setValue(row, col);
			m.x = new float*[row];
			for (int i = 0; i < row; i++) {
				m.x[i] = new float[col];
				for (int j = 0; j < col; j++) m.x[i][j] = val;
			}
			return m;
		}
		static Matrix fill(const Vector2 &size, const float &val) { 
			Matrix m; m.size = size;
			m.x = new float*[size.x];
			for (int i = 0; i < size.x; i++) {
				m.x[i] = new float[size.y];
				for (int j = 0; j < size.y; j++) m.x[i][j] = val;
			}
			return m;
		}

		const float* operator[](uint8_t i) const { return x[i]; }
		float* operator[](uint8_t i) { return x[i]; }

		Matrix& operator=(const Matrix& v) {
			Clear();
			size = v.size;
			x = new float*[size.x];
			for (int i = 0; i < size.x; i++) {
				x[i] = new float[size.y];
				for (int j = 0; j < size.y; j++)
					x[i][j] = v[i][j];
			}
			return *this;
		}

		bool operator==(const Matrix& v) const {
			if (size != v.size) return false;
			for (int i = 0; i < size.x; i++) {
				for (int j = 0; j < size.y; j++) {
					if (x[i][j] != v[i][j]) return false;
				}
			}
			return true;
		}

		bool operator!=(const Matrix& v) const {
			return !(*this == v);
		}

		// Multiply the current matrix with another matrix (rhs)
		Matrix operator*(const Matrix& v) const {
			if (this->size.y != v.size.x) {
				std::cout << "size of two matrix are not the same, can't apply * operation!" << std::endl;
				std::cout << "size of mat a: " << this->size << std::endl;
				std::cout << "size of mat b: " << v.size << std::endl;
				assert(this->size.y == v.size.x);
				return *this;
			}

			Matrix tmp(Vector2(this->size.x, v.size.y), 1);
			multiply(*this, v, tmp);
			return tmp;
		}

		Matrix operator+(const Matrix& v) const {
			const Matrix* big; const Matrix* lit;
			if (this->dimension() > v.dimension()) { big = this; lit = &v; } 
			else { big = &v; lit = this; }

			if (lit->dimension() == 1) {
				Matrix tmp(*big);
				for (int i = 0; i < big->size.x; i++)
					for (int j = 0; j < big->size.y; j++)
						tmp[i][j] += (*lit)[0][0];
				return tmp;
			} else if (this->size == v.size) {
				Matrix tmp(size, 0);
				for (int i = 0; i < size.x; i++)
					for (int j = 0; j < size.y; j++)
						tmp[i][j] = x[i][j] + v[i][j];
				return tmp;
			} else {
				std::cout << "size of two matrix are not the same, can't apply + operation!" << std::endl;
				std::cout << "size of mat a: " << this->size << std::endl;
				std::cout << "size of mat b: " << v.size << std::endl;
				assert(this->size == v.size);
				return *this;
			}
		}

		Matrix operator-(const Matrix& v) const {
			const Matrix* big; const Matrix* lit;
			if (this->dimension() > v.dimension()) { big = this; lit = &v; } 
			else { big = &v; lit = this; }

			if (lit->dimension() == 1) {
				Matrix tmp(big->size, 0);
				for (int i = 0; i < big->size.x; i++)
					for (int j = 0; j < big->size.y; j++)
						if (big == this) tmp[i][j] = x[i][j] - v[0][0];
						else tmp[i][j] = x[0][0] - v[i][j];
				return tmp;
			} else if (this->size == v.size) {
				Matrix tmp(size, 0);
				for (int i = 0; i < size.x; i++)
					for (int j = 0; j < size.y; j++)
						tmp[i][j] = x[i][j] - v[i][j];
				return tmp;
			} else {
				std::cout << "size of two matrix are not the same, can't apply - operation!" << std::endl;
				std::cout << "size of mat a: " << this->size << std::endl;
				std::cout << "size of mat b: " << v.size << std::endl;
				assert(this->size == v.size);
				return *this;
			}
		}

		Matrix& operator+=(const Matrix& v) {
			if (this->size != v.size) {
				std::cout << "size of two matrix are not the same, can't apply + operation!" << std::endl;
				std::cout << "size of mat a: " << this->size << std::endl;
				std::cout << "size of mat b: " << v.size << std::endl;
				assert(this->size == v.size);
				return *this;
			}

			for (int i = 0; i < size.x; i++)
				for (int j = 0; j < size.y; j++)
					x[i][j] = x[i][j] + v[i][j];

			return *this;
		}

		friend Matrix operator*(const float &t, const Matrix &v) {
			Matrix temp(v);
			for (int i = 0; i < v.size.x; i++)
				for (int j = 0; j < v.size.y; j++)
					temp[i][j] *= t;
			return temp;
		}

		friend Matrix operator*(const Matrix &v, const float &t) {
			Matrix temp(v);
			for (int i = 0; i < v.size.x; i++)
				for (int j = 0; j < v.size.y; j++)
					temp[i][j] *= t;
			return temp;
		}

		friend Matrix operator/(const Matrix &v, const float &t) {
			Matrix temp(v);
			for (int i = 0; i < v.size.x; i++)
				for (int j = 0; j < v.size.y; j++)
					temp[i][j] /= t;
			return temp;
		}

		friend Matrix operator/(const float &t, const Matrix &v) {
			Matrix temp(v);
			for (int i = 0; i < v.size.x; i++)
				for (int j = 0; j < v.size.y; j++)
					temp[i][j] = t / temp[i][j];
			return temp;
		}

		float sum() const {
			float sum = 0;
			for (int i = 0; i < size.x; i++)
				for (int j = 0; j < size.y; j++)
					sum += x[i][j];
			return sum;
		}

		float mean() const {
			float sum = this->sum();
			return sum / size.x / size.y;
		}

		int dimension() const {
			return size.x * size.y;
		}

		/*
		A   : m x p
		B   : p x n
		dst : m x n
		(AB)ij = Ai1Bij + ai2b2j + ... + aipbpj
		*/
		static void multiply(const Matrix &a, const Matrix& b, Matrix &dst) {
			if (a.size.y != b.size.x) {
				std::cout << "size of two matrix are not the same, can't apply multiply operation!" << std::endl;
				std::cout << "size of mat a: " << a.size << std::endl;
				std::cout << "size of mat b: " << b.size << std::endl;
				assert(a.size.y == b.size.x);
				return;
			}
			if (dst.size != Vector2(a.size.x, b.size.y)) dst.NewMat(a.size.x, b.size.y, 1);

			int p = a.size.y;
			for (int i = 0; i < dst.size.x; i++) {
				for (int j = 0; j < dst.size.y; j++) {
					dst[i][j] = 0;
					for (int k = 0; k < p; k++)
						dst[i][j] += a[i][k] * b[k][j];
				}
			}
		}

		static void AppendRow(const Matrix &a, const Matrix& b, Matrix &dst) {
			//assert (a.size.y == b.size.y);
			if (a.size.y != b.size.y) throw - 1;
			dst.NewMat(a.size.x + b.size.x, a.size.y, 0);
			for (int i = 0; i < dst.size.x; i++) {
				for (int j = 0; j < dst.size.y; j++) {
					if (i < a.size.x) dst[i][j] = a[i][j];
					else dst[i][j] = b[(int)(i - a.size.x)][j];
				}
			}
		}

		static void AppendCol(const Matrix &a, const Matrix& b, Matrix &dst) {
			//assert(a.size.x == b.size.x);
			if (a.size.x != b.size.x) throw - 1;
			dst.NewMat(a.size.x, a.size.y + b.size.y, 0);
			for (int j = 0; j < dst.size.y; j++) {
				for (int i = 0; i < dst.size.x; i++) {
					if (j < a.size.y) dst[i][j] = a[i][j];
					else dst[i][j] = b[i][(int)(j - a.size.y)];
				}
			}
		}

		static void multiElem(const Matrix &a, const Matrix& b, Matrix &dst) {
			const Matrix* big; const Matrix* lit;
			if (a.dimension() > b.dimension()) { big = &a; lit = &b; } 
			else { big = &b; lit = &a; }

			if (lit->dimension() == 1) {
				if (dst.size != big->size) dst.NewMat(big->size, 1);

				for (int i = 0; i < big->size.x; i++)
					for (int j = 0; j < big->size.y; j++)
						dst[i][j] = (*big)[i][j] * (*lit)[0][0];
			} else if (a.size == b.size) {
				if (dst.size != a.size) dst.NewMat(a.size, 1);

				for (int i = 0; i < dst.size.x; i++)
					for (int j = 0; j < dst.size.y; j++)
						dst[i][j] = a[i][j] * b[i][j];
			} else if (b.size.x == 1 && b.size.y == a.size.y) {
				if (dst.size != big->size) dst.NewMat(big->size, 1);

				for (int i = 0; i < big->size.x; i++)
					for (int j = 0; j < big->size.y; j++)
						dst[i][j] = (*big)[i][j] * (*lit)[0][j];
			} else {
				std::cout << "operands could not be broadcast together：" << std::endl;
				std::cout << "size of mat a: " << a.size << std::endl;
				std::cout << "size of mat b: " << b.size << std::endl;
				assert(a.size == b.size);
				return;
			}
		}

		static Matrix Pow(const Matrix &a, const float& n) {
			Matrix res(a);
			for (int i = 0; i < res.size.x; i++) {
				for (int j = 0; j < res.size.y; j++) {
					res[i][j] = pow(res[i][j], n);
				}
			}
			return res;
		}

		// return a transposed copy of the current matrix as a new matrix
		Matrix transposed() const {
			Matrix t(Vector2(size.y, size.x), 1);
			for (uint8_t i = 0; i < size.y; ++i) {
				for (uint8_t j = 0; j < size.x; ++j) {
					t[i][j] = x[j][i];
				}
			}
			return t;
		}

		// transpose itself
		Matrix& transpose() {
			*this = this->transposed();
			return *this;
		}

		friend std::ostream& operator << (std::ostream &s, const Matrix &m) {
			std::ios_base::fmtflags oldFlags = s.flags();
			int width = 12;
			s.precision(5);
			s.setf(std::ios_base::fixed);

			for (int i = 0; i < m.size.x; i++) {
				s << "|";
				for (int j = 0; j < m.size.y; j++) {
					s << std::setw(width) << m[i][j] << " ";
				}
				s << "|\n";
			}

			s.flags(oldFlags);
			return s;
		}

		/*
		return a new matrix consists of some rows selected from this matrix.
		rowArr: 1D array, id list of all selected rows.
		*/
		Matrix Sel_Row(const Matrix &rowArr) {
			// assert(rowArr.size.x == 1);
			Matrix temp(rowArr.size.y, size.y, 0);
			for (int i = 0; i < temp.size.x; i++)
				for (int j = 0; j < temp.size.y; j++)
					temp[i][j] = x[(int)rowArr[0][i]][j];
			return temp;
		}
		Matrix Sel_Row(const std::vector<int> &rowArr) {
			Matrix temp(rowArr.size(), size.y, 0);
			for (int i = 0; i < temp.size.x; i++)
				for (int j = 0; j < temp.size.y; j++)
					temp[i][j] = x[(int)rowArr[i]][j];
			return temp;
		}

		/*
		return a new matrix consists of some cols selected from this matrix.
		colArr: 1D array, id list of all selected cols.
		*/
		Matrix Sel_Col(const Matrix &colArr) {
			// assert(colArr.size.x == 1);
			Matrix temp(size.x, colArr.size.y, 0);
			for (int i = 0; i < temp.size.x; i++)
				for (int j = 0; j < temp.size.y; j++)
					temp[i][j] = x[i][(int)colArr[0][j]];
			return temp;
		}
		Matrix Sel_Col(const std::vector<int> &colArr) {
			Matrix temp(size.x, colArr.size(), 0);
			for (int i = 0; i < temp.size.x; i++)
				for (int j = 0; j < temp.size.y; j++)
					temp[i][j] = x[i][colArr[j]];
			return temp;
		}

		Matrix Select(const Matrix &rowArr, const Matrix &colArr) {
			Matrix res = Sel_Row(rowArr);
			res = res.Sel_Col(colArr);
			return res;
		}

		template <typename R> Matrix Map(R const &func) {
			Matrix temp(*this);

			for (int i = 0; i < size.x; i++)
				for (int j = 0; j < size.y; j++)
					func(temp[i][j]);

			return temp;
		}

		template <typename R> void Mapi(R const &func) {
			for (int i = 0; i < size.x; i++)
				for (int j = 0; j < size.y; j++)
					func(x[i][j]);
		}

		template <class T>
		static Matrix Random(const T &start, const T &end, const Vector2 &size) {
			Matrix temp(size, 1);
			for (int i = 0; i < size.x; i++) for (int j = 0; j < size.y; j++)
				temp[i][j] = MoonMath::RandomRange(start, end);
			return temp;
		}

		/*
		Return a radom Gaussian matrix
		loc(E): Average, that is, the position of the Y-axis
		scale(V): Standard deviation (variance)
		*/
		static Matrix Random_Normal(float loc, float scale, const Vector2 &size) {
			Matrix temp(size, 1);
			for (int i = 0; i < size.x; i++) {
				for (int j = 0; j < size.y; j++) {
					temp[i][j] = MoonMath::GaussianRand(loc, scale);
				}
			}
			return temp;
		}

		static Matrix& FillDiagonal(Matrix &target, const Matrix &value) {
			int n = target.size.x / value.size.x;
			assert(n == target.size.y / value.size.y);

			for (int i = 0; i < n; i++) {
				for (int x = 0; x < value.size.x; x++) {
					for (int y = 0; y < value.size.y; y++) {
						Vector2 start = value.size * i;
						target[(int)start.x + x][(int)start.y + y] = value[x][y];
					}
				}
			}

			return target;
		}

		static Matrix Diag(const Matrix &value) {
			Matrix res;
			if (value.size.x == 1) {
				res.NewMat(value.size.y, value.size.y, 0);
				for (int i = 0; i < value.size.y; i++)
					res[i][i] = value[0][i];
			} else {
				int size = std::min(value.size.x, value.size.y);
				res.NewMat(1, size, 1);
				for (int i = 0; i < size; i++)
					res[0][i] = value[i][i];
			}
			return res;
		}

		static Matrix Arange(int start, int end) {
			Matrix res(Vector2(1, end - start), start);
			for (int i = 1; i < res.size.y; i++) {
				res[0][i] = start + i;
			}
			return res;
		}

		template <typename T>
		static Matrix Arange(T start, T end, T step = 1) {
			Matrix res(Vector2(1, (int)((end - start) / step)), start);
			for (int i = 1; i < (int)res.size.y; i++) {
				res[0][i] = start + i * step;
			}
			return res;
		}

		static Matrix Ravel(const Matrix &input) {
			Matrix res(Vector2(1, input.size.x * input.size.y), 0);
			int row, col;
			for (int i = 0; i < res.size.y; i++) {
				GetID(i + 1, input.size.y, row, col);
				res[0][i] = input[row][col];
			}
			return res;
		}

		Matrix Ravel() {
			Matrix res(Vector2(1, dimension()), 0);
			int row, col;
			for (int i = 0; i < res.size.y; i++) {
				GetID(i + 1, size.y, row, col);
				res[0][i] = x[row][col];
			}
			return res;
		}

		static Matrix Reshape(const Matrix &input, const Vector2 &shape) {
			if (input.dimension() != shape.x * shape.y) {
				std::cout << "size of new matrix are not the same as input, can't reshape it!" << std::endl;
				std::cout << "size of input mat: " << input.size << std::endl;
				std::cout << "reshape size: " << shape << std::endl;
				assert(input.dimension() == shape.x * shape.y);
				return input;
			}
			Matrix res(shape, 1);
			int row, col, cnt;
			for (int i = 0; i < shape.x; i++) {
				for (int j = 0; j < shape.y; j++) {
					cnt = i * shape.y + j + 1;
					GetID(cnt, input.size.y, row, col);
					/*int row = cnt / input.size.y;
					int col = cnt - row * input.size.y;
					if (col > 0) row++;
					else col = input.size.y;*/
					res[i][j] = input[row][col];
					//std::cout << "i:" << i << " j:" << j << "row:" << row-1 << " col:" << col-1 << std::endl;
				}
			}
			return res;
		}

		Matrix Reshape(const Vector2 &shape) {
			if (dimension() != shape.x * shape.y) {
				std::cout << "size of new matrix are not the same as input, can't reshape it!" << std::endl;
				std::cout << "size of input mat: " << size << std::endl;
				std::cout << "reshape size: " << shape << std::endl;
				//assert(dimension() == shape.x * shape.y);
				if (dimension() != shape.x * shape.y) throw - 1;
				return *this;
			}
			Matrix res(shape, 1);
			int row, col, cnt;
			for (int i = 0; i < shape.x; i++) {
				for (int j = 0; j < shape.y; j++) {
					cnt = i * shape.y + j + 1;
					GetID(cnt, size.y, row, col);
					res[i][j] = x[row][col];
				}
			}
			return res;
		}

		Matrix Reshape(const int &row, const int &col) {
			return Reshape(Vector2(row, col));
		}

	};
}

/*
关于动态二维数组的指针访问需要注意的问题：
1、如果是一维数组
	char a[100] 以及 char * a = (char*)malloc(8 * sizeof(char)); 
	是一样的，申请一段连续的内存地址，可以直接指针访问没有区别；
2、二维数组
（1）char a[8][4] = { 0 };
	此时申请的内存地址是连续的（和一维数组一样），所以可以指针指向首地址然后直接下标偏移访问；
（2）char** a = (char**)malloc(8 * sizeof(char*));
     for (int i = 0; i < 8; i++) *(a + i) = (char*)malloc(4 * sizeof(char));
	此时情况就不同了，每一组4个char是连续地址，但不同8个分组间是不连续的地址，
	也就是有8个不同的指针指向离散的地址块，每个块大小为4
	所以不能直接使用首指针加偏移直接访问整个数组
*/