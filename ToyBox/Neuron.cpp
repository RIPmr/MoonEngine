#include "Neuron.h"
#include "NNManager.h"

namespace MOON {
	namespace NN {
		Neuron::Neuron(NGraph* graph) {
			this->graph = graph;
			this->value = NULL;
			this->jacobi = NULL;
			this->id = graph->GenNeuronID();
			graph->AddNode(this);
		}

		Neuron::Neuron(Neuron* singleParent, NGraph* graph) {
			this->graph = graph;
			this->value = NULL;
			this->jacobi = NULL;
			this->id = graph->GenNeuronID();
			if (singleParent != NULL) {
				this->parent.push_back(singleParent);
				if (singleParent != this)
					singleParent->child.push_back(this);
			}
			graph->AddNode(this);
		}

		Neuron::Neuron(std::vector<Neuron*> &parent, NGraph* graph) {
			this->graph = graph;
			//this->parent = parent;
			this->value = NULL;
			this->jacobi = NULL;
			this->id = graph->GenNeuronID();
			for (auto iter = parent.begin(); iter != parent.end(); iter++) {
				this->parent.push_back(*iter);
				(*iter)->child.push_back(this);
			}
			graph->AddNode(this);
		}

		void Neuron::SetGraph(NGraph* g) {
			graph->RemoveNode(this);
			this->id = g->GenNeuronID();
			g->AddNode(this);
			// TODO : clear childs and parents
		}

		bool operator==(const Neuron &n1, const Neuron &n2) {
			/*if (n1.parent == n2.parent && n1.child == n2.child &&
				n1.value == n2.value && n1.jacobi == n2.jacobi &&
				n1.graph == n2.graph) return true;*/
			if (n1.graph == n2.graph && n1.id == n2.id) return true;
			else return false;
		}
	}
}