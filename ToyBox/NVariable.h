#pragma once
#include "Neuron.h"

namespace MOON {
	namespace NN {
		enum Activation {
			relu,
			logistic,
			tanh,
			softmax,
			none
		};

		enum LossFunction {
			MSE_,
			CE_SM
		};

		class NVariable : public Neuron {
		public:
			Vector2 dimension;
			bool trainable;

			void Compute() override {}

			NVariable(const Vector2 &dim, NGraph* graph, const bool &init = false, const bool &trainable = true) : Neuron(this, graph) {
				this->dimension = dim;

				if (init) {
					this->value = new Matrix(Matrix::Random_Normal(0, 0.001f, dim));
				}

				this->trainable = trainable;

				std::cout << "[NN] new [Variable] created: " << std::endl;
				std::cout << "  - id: " << id << std::endl;
				std::cout << "  - shape: " << dim << std::endl;
				std::cout << "  - trainable: " << trainable << std::endl;
				if (init) {
					std::cout << "  - val: " << std::endl;
					std::cout << *value << std::endl;
				} else {
					std::cout << "  - val: NULL\n" << std::endl;
				}
			}

			void ReInit(const Vector2 &dim, const float &loc = 0, const float &scale = 0.001f) {
				dimension = dim;
				if (value != NULL) delete value;
				value = new Matrix(Matrix::Random_Normal(loc, scale, dim));
			}

			void SetVal(const Matrix &val) override {
				//assert(val.size == dimension);
				if (val.size != dimension) throw -1;

				ResetVal();
				this->value = new Matrix(val);
			}
		};

		// A = B + C
		class NAdd : public Neuron {
		public:
			NAdd(NGraph* graph) : Neuron(graph) {
				std::cout << "[NN]: new [Add] node created: " << std::endl;
				std::cout << "  - id: " << id << std::endl;
				std::cout << "  - no parent." << std::endl;
			}
			NAdd(std::vector<Neuron*> parent, NGraph* graph) : Neuron(parent, graph) {
				std::cout << "[NN]: new [Add] node created: " << std::endl;
				std::cout << "  - id: " << id << std::endl;
				std::cout << "  - parent[0] id : " << parent[0]->id << std::endl;
				std::cout << "  - parent[1] id : " << parent[1]->id << std::endl << std::endl;
			}

			void Compute() override {
				//assert(parent.size() == 2 && parent[0]->Shape() == parent[1]->Shape());
				if (parent.size() != 2 || parent[0]->Shape() != parent[1]->Shape()) throw -1;

				if (value != NULL) delete value;
				value = new Matrix(*(parent[0]->value) + *(parent[1]->value));

				// sum up all parents
				/*value = new Matrix(parent[0]->Shape(), 0);
				for (auto iter = parent.begin(); iter != parent.end(); iter++) {
					*value += *(*iter)->value;
				}*/
			}

			Matrix CalcJacobian(const Neuron* parent) override {
				return Matrix(Dimension(), 1);
			}
		};

		// A = B^n
		class NPow : public Neuron {
		public:
			float n;

			NPow(NGraph* graph) : Neuron(graph), n(2) {
				std::cout << "[NN]: new [Pow] node created: " << std::endl;
				std::cout << "  - id: " << id << std::endl;
				std::cout << "  - no parent." << std::endl;
			}
			NPow(Neuron* singleParent, float& n, NGraph* graph) : Neuron(singleParent, graph), n(n) {
				std::cout << "[NN]: new [Pow] node created: " << std::endl;
				std::cout << "  - id: " << id << std::endl;
				std::cout << "  - parent id : " << singleParent->id << std::endl << std::endl;
			}

			void Compute() override {
				//assert(parent.size() == 1);
				if (parent.size() != 1) throw - 1;

				if (value != NULL) delete value;
				value = new Matrix(Matrix::Pow(*parent[0]->value, n));
			}

			Matrix CalcJacobian(const Neuron* parent) override {
				return Matrix::Diag(
					Matrix::Pow(*this->parent[0]->value, n - 1) * n
				);
			}
		};

		// A = B^T * C
		class NDot : public Neuron {
		public:
			NDot(NGraph* graph) : Neuron(graph) {
				std::cout << "[NN]: new [NDot] node created: " << std::endl;
				std::cout << "  - id: " << id << std::endl;
				std::cout << "  - no parent." << std::endl;
			}
			NDot(std::vector<Neuron*> parent, NGraph* graph) : Neuron(parent, graph) {
				std::cout << "[NN]: new [NDot] node created: " << std::endl;
				std::cout << "  - id: " << id << std::endl;
				std::cout << "  - parent[0] id : " << parent[0]->id << std::endl;
				std::cout << "  - parent[1] id : " << parent[1]->id << std::endl;
			}

