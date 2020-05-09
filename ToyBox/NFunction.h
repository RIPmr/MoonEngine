#pragma once
#include <cmath>
#include <vector>
#include <cstdlib>
#include <iostream>
#include <algorithm>
#include <unordered_map>

#include "Optimizer.h"

namespace MOON {
	namespace NN {
		enum OptimizerType {
			GD,
			RMSP,
			ADAM
		};

		// Gradient Descent
		class GradDesc : public Optimizer {
		public:
			GradDesc(NGraph* graph, Neuron* target, float learningRate = 0.1f, int batch_size = 10) :
				Optimizer(graph, target, learningRate, batch_size) {}

			void Update() override;
		};

		// Root Mean Square Prop
		class RMSProp : public Optimizer {
		public:
			float friction;
			std::unordered_map<int, Matrix> s;

			RMSProp(NGraph* graph, Neuron* target, float learningRate = 0.01f, float friction = 0.9f, int batch_size = 32) :
				Optimizer(graph, target, learningRate, batch_size) {
				assert(0 < friction && friction < 1);
				this->friction = friction;
			}

			void Update() override;
		};

		// Adaptive Moment Estimation
		class Adam : public Optimizer {
		public:
			Vector2 friction;
			std::unordered_map<int, Matrix> s, v;

			Adam(NGraph* graph, Neuron* target, float learningRate = 0.01f, float f_1 = 0.9f, float f_2 = 0.99f, int batch_size = 10) :
				Optimizer(graph, target, learningRate, batch_size) {
				assert(0 < f_1 && f_1 < 1);
				assert(0 < f_2 && f_2 < 1);
				friction.setValue(f_1, f_2);
			}

			void Update() override;
		};

	}
}
