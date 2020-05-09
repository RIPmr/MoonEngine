#pragma once
#include <cmath>
#include <vector>
#include <string>
#include <cstdlib>
#include <iostream>
#include <algorithm>
#include <unordered_map>

#include "ButtonEx.h"
#include "NodeEditorBase.h"
#include "NVariable.h"
#include "NFunction.h"
#include "Utility.h"
#include "Matrix.h"
#include "Plotter.h"
#include "MathUtils.h"

namespace MOON {
	namespace NN {
		extern class NNBase;
		class NGraph : public NodeEditor {
		public:
			unsigned int id = 0;
			bool visible = true;
			std::string name;

			bool isPlay;
			int epoch;
			unsigned int currentStep;

			unsigned int neuronCnt;
			std::vector<Neuron*> neurons;
			std::vector<Optimizer*> optimizers;

			std::vector<ImNodes::MyNode*> inputs;

			NGraph();
			NGraph(const std::string &name);

			~NGraph() {
				if (neurons.size() > 0) {
					for (int i = 0; i < neurons.size(); i++) delete neurons[i];
					neurons.clear();
				}
				if (optimizers.size() > 0) {
					for (int i = 0; i < optimizers.size(); i++) delete optimizers[i];
					optimizers.clear();
				}
				std::cout << "NN Graph: " << name << "(id: " << std::to_string(id) << ") released." << std::endl;
			}

			void DefineNodes();

			// TODO
			void ClearGraph() {
				std::cout << "[NN] Graph: " << name << " cleared." << std::endl;
			}

			unsigned int GenNeuronID() {
				return neuronCnt++;
			}

			// *NOTE: pay attention when calling this method,
			// beacuse neurons will added to graph automatically while create them
			void AddNode(Neuron* node) {
				neurons.push_back(node);
			}
			void AddOpt(Optimizer* opt) {
				optimizers.push_back(opt);
			}

			void RemoveNode(Neuron* node) {
				RemoveElem<Neuron>(neurons, node);
			}
			void RemoveOpt(Optimizer* opt) {
				RemoveElem<Optimizer>(optimizers, opt);
			}

			Neuron* CreateActivation(const Activation &actType, Neuron* input) {
				if (actType == relu)
					return new ReLU(input, this);
				else if (actType == logistic)
					return new Logistic(input, this);
			}

			Neuron* CreateNeuron(const std::string &type) {
				Neuron* node = nullptr;
				if (type._Equal("Input")) {
					node = new NVariable(Vector2::ZERO(), this, false, false);
				} else if (type._Equal("Variable")) {
					node = new NVariable(Vector2::ONE(), this, true, true);
				} else if (type._Equal("Add")) {
					node = new NAdd(this);
				} else if (type._Equal("Pow")) {
					node = new NPow(this);
				} else if (type._Equal("Dot")) {
					node = new NDot(this);
				} else if (type._Equal("Multiply")) {
					node = new NMatMul(this);
				} else if (type._Equal("Logistic")) {
					node = new Logistic(this);
				} else if (type._Equal("ReLU")) {
					node = new ReLU(this);
				} else if (type._Equal("SoftMax")) {
					node = new SoftMax(this);
				} else if (type._Equal("CE")) {
					node = new CrossEntropy_SoftMax(this);
				} else if (type._Equal("MSE")) {
					node = new MSE(this);
				}
				//neurons.push_back(node);
				return node;
			}

			Neuron* CreateLossFunction(const LossFunction &lossType, const std::vector<Neuron*>& parent) {
				if (lossType == MSE_)
					return new MSE(parent, this);
				else if (lossType == CE_SM)
					return new CrossEntropy_SoftMax(parent, this);
			}

			Optimizer* CreateOptimizer(const OptimizerType &actType, Neuron* loss) {
				if (actType == GD)
					return new GradDesc(this, loss);
				else if (actType == RMSP)
					return new RMSProp(this, loss);
				else if (actType == ADAM)
					return new Adam(this, loss);
			}

			Optimizer* CreateOptimizer(const std::string &type) {
				Optimizer* node = nullptr;
				if (type._Equal("GradDesc")) {
					node = new GradDesc(this, NULL);
				} else if (type._Equal("RMSProp")) {
					node = new RMSProp(this, NULL);
				} else if (type._Equal("Adam")) {
					node = new Adam(this, NULL);
				}
				return node;
			}

			Plotter* CreatePlotter(const std::string &type) {
				Plotter* plt = new Plotter();
				return plt;
			}

			void ClearJacobian() {
				for (auto iter = neurons.begin(); iter != neurons.end(); iter++) {
					(*iter)->ClearJacobian();
				}
			}

			void ResetValue() {
				for (auto iter = neurons.begin(); iter != neurons.end(); iter++) {
					(*iter)->ResetVal(false);
				}
			}

