#include "Optimizer.h"
#include "NNManager.h"
#include "NGraph.h"

namespace MOON {
	namespace NN {

		Optimizer::Optimizer(NGraph* graph, Neuron* target, float learning_rate = 0.01f, int batch_size = 12) {
			this->id = graph->GenNeuronID();
			this->graph = graph;
			this->target = target;
			this->batchSize = batch_size;
			this->lr = learning_rate;

			// 为每个参与训练的节点累加一个Mini Batch的全部样本的梯度
			//this->acc_gradient.clear();
			this->acc_cnt = 0;
			graph->AddOpt(this);
		}

		void Optimizer::ClearGrad() {
			acc_gradient.clear();
		}

		// 反向传播计算梯度
		void Optimizer::Backward() {
			graph->ClearJacobian();
			for (auto iter = graph->neurons.begin(); iter != graph->neurons.end(); iter++) {
				if (typeid(**iter) == typeid(NVariable)) {
					NVariable* node = dynamic_cast<NVariable*>(*iter);
					if (node->trainable) {
						node->Backward(target);

						// 最终结果（标量）对节点值（视作向量）的雅克比是一个行向量，将其转置是梯度（列向量）
						// 这里将梯度 reshape 成与节点值相同的形状，方便对节点值进行更新
						Matrix gradient = node->jacobi->Reshape(node->Shape());
						//std::cout << "gradient, id: " << node->id << std::endl;
						//std::cout << gradient << std::endl;

						if (acc_gradient.find(node->id) == acc_gradient.end())
							acc_gradient[node->id] = gradient;
						else acc_gradient[node->id] += gradient;
					}
				}
			}
		}

		// 前向传播计算结果节点
		void Optimizer::Forward() {
			target->Forward();
		}

		Matrix Optimizer::GetGradient(const Neuron* node) {
			auto n = acc_gradient.find(node->id);
			assert(n != acc_gradient.end());
			//std::cout << "node: " << node->id << " : \n" << *node->value << std::endl;
			//std::cout << "gradient of node: " << node->id << " : \n" << (*n).second / batchSize << std::endl;
			return (*n).second / batchSize;
		}

		void Optimizer::Step() {
			this->Forward();
			this->Backward();
			this->acc_cnt++;

			if (acc_cnt >= batchSize) {
				this->Update();
				this->ClearGrad();
				this->acc_cnt = 0;
			}
		}

	}
}