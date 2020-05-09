#include "NFunction.h"
#include "NGraph.h"

namespace MOON {
	namespace NN {

		void GradDesc::Update() {
			for (auto iter = graph->neurons.begin(); iter != graph->neurons.end(); iter++) {
				if (typeid(**iter) == typeid(NVariable)) {
					NVariable* node = dynamic_cast<NVariable*>(*iter);
					if (node->trainable) {
						Matrix gradient = this->GetGradient(node);
						node->SetVal(*node->value - lr * gradient);
					}
				}
			}
		}

		void RMSProp::Update() {
			for (auto iter = graph->neurons.begin(); iter != graph->neurons.end(); iter++) {
				if (typeid(**iter) == typeid(NVariable)) {
					NVariable* node = dynamic_cast<NVariable*>(*iter);
					if (node->trainable) {
						Matrix gradient = this->GetGradient(node);
						if (s.find(node->id) == s.end()) {
							s[node->id] = gradient.Map([](float& i) -> void {i = i * i;});
						} else {
							s[node->id] = friction * s[node->id] + (1.0f - friction) *
								gradient.Map([](float& i) -> void {i = i * i;});
						}
						Matrix res;
						Matrix::multiElem(lr * gradient, s[node->id].Map([](float &i) ->
							void { i = 1.0 / std::sqrt(i + 1e-10); }), res);
						node->SetVal(*node->value - res);
					}
				}
			}
		}

		void Adam::Update() {
			for (auto iter = graph->neurons.begin(); iter != graph->neurons.end(); iter++) {
				if (typeid(**iter) == typeid(NVariable)) {
					NVariable* node = dynamic_cast<NVariable*>(*iter);
					if (node->trainable) {
						Matrix gradient = this->GetGradient(node);
						int id = node->id;
						if (s.find(id) == s.end()) {
							v[id] = gradient;
							s[id] = gradient.Map([](float &i) -> void { i = i * i; });
						} else {
							v[id] = friction.x * v[id] + (1.0 - friction.x) * gradient;
							s[id] = friction.y * s[id] + (1.0 - friction.y) *
								gradient.Map([](float &i) -> void { i = i * i; });
						}
						Matrix res;
						Matrix::multiElem(lr * v[id], s[id].Map([](float& i) ->
							void { i = 1.0 / std::sqrt(i + 1e-10); }), res);
						node->SetVal(*node->value - res);
					}
				}
			}
		}

	}
}