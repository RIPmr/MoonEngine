#pragma once
#include <cmath>
#include <vector>
#include <cstdlib>
#include <iostream>
#include <algorithm>

#include "NVariable.h"
#include "NGraph.h"

namespace MOON {
	#define MOON_NeuralNetworkManager NN::NNManager
	#define NNM NNManager
	namespace NN {
		class NNManager {
		public:
			static unsigned int graphCnt;
			static NGraph* globalNNGraph;
			static NGraph* currentGraph;
			static std::vector<NGraph*> graphList;

			static void Init() {
				globalNNGraph = new NN::NGraph();
				currentGraph = NN::NNM::globalNNGraph;
				graphList.push_back(globalNNGraph);
			}

			static void Release() {
				for (auto& iter : graphList) delete iter;
				graphList.clear();
			}

			static unsigned int GenGraphID() {
				return graphCnt++;
			}

			static NGraph* NewGraph() {
				NGraph* newGraph = new NGraph();
				AddGraph(newGraph);
				return newGraph;
			}

			static NGraph* NewGraph(const std::string &name) {
				NGraph* newGraph = new NGraph(name);
				AddGraph(newGraph);
				return newGraph;
			}

			static void AddGraph(NGraph* g) {
				auto count = graphList.size();
				graphList.push_back(g);
				currentGraph = g;

				std::cout << "[NNM]: Graph " << g->name << (graphList.size() > count ? " was" : " was not") << " inserted." << std::endl;
			}

			// TODO
			static void RemoveGraph(const NGraph* g) {

			}
			static void RemoveGraph(const std::string &name) {

			}
			static void RemoveGraph(const int &id) {

			}

			// TODO
			static void PrintAllGraphs() {
				std::cout << "total graph: " << graphCnt << std::endl;
			}

			static void Draw();

		};
	}
}