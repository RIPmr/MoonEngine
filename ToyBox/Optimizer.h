#pragma once
#include <cmath>
#include <vector>
#include <cstdlib>
#include <iostream>
#include <algorithm>
#include <unordered_map>

#include "NVariable.h"

namespace MOON {
	namespace NN {
		extern class NGraph;
		class Optimizer {
		public:
			unsigned int id;
			NGraph* graph;
			Neuron* target;

			int batchSize;
			float lr; // learning rate

			std::unordered_map<int, Matrix> acc_gradient;
			int acc_cnt;

			Optimizer(NGraph* graph, Neuron* target, float learning_rate, int batch_size);
			virtual ~Optimizer() = default;

			// 前向传播计算结果节点的值
			void Forward();
			// 反向传播计算结果节点对各个节点的梯度
			void Backward();
			// 清空梯度
			void ClearGrad();
			// 返回一个Mini Batch的样本的平均梯度
			Matrix GetGradient(const Neuron* node);
			// 利用Mini Batch的平均梯度以各种不同的方法更新变量值
			virtual void Update() = 0;
			// 计算并累加样本的梯度，一个Mini Batch结束后执行变量更新
			void Step();

		};
	}
}