			// TODO
			void CollectingTerminus() {
				std::cout << "CollectingTerminus" << std::endl;
			}

			void ResetNetwork() {
				std::cout << "ResetNetwork" << std::endl;
			}

			void Training_OneStep() {
				// set feature and label
				for (auto input : inputs) {
					input->output_slots[1].data.id[0]++;
					input->Backward();
					//std::cout << "set batch of " << input->title << " to " << 
						//std::to_string(input->output_slots[1].data.id[0]) << std::endl;
				}

				// forward calc and backward propagation
				for (auto opt : optimizers) {
					opt->Step();
				}
			}

			void Instantiate(NNBase* network) {

			}

			void PopMenu() override;

			void OnDrawGraph() override {
				ImVec2 start = ImGui::GetWindowPos();
				ImVec2 size = ImGui::GetWindowSize();
				ImVec2 btnSize(30.0f, 30.0f);
				ImVec2 gap(10.0f, 10.0f);

				ImGui::SetCursorPos(gap);
				ImGui::BeginGroup();
				SwitchButton(ICON_FA_PAUSE, ICON_FA_PLAY, isPlay, btnSize);
				ImGui::SameLine();
				if (ImGui::Button(ICON_FA_STEP_FORWARD, btnSize)) {
					if (currentStep < epoch) {
						isPlay = false;
						Training_OneStep();
						currentStep++;
					}
				}
				ImGui::SameLine();
				if (ImGui::Button(ICON_FA_STOP, btnSize)) {
					isPlay = false;
					currentStep = 0;
				}

				ImGui::AlignTextToFramePadding();
				ImGui::Text((std::to_string(currentStep) + " of").c_str()); 
				ImGui::SameLine();
				ImGui::SetNextItemWidth(ImGui::CalcTextSize(std::to_string(epoch).c_str()).x + 20);
				ImGui::DragInt("Epoch", &epoch, 1.0f, 1, INFINITY_INT);
				ImGui::EndGroup();

				// do training
				if (isPlay && currentStep < epoch) {
					Training_OneStep();
					currentStep++;
				}
			}

			void OnDeleteNode(ImNodes::MyNode* node) override {
				RemoveElem(inputs, node);
			}

			void OnCreateNode(ImNodes::MyNode* node, const std::string& type) override {
				if (type._Equal("Neuron")) {
					Neuron* neu = this->CreateNeuron(node->title);
					node->attachment = neu;
					if (node->title._Equal("Input")) inputs.push_back(node);
				} else if (type._Equal("Optimizer")) {
					Optimizer* opt = this->CreateOptimizer(node->title);
					node->attachment = opt;
				} else if (type._Equal("Visualization")) {
					if (node->title._Equal("BatchPlot")) {
						node->attachment = this;
					} else {
						Plotter* plt = new Plotter();
						node->attachment = plt;
					}
				} else {
					// fake node
					node->attachment = new NVariable(Vector2::ZERO(), this, false, false);
				}
			}

			void OnDeleteConnection(const ImNodes::MyConnection &con,
				ImNodes::MyNode* inNode, ImNodes::MyNode* outNode,
				ImNodes::SlotInfo* inSlot, ImNodes::SlotInfo* outSlot) override {
				if (outNode->attachment != nullptr) { // filter out data and operators
					if (inNode->output_slots.size() < 1 ||
						inNode->input_slots[0].title._Equal("prop")) {
						if (inNode->input_slots.size() == 1) { // input is optimizer
							Optimizer* opt = (Optimizer*)inNode->attachment;
							opt->target = nullptr;
						} else; // input is plotter
					} else { // input and output are all neurons
						Neuron* neuIn = (Neuron*)inNode->attachment;
						Neuron* neuOut = (Neuron*)outNode->attachment;
						neuIn->RemoveParent(neuOut);
					}
				}
			}

			void OnCreateConnection(const ImNodes::MyConnection &con,
				ImNodes::MyNode* inNode, ImNodes::MyNode* outNode,
				ImNodes::SlotInfo* inSlot, ImNodes::SlotInfo* outSlot) override {
				if (outNode->attachment != nullptr) { // filter out data and operators
					Neuron* neuOut = (Neuron*)outNode->attachment;
					if (inNode->output_slots.size() < 1 || 
						inNode->input_slots[0].title._Equal("prop")) {
						if (inNode->input_slots.size() == 1) { // input is optimizer
							Optimizer* opt = (Optimizer*)inNode->attachment;
							opt->target = neuOut;
						} else; // input is plotter
					} else { // input and output are all neurons
						Neuron* neuIn = (Neuron*)inNode->attachment;
						neuIn->AddParent(neuOut);
					}
				}
			}

		};

	}
}