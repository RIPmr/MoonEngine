#pragma once
#include "Neuron.h"
#include "NVariable.h"
#include "Optimizer.h"
#include "NFunction.h"
#include "NNManager.h"
#include "Plotter.h"

namespace MOON {
	namespace NN {
		extern class NGraph;
		class NNBase {
		public:
			NGraph* graph;

			Optimizer* opt;

			Neuron* loss;
			Neuron* label;
			Neuron* feature;
			Neuron* predict;

			// dimension of input vector
			unsigned int in_size;
			// dimension of output vector
			unsigned int out_size;

			NNBase(const float &in_size, const float &out_size, NGraph* graph = NNM::globalNNGraph) : 
				graph(graph), in_size(in_size), out_size(out_size) {
				opt = nullptr; loss = nullptr; feature = nullptr; label = nullptr; predict = nullptr;
			}

			virtual ~NNBase() = default;

		};
	}
}
