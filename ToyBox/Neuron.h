#pragma once
#include <cmath>
#include <vector>
#include <iostream>
#include <algorithm>
#include <assert.h>

#include "Matrix.h"
#include "Utility.h"

namespace MOON {
	namespace NN {
		extern class NGraph;
		class Neuron {
		public:
			unsigned int id;
			std::vector<Neuron*> parent;
			std::vector<Neuron*> child;
			Matrix* value;
			Matrix* jacobi;
			NGraph* graph;

			Neuron(NGraph* graph);
			Neuron(Neuron* singleParent, NGraph* graph);
			Neuron(std::vector<Neuron*> &parent, NGraph* graph);

			virtual ~Neuron() {
				parent.clear();
				child.clear();
				if (value != NULL) delete value;
				if (jacobi != NULL) delete jacobi;
			}

			friend bool operator==(const Neuron &n1, const Neuron &n2);

			void AddParent(Neuron* p) {
				parent.push_back(p);
				p->child.push_back(this);
			}
			void RemoveParent(Neuron* target) {
				Utility::RemoveElem<Neuron>(target->child, this);
				Utility::RemoveElem<Neuron>(parent, target);
			}

			void SetGraph(NGraph* g);

			virtual void Compute() = 0;
			virtual void SetVal(const Matrix &val) {}

			void Forward() {
				//std::cout << "forward: " << this->id << std::endl;
				for (auto iter = parent.begin(); iter != parent.end(); iter++) {
					if ((*iter)->value == NULL) (*iter)->Forward();
				}
				this->Compute();
			}

			/*
			calculate Jacobian Matrix from parent node to this node
			*/
			virtual Matrix CalcJacobian(const Neuron* parent) {
				return Matrix(Dimension(), 1);
			}

			int Dimension() const {
				assert(value != NULL);
				return value->size.x * value->size.y;
			}

			Vector2 Shape() const {
				if (value != NULL) return value->size;
				else {
					//std::cout << "[NN]: value is empty! id: " << id << std::endl;
					return Vector2::ZERO();
				}
			}

			void ResetVal(bool recursive = true) {
				if (value != NULL) delete value;
				value = NULL;
				//std::cout << "reset: " << id << std::endl;

				if (recursive) {
					for (auto iter = child.begin(); iter != child.end(); iter++) {
						(*iter)->ResetVal();
					}
				}
			}

			void ClearJacobian() {
				if (jacobi != NULL) delete jacobi;
				jacobi = NULL;
			}

			/*
			calculate Jacobian Matrix from result node to this node
			r : result node
			*/
			Matrix Backward(const Neuron* r) {
				if (jacobi == NULL) {
					if (this == r) {
						jacobi = new Matrix(Dimension(), 1);
					} else {
						jacobi = new Matrix(r->Dimension(), Dimension(), 0);

						for (auto iter = child.begin(); iter != child.end(); iter++) {
							if ((*iter)->value != NULL) {
								*jacobi += (*iter)->Backward(r) * (*iter)->CalcJacobian(this);
							}
						}
					}
				}

				return *jacobi;
			}
		};
	}
}