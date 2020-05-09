#pragma once
#include <vector>
#include <imgui.h>

#include "NNBase.h"
#include "MathUtils.h"
#include "Plotter.h"

namespace MOON {
	namespace NN {

		class FNN : public NNBase {
		public:
			// neural number of each hidden layer
			std::vector<int> hidden;
			Activation actFunc;
			// TODO: activation function of each hidden layer
			//std::vector<Activation> actFunc;

			// construct a full connection neural network
			FNN(NGraph* graph, int input_size, int output_size = 1, const std::vector<int> hidden = {10},
				const Activation actFunc = relu, const LossFunction lossFunc = MSE_, const OptimizerType optFunc = ADAM) : 
				NNBase(input_size, output_size, graph), hidden(hidden), actFunc(actFunc) {
				// n-dimensional input vector, not participating in training
				this->feature = new NVariable(Vector2(input_size, 1), graph, false, false);
				// construct full connection layer
				Neuron* input_vec = feature;
				for (auto iter = hidden.begin(); iter != hidden.end(); iter++) {
					int hsize = *iter;

					// y = wx+b
					Neuron* output = new NAdd({
						new NMatMul({
							new NVariable(Vector2(hsize, input_size), graph, true),
							input_vec
						}, graph),
						new NVariable(Vector2(hsize, 1), graph, true)
					}, graph);

					/*
					Another form:
						Neuron* w = new NVariable(Vector2(hsize, input_size), graph, true);
						Neuron* mul = new NMatMul({w, input_vec}, graph);
						Neuron* b = new NVariable(Vector2(hsize, 1), graph, true);
						Neuron* output = new NAdd({mul, b}, graph);
					*/

					output = graph->CreateActivation(actFunc, output);

					input_size = hsize;
					input_vec = output;
				}

				// output neural node
				predict = new NAdd({
					new NMatMul({
						new NVariable(Vector2(output_size, input_size), graph, true),
						input_vec
					}, graph),
					new NVariable(Vector2(output_size, 1), graph, true)
				}, graph);

				// label
				label = new NVariable(Vector2(output_size, 1), graph, false, false);

				// loss function
				loss = graph->CreateLossFunction(lossFunc, { predict, label });

				// optimizer
				opt = graph->CreateOptimizer(optFunc, loss);
			}

			static void regression() {
				int dataLen = 100, epoches = 60;
				std::vector<ImVec2> pdata, predLine;

				Matrix feature = Matrix::Arange(-1.0f, 1.0f, 2.0f / dataLen);
				Matrix data(feature), predData(1, dataLen, 0);
				dataLen = (int)feature.size.y;
				for (int i = 0; i < dataLen; i++) {
					data[0][i] = pow(feature[0][i], 2.0f) + MoonMath::RandomRange(-0.2f, 0.2f);
					pdata.push_back(ImVec2(feature[0][i], data[0][i]));
				}
				std::cout << "random data:" << std::endl;
				std::cout << feature << std::endl;
				std::cout << data << std::endl;

				Plotter *plt = new Plotter();
				plt->AddItem("data", MOON_PlotType::Scatter, 2.0f, ImVec4(1.0f, 0, 0, 1.0f), pdata);
				plt->SetRangeX(-1, 1);
				plt->SpecifyWindow("data_plt");

				FNN nn(NNM::globalNNGraph, 1);
				for (int i = 0; i < epoches; i++) {
					std::cout << "[---------- Epoch: " << i << " ----------]" << std::endl;
					Matrix losses(1, dataLen, 0); predLine.clear();
					for (int x = 0; x < dataLen; x++) {
						nn.feature->SetVal(feature.Sel_Col({ x }));
						//std::cout << " ¡ý feature: \n" << *input->value << std::endl;
						nn.label->SetVal(data.Sel_Col({ x }));
						//std::cout << " - label: \n" << *label->value << std::endl;
						nn.loss->Forward();
						//std::cout << " - predict: \n" << *predict->value << std::endl;
						//std::cout << " - ls: \n" << *loss->value << std::endl;

						losses[0][x] = (*nn.loss->value)[0][0];
						predData[0][x] = (*nn.predict->value)[0][0];
						predLine.push_back(ImVec2(feature[0][x], predData[0][x]));

						nn.opt->Step();
					}
					std::cout << " - loss: " << losses.mean() << std::endl;
					//std::cout << " - target: \n" << data << std::endl;
					//std::cout << " - predict: \n" << predData << std::endl;
				}
				plt->AddItem("pred", MOON_PlotType::Line, 2.0f, ImVec4(0.0f, 1.0f, 0, 1.0f), predLine);
			}

			static void classification() {
				int dataLen = 10, epoches = 50;
				Matrix data = Matrix::fill(Vector2(2, dataLen), 2);
				for (int i = 0; i < dataLen; i++) {
					if (i < dataLen / 2) {
						data[0][i] *= -1; data[1][i] *= -1;
					}
					data[0][i] += MoonMath::GaussianRand();
					data[1][i] += MoonMath::GaussianRand();
				}
				Matrix target = Matrix::fill(Vector2(2, dataLen), 1);
				for (int i = 0; i < dataLen / 2; i++) target[1][i] = 0;
				for (int i = dataLen / 2; i < dataLen; i++) target[0][i] = 0;

				std::cout << "random data:" << std::endl;
				std::cout << data << std::endl;
				std::cout << target << std::endl;

				FNN nn(NNM::globalNNGraph, 2, 2, {10}, relu, CE_SM, GD);
				Neuron* prob = new SoftMax(nn.predict, NNM::globalNNGraph);
				Matrix predData(1, dataLen, 0), rawPred(2, dataLen, 0);
				for (int i = 0; i < epoches; i++) {
					std::cout << "[---------- Epoch: " << i << " ----------]" << std::endl;
					Matrix losses(1, dataLen, 0);
					for (int x = 0; x < dataLen; x++) {
						//std::cout << " - batch: " << x << std::endl;
						nn.feature->SetVal(data.Sel_Col({ x }));
						//std::cout << " - feature: " << *input->value << std::endl;
						nn.label->SetVal(target.Sel_Col({ x }));
						//std::cout << " - label: \n" << *label->value << std::endl;

						prob->Forward();
						predData[0][x] = (*prob->value)[0][0] > (*prob->value)[1][0] ? 0 : 1;
						rawPred[0][x] = (*prob->value)[0][0]; rawPred[1][x] = (*prob->value)[1][0];

						nn.loss->Forward();
						losses[0][x] = (*nn.loss->value)[0][0];

						/*std::cout << " - size: " << std::endl;
						for (auto iter = graph->neurons.begin(); iter != graph->neurons.end(); iter++) {
							if (typeid(**iter) != typeid(NVariable)) {
								std::cout << " - node: " << (*iter)->id << std::endl;
								std::cout << (*iter)->Shape() << std::endl;
							}
						}*/

						nn.opt->Step();
					}
					/*std::cout << " - weight: " << std::endl;
					for (auto iter = graph->neurons.begin(); iter != graph->neurons.end(); iter++) {
						if (typeid(**iter) == typeid(NVariable)) {
							NVariable* node = dynamic_cast<NVariable*>(*iter);
							if (node->trainable) {
								std::cout << " - node: " << node->id << std::endl;
								std::cout << *node->value << std::endl;
							}
						}
					}*/

					std::cout << " - loss: " << losses.mean() << std::endl;
					std::cout << " - target: \n" << target << std::endl;
					std::cout << " - predict: \n" << predData << std::endl;
					std::cout << " - raw predict: \n" << rawPred << std::endl;
				}
			}

		};
	}
}