			void Compute() override {
				//assert(parent.size() == 2 && parent[0]->Dimension() == parent[1]->Dimension());
				if (parent.size() != 2 || parent[0]->Dimension() != parent[1]->Dimension()) throw - 1;

				if (value != NULL) delete value;
				// result is a 1x1 matrix (scalar)
				value = new Matrix(parent[0]->value->transposed() * *(parent[1]->value));
			}

			Matrix CalcJacobian(const Neuron* parent) override {
				if (parent == this->parent[0]) {
					return this->parent[1]->value->transposed();
				} else {
					return this->parent[0]->value->transposed();
				}
			}
		};

		// A = B * C
		class NMatMul : public Neuron {
		public:
			NMatMul(NGraph* graph) : Neuron(graph) {
				std::cout << "[NN]: new [NMatMul] node created: " << std::endl;
				std::cout << "  - id: " << id << std::endl;
				std::cout << "  - no parent." << std::endl;
			}

			NMatMul(std::vector<Neuron*> parent, NGraph* graph) : Neuron(parent, graph) {
				std::cout << "[NN]: new [NMatMul] node created: " << std::endl;
				std::cout << "  - id: " << id << std::endl;
				std::cout << "  - parent[0] id : " << parent[0]->id << std::endl;
				std::cout << "  - parent[1] id : " << parent[1]->id << std::endl << std::endl;
			}

			void Compute() override {
				if (parent[0]->Shape()[1] != parent[1]->Shape()[0]) {
					std::cout << "[NMatMul] Error: " << std::endl;
					std::cout << "  - parent[0] id : " << parent[0]->id << std::endl;
					std::cout << "  - parent[1] id : " << parent[1]->id << std::endl << std::endl;
					std::cout << "  - parent[0] shape: " << parent[0]->Shape() << std::endl;
					std::cout << "  - parent[1] shape: " << parent[1]->Shape() << std::endl;
				}
				//assert(parent.size() == 2 && parent[0]->Shape()[1] == parent[1]->Shape()[0]);
				if (parent.size() != 2 || parent[0]->Shape()[1] != parent[1]->Shape()[0]) throw - 1;

				if (value != NULL) delete value;
				value = new Matrix(*(parent[0]->value) * *(parent[1]->value));
			}

			Matrix CalcJacobian(const Neuron* parent) override {
				Matrix zeros(Dimension(), parent->Dimension(), 0);
				if (parent == this->parent[0]) {
					return Matrix::FillDiagonal(zeros, this->parent[1]->value->transposed());
				} else {
					Matrix row_sort = Matrix::Arange(0, Dimension()).
										Reshape(Shape().y, Shape().x).
										transposed().Ravel();
					Matrix col_sort = Matrix::Arange(0, parent->Dimension()).
										Reshape(parent->Shape().y, parent->Shape().x).
										transposed().Ravel();
					return Matrix::FillDiagonal(zeros, *this->parent[0]->value).Select(row_sort, col_sort);
				}
			}
		};

		class Logistic : public Neuron {
		public:
			Logistic(NGraph* graph) : Neuron(graph) {
				std::cout << "[NN]: new [Logistic] node created: " << std::endl;
				std::cout << "  - id: " << id << std::endl;
				std::cout << "  - no parent." << std::endl;
			}

			Logistic(Neuron* singleParent, NGraph* graph) : Neuron(singleParent, graph) {
				std::cout << "[NN]: new [Logistic] node created: " << std::endl;
				std::cout << "  - id: " << id << std::endl;
				std::cout << "  - parent[0] id : " << parent[0]->id << std::endl << std::endl;
			}

			void Compute() override {
				Matrix* pval = parent[0]->value;
				if (value != NULL) delete value;
				value = &pval->Map([](float &i) -> void {
					//i = 1.0 / MoonMath::ExpTylor(-i > 1e2 ? 1e2 : -i);
					i = 1.0 / std::exp(-i > 1e2 ? 1e2 : -i);
				});
			}

			Matrix CalcJacobian(const Neuron* parent) override {
				Matrix res;
				Matrix::multiElem(*value, value->Map([](float &i) -> void {
					i = 1.0f - i;
				}), res);
				return Matrix::Diag(res.Ravel());
			}
		};

		class ReLU : public Neuron {
		public:
			ReLU(NGraph* graph) : Neuron(graph) {
				std::cout << "[NN]: new [ReLU] node created: " << std::endl;
				std::cout << "  - id: " << id << std::endl;
				std::cout << "  - no parent." << std::endl;
			}

			ReLU(Neuron* singleParent, NGraph* graph) : Neuron(singleParent, graph) {
				std::cout << "[NN]: new [ReLU] node created: " << std::endl;
				std::cout << "  - id: " << id << std::endl;
				std::cout << "  - parent[0] id : " << parent[0]->id << std::endl << std::endl;
			}

			void Compute() override {
				if (value != NULL) delete value;
				value = new Matrix(parent[0]->value->Map([](float &i) -> void {
					i = i > 0 ? i : (0.1f * i);
				}));
			}

			Matrix CalcJacobian(const Neuron* parent) override {
				Matrix res = this->parent[0]->value->Ravel();
				res.Mapi([](float &i) -> void {
					i = i > 0 ? 1.0f : 0.1f;
				});
				return Matrix::Diag(res);
			}
		};

		class SoftMax : public Neuron {
		public:
			SoftMax(NGraph* graph) : Neuron(graph) {
				std::cout << "[NN]: new [SoftMax] node created: " << std::endl;
				std::cout << "  - id: " << id << std::endl;
				std::cout << "  - no parent." << std::endl;
			}
			SoftMax(Neuron* singleParent, NGraph* graph) : Neuron(singleParent, graph) {
				std::cout << "[NN]: new [SoftMax] node created: " << std::endl;
				std::cout << "  - id: " << id << std::endl;
				std::cout << "  - parent[0] id : " << parent[0]->id << std::endl << std::endl;
			}

			// TODO: need some fix
			static Matrix Calc(Matrix &a) {
				a.Mapi([](float &i) -> void{
					i = i > 1e2 ? 1e2 : i;
				});
				Matrix ep = a.Map([](float &i) -> void {
					//i = MoonMath::ExpTylor(i);
					i = std::exp(i);
				});
				float sum = ep.sum();
				return ep.Map([&](float &i) -> void {
					i = i / sum;
				});
			}

			void Compute() override {
				if (value != NULL) delete value;
				value = new Matrix(this->Calc(*parent[0]->value));
			}

			// 无用
			Matrix CalcJacobian(const Neuron* parent) override {
				return Matrix(Dimension(), 1);
			}
		};

		// 对第一个父节点施加 SoftMax 之后
		// 再与第二个父节点作为标签 (One-Hot编码的向量) 计算交叉熵
		class CrossEntropy_SoftMax : public Neuron {
		public:
			CrossEntropy_SoftMax(NGraph* graph) : Neuron(graph) {}
			CrossEntropy_SoftMax(std::vector<Neuron*> parent, NGraph* graph) : Neuron(parent, graph) {}

			void Compute() override {
				Matrix res, prob = SoftMax::Calc(*parent[0]->value);
				prob.Mapi([](float &i) -> void {
					i = std::log(i + 1e-10);
				});
				Matrix::multiElem(*parent[1]->value, prob, res);
				if (value != NULL) delete value;
				value = new Matrix(1, -res.sum());
			}

			Matrix CalcJacobian(const Neuron* parent) override {
				Matrix prob = SoftMax::Calc(*this->parent[0]->value);
				if (parent == this->parent[0]) {
					return (prob - *this->parent[1]->value).Ravel();
				} else {
					std::cout << "cannot calc jacobian for label" << std::endl;
					//assert(false);
					throw - 1;
				}
			}
		};

		class MSE : public Neuron {
		public:
			MSE(NGraph* graph) : Neuron(graph) {}
			MSE(std::vector<Neuron*> parent, NGraph* graph) : Neuron(parent, graph) {}

			void Compute() override {
				//assert(parent.size() == 2 && parent[0]->Shape() == parent[1]->Shape());
				if (parent.size() != 2 || parent[0]->Shape() != parent[1]->Shape()) throw - 1;

				float res = 0;
				for (int i = 0; i < parent[0]->Shape().x; i++)
					for (int j = 0; j < parent[0]->Shape().y; j++)
						res += pow((*parent[0]->value)[i][j] - (*parent[1]->value)[i][j], 2.0f);

				if (value != NULL) delete value;
				value = new Matrix(1, res / parent[0]->Dimension());
			}

			Matrix CalcJacobian(const Neuron* parent) override {
				Matrix jacobian(1, parent->Dimension(), 0);
				Matrix P0 = this->parent[0]->value->Ravel();
				Matrix P1 = this->parent[1]->value->Ravel();
				for (int i = 0; i < parent->Dimension(); i++) {
					jacobian[0][i] = 2.0f * (P0[0][i] - P1[0][i]) / parent->Dimension();
					if (parent == this->parent[1]) jacobian[0][i] *= -1;
				}
				return jacobian;
			}
		};

	}
}