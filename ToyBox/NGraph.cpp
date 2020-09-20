#include "Utility.h"
#include "NGraph.h"
#include "NNManager.h"
#include "Plotter.h"
#include "Icons.h"

using namespace ImNodes;

namespace MOON {
	namespace NN {

		NGraph::NGraph() : id(NNM::GenGraphID()), neuronCnt(1),
			isPlay(false), epoch(60), currentStep(0) {
			this->name = "ngraph_" + std::to_string(id);
			DefineNodes();
		}

		NGraph::NGraph(const std::string &name) : id(NNM::GenGraphID()),
			neuronCnt(1), name(name), isPlay(false), epoch(60), currentStep(0) {
			DefineNodes();
		}

		bool ShowMatrix(Matrix &mat, const float &len = 1.0f, const std::string &id = "0", const float &adaptiveHeight = 0) {
			bool changed = false;
			ImGui::BeginChild(("scrolling_" + id).c_str(), 
							  ImVec2(0, adaptiveHeight), true, ImGuiWindowFlags_HorizontalScrollbar);
			for (int i = 0; i < mat.size.x; i++) {
				for (int j = 0; j < mat.size.y; j++) {
					if (ImGui::InputFloat(("data_" + std::to_string(i) + "_" +
						std::to_string(j)).c_str(), &mat[i][j], 0, 0, "%.3f", 0, true, len * 2.0f))
						changed = true;
					if (j < mat.size.y - 1) ImGui::SameLine();
				}
			}
			ImGui::EndChild();
			return changed;
		}

		bool IsInputSlotFull(const MyNode* node) {
			if (node->input_slots.size() > 0) {
				for (int i = node->input_slots.size() - 1; i >= 0; i--) {
					if (node->input_slots[i].kind <= SlotType::InnerData) break;
					else if (node->input_slots[i].con == nullptr) return false;
				}
				//for (auto &slot : node->input_slots) if (slot.kind > 3 && slot.con == nullptr) return false;
			}
			return true;
		}

		void SyncMatrix(MyNode* node) {
			Matrix* mat = ((Neuron*)node->attachment)->value;
			//if (mat != nullptr && outputMat(0) != *mat) {
			if (mat != nullptr) {
				outputMat(0) = *mat;
				//node->Backward();
			}
		}

		void SyncFakeNode(MyNode* node) {
			NVariable* fakeNode = (NVariable*)node->attachment;
			fakeNode->dimension = outputMat(0).size;
			fakeNode->SetVal(outputMat(0));
		}

		void SortParent(MyNode* node, Neuron* neu) {
			neu->parent.clear();
			for (int i = node->input_slots.size() - 1; i >= 0; i--) {
				if (node->input_slots[i].kind <= SlotType::InnerData) break;
				else neu->parent.insert(neu->parent.begin(), (Neuron*)parentNode(i)->attachment);
			}
			/*std::cout << "inserted:" << std::endl;
			for(auto &n : neu->parent) {
				std::cout << n->id << std::endl;
			}*/
		}

		void BlockMouseAction(MyNode* node) {
			assert (node->parent != nullptr);
			((NodeEditor*)node->parent)->blockMouseAction = true;
		}

		void NeuronRenderer(MyNode* node, bool hideInNode, void(*content)(MyNode*) = nullptr) {
			if (node == nullptr) return;

			if (!hideInNode) node->ListName();
			if (node->attachment == nullptr) {
				ImGui::Text("no neuron attached.");
				return;
			}

			Neuron* neu = (Neuron*)node->attachment;
			std::string tmp = "(" + std::to_string((int)neu->Shape().x) + u8"¡Á" + std::to_string((int)neu->Shape().y) + ")";

			if (!hideInNode) {
				if (neu->value != NULL) {
					if (*content != nullptr) (*content)(node);
					else ImGui::Text(("shape: " + tmp).c_str());
				}

				ImGui::DragFloat("field length", &inputVec(0, 0).z, 0.1f);
				if (inputVec(0, 0).z < 1) inputVec(0, 0).z = 1;
				ImGui::Separator();

				ImGui::Text(("id: " + std::to_string(neu->id)).c_str());
				ImGui::Text("value:");
				if (neu->value != NULL) ShowMatrix(*(Matrix*)(neu->value), inputVec(0, 0).z, "0", 300);
				else { ImGui::SameLine(); ImGui::Text("empty."); }

				ImGui::Separator();

				ImGui::Text("jacobi:");
				if (neu->jacobi != NULL) ShowMatrix(*(Matrix*)(neu->jacobi), inputVec(0, 0).z, "1", 300);
				else { ImGui::SameLine(); ImGui::Text("empty."); }
			} else {
				if (IsInputSlotFull(node)) ImGui::Text(tmp.c_str());
				else ImGui::TextUnformatted(ICON_FA_BAN);
			}
		}

		void OptiRenderer(MyNode* node, bool hideInNode, void(*content)(MyNode*) = nullptr, void(*preview)(MyNode*) = nullptr) {
			if (node == nullptr) return;

			if (!hideInNode) node->ListName();
			if (node->attachment == nullptr) {
				ImGui::Text("no optimizer attached.");
				return;
			}

			Optimizer* opt = (Optimizer*)node->attachment;
			if (!hideInNode) {
				ImGui::Text(("acc: " + std::to_string(opt->acc_cnt)).c_str());
				ImGui::DragInt("batch size", &opt->batchSize);
				ImGui::DragFloat("learning rate", &opt->lr, 0.01f);
				if (*content != nullptr) (*content)(node);
				if (inputVec(0, 0).z < 1) inputVec(0, 0).z = 1;
				ImGui::Separator();

				ImGui::Text(("id: " + std::to_string(opt->id)).c_str());
				ImGui::Text("gradient map:");
				if (opt->acc_gradient.size() > 0) {
					for (auto& iter : opt->acc_gradient) {
						ImGui::Text(("neuron(id): " + std::to_string(iter.first) + ", gradient:").c_str());
						ShowMatrix(iter.second, inputVec(0, 0).z, std::to_string(iter.first), iter.second.size.x * 40.0f);
					}
				} else { ImGui::SameLine(); ImGui::Text("empty."); }
			} else {
				if (IsInputSlotFull(node)) {
					ImGui::TextUnformatted(("BS:" + std::to_string(opt->batchSize)).c_str());
					ImGui::TextUnformatted(("Acc:" + std::to_string(opt->acc_cnt)).c_str());
					ImGui::TextUnformatted(("LR:" + Strutil::to_string_precision(opt->lr, 1, 3)).c_str());
					if (*preview != nullptr) (*preview)(node);
				} else ImGui::TextUnformatted(ICON_FA_BAN);
			}
		}

		std::vector<ImVec2> MatToVec(const Matrix& mat) {
			//assert(mat.size.x == 2);
			if (mat.size.x != 2) return { ImVec2(0, 0) };

			std::vector<ImVec2> res;
			for (int i = 0; i < mat.size.y; i++) 
				res.push_back(ImVec2(mat[0][i], mat[1][i]));
			return res;
		}

		ImVec4 VecToCol(const Vector3& vec) {
			return ImVec4(vec.x, vec.y, vec.z, 1.0f);
		}

		void NGraph::DefineNodes() {
			available_nodes = new Nodes{
				// Data Nodes -----------------------------------------------------------------------
				#pragma region Data_Nodes
				{ "Data", []() -> MyNode* { return new MyNode("Random",
					{	/// internal data
						{ "range",  InnerData,   DataSizeDef(0, 1, 0), true },
						{ "shape",  InnerData,   DataSizeDef(0, 1, 0), true },
					}, {/// Output slots
						{ "out",	Slot_Matrix, DataSizeDef(0, 0, 1) }
					},	/// content renderer
					[](MyNode* node, bool hideInNode) {
						if (!hideInNode) {
							node->ListName();
							ImGui::DragFloat2("range", (float*)&inputVec(0, 0));
							if (ImGui::DragFloat2("shape", (float*)&inputVec(1, 0)))  node->Backward();
							if (ImGui::DragFloat("field length", &inputVec(1, 0).z, 0.1f))  node->Backward();
							if (ImGui::Button("regenerate", ImVec2(ImGui::GetContentRegionAvailWidth(), 30.0f))) {
								outputMat(0) = Matrix::Random(
									inputVec(0, 0).x, 
									inputVec(0, 0).y, 
									inputVec(1, 0)
								);
								node->Backward();
							}
							ImGui::Separator(); ImGui::Text("data:");
							if (ShowMatrix(outputMat(0), inputVec(1, 0).z)) node->Backward();
						} else {
							std::string tmp = "[" + Strutil::to_string_precision(inputVec(0, 0).x, 1, 3) + ", " + Strutil::to_string_precision(inputVec(0, 0).y, 1, 3) + "]";
							ImGui::TextUnformatted(tmp.c_str());
							tmp = "(" + std::to_string((int)inputVec(1, 0).x) + u8"¡Á" + std::to_string((int)inputVec(1, 0).y) + ")";
							ImGui::TextUnformatted(tmp.c_str());
						}
					},	/// process data
					[](MyNode* node) {
						if (inputVec(1, 0).z < 1) inputVec(1, 0).z = 1;
						if (inputVec(1, 0).y < 1) inputVec(1, 0).y = 1;
						if (inputVec(1, 0).x < 1) inputVec(1, 0).x = 1;

						SyncFakeNode(node);
					}
				);}},

				{ "Data", []() -> MyNode* { return new MyNode("Uniform", 
					{	/// internal data
						{ "param",  InnerData,   DataSizeDef(0, 1, 0), true },
						{ "shape",  InnerData,   DataSizeDef(0, 1, 0), true },
					}, {/// Output slots
						{ "out",	Slot_Matrix, DataSizeDef(0, 0, 1) }
					},	/// content renderer
					[](MyNode* node, bool hideInNode) {
						if (!hideInNode) {
							node->ListName();
							ImGui::DragFloat("location(E)", &inputVec(0, 0).x);
							ImGui::DragFloat("scale(V)", &inputVec(0, 0).y);
							if (ImGui::DragFloat2("shape", (float*)&inputVec(1, 0)))  node->Backward();
							if (ImGui::DragFloat("field length", &inputVec(1, 0).z, 0.1f))  node->Backward();
							if (ImGui::Button("regenerate", ImVec2(ImGui::GetContentRegionAvailWidth(), 30.0f))) {
								outputMat(0) = Matrix::Random_Normal(
									inputVec(0, 0).x,
									inputVec(0, 0).y,
									inputVec(1, 0)
								);
								node->Backward();
							}
							ImGui::Separator();
							ImGui::Text("data:");
							if (ShowMatrix(outputMat(0), inputVec(1, 0).z)) node->Backward();
						} else {
							std::string tmp = "E: " + Strutil::to_string_precision(inputVec(0, 0).x, 1, 3);
							ImGui::TextUnformatted(tmp.c_str());
							tmp = "V: " + Strutil::to_string_precision(inputVec(0, 0).y, 1, 3);
							ImGui::TextUnformatted(tmp.c_str());
							tmp = "(" + std::to_string((int)inputVec(1, 0).x) + u8"¡Á" + std::to_string((int)inputVec(1, 0).y) + ")";
							ImGui::TextUnformatted(tmp.c_str());
						}
					},	/// process data
					[](MyNode* node) {
						if (inputVec(1, 0).z < 1) inputVec(1, 0).z = 1;
						if (inputVec(1, 0).y < 1) inputVec(1, 0).y = 1;
						if (inputVec(1, 0).x < 1) inputVec(1, 0).x = 1;

						SyncFakeNode(node);
					}
				);}},

				{ "Data", []() -> MyNode* { return new MyNode("Fill", 
					{	/// internal data
						{ "value",  InnerData,   DataSizeDef(0, 1, 0), true },
						{ "shape",  InnerData,   DataSizeDef(0, 1, 0), true },
					}, {/// Output slots
						{ "out",	Slot_Matrix, DataSizeDef(0, 0, 1) }
					},	/// content renderer
					[](MyNode* node, bool hideInNode) {
						if (!hideInNode) {
							node->ListName();
							ImGui::DragFloat("value", &inputVec(0, 0).x);
							if (ImGui::DragFloat2("shape", (float*)&inputVec(1, 0)))  node->Backward();
							if (ImGui::DragFloat("field length", &inputVec(1, 0).z, 0.1f))  node->Backward();
							if (ImGui::Button("regenerate", ImVec2(ImGui::GetContentRegionAvailWidth(), 30.0f))) {
								outputMat(0) = Matrix::fill(
									inputVec(1, 0),
									inputVec(0, 0).x
								);
								node->Backward();
							}
							ImGui::Separator(); ImGui::Text("data:");
							if (ShowMatrix(outputMat(0), inputVec(1, 0).z)) node->Backward();
						} else {
							std::string tmp = "V: " + Strutil::to_string_precision(inputVec(0, 0).x, 1, 3);
							ImGui::TextUnformatted(tmp.c_str());
							tmp = "(" + std::to_string((int)inputVec(1, 0).x) + u8"¡Á" + std::to_string((int)inputVec(1, 0).y) + ")";
							ImGui::TextUnformatted(tmp.c_str());
						}
					},	/// process data
					[](MyNode* node) {
						if (inputVec(1, 0).z < 1) inputVec(1, 0).z = 1;
						if (inputVec(1, 0).y < 1) inputVec(1, 0).y = 1;
						if (inputVec(1, 0).x < 1) inputVec(1, 0).x = 1;

						SyncFakeNode(node);
					}
				);}},

				{ "Data", []() -> MyNode* { return new MyNode("Eye", 
					{	/// internal data
						{ "value",  InnerData,   DataSizeDef(0, 1, 0), true },
						{ "shape",  InnerData,   DataSizeDef(0, 1, 0), true },
					}, {/// Output slots
						{ "out",	Slot_Matrix, DataSizeDef(0, 0, 1) }
					},	/// content renderer
					[](MyNode* node, bool hideInNode) {
						if (!hideInNode) {
							node->ListName();
							ImGui::DragFloat("diag", &inputVec(0, 0).x);
							if (ImGui::DragFloat2("shape", (float*)&inputVec(1, 0))) node->Backward();
							if (ImGui::DragFloat("field length", &inputVec(1, 0).z, 0.1f)) node->Backward();
							if (ImGui::Button("regenerate", ImVec2(ImGui::GetContentRegionAvailWidth(), 30.0f))) {
								outputMat(0).NewMat(
									inputVec(1, 0),
									inputVec(0, 0).x
								);
								node->Backward();
							}
							ImGui::Separator(); ImGui::Text("data:");
							if (ShowMatrix(outputMat(0), inputVec(1, 0).z)) node->Backward();
						} else {
							std::string tmp = "D: " + Strutil::to_string_precision(inputVec(0, 0).x, 1, 3);
							ImGui::TextUnformatted(tmp.c_str());
							tmp = "(" + std::to_string((int)inputVec(1, 0).x) + u8"¡Á" + std::to_string((int)inputVec(1, 0).y) + ")";
							ImGui::TextUnformatted(tmp.c_str());
						}
					},	/// process data
					[](MyNode* node) {
						if (inputVec(1, 0).z < 1) inputVec(1, 0).z = 1;
						if (inputVec(1, 0).y < 1) inputVec(1, 0).y = 1;
						if (inputVec(1, 0).x < 1) inputVec(1, 0).x = 1;

						SyncFakeNode(node);
					}
				);}},
					
				{ "Data", []() -> MyNode* { return new MyNode("Arange", 
					{	/// internal data
						{ "range",  InnerData,   DataSizeDef(0, 1, 0), true },
						{ "shape",  InnerData,   DataSizeDef(0, 1, 0), true },
					}, {/// Output slots
						{ "out",	Slot_Matrix, DataSizeDef(0, 0, 1) }
					},	/// content renderer
					[](MyNode* node, bool hideInNode) {
						std::string tmp = "(" + std::to_string((int)inputVec(1, 0).x) + u8"¡Á" + 
										std::to_string((int)inputVec(1, 0).y) + ")";
						if (!hideInNode) {
							node->ListName();
							ImGui::Text(("shape: " + tmp).c_str());
							ImGui::DragFloat3("range/step", (float*)&inputVec(0, 0));
							if (ImGui::DragFloat("field length", &inputVec(1, 0).z, 0.1f)) node->Backward();
							if (ImGui::Button("regenerate", ImVec2(ImGui::GetContentRegionAvailWidth(), 30.0f))) {
								outputMat(0) = Matrix::Arange(inputVec(0, 0).x, inputVec(0, 0).y, inputVec(0, 0).z);
								node->Backward();
							}
							ImGui::Separator(); ImGui::Text("data:");
							if (ShowMatrix(outputMat(0), inputVec(1, 0).z)) node->Backward();
						} else {
							ImGui::TextUnformatted(tmp.c_str());
							tmp = "R: (" + Strutil::to_string_precision(inputVec(0, 0).x, 1, 3) + "," +
								Strutil::to_string_precision(inputVec(0, 0).y, 1, 3) + "," +
								Strutil::to_string_precision(inputVec(0, 0).z, 1, 3) + ")";
							ImGui::TextUnformatted(tmp.c_str());
						}
					},	/// process data
					[](MyNode* node) {
						if (inputVec(1, 0).z < 1) inputVec(1, 0).z = 1;
						if (inputVec(1, 0).y < 1) inputVec(1, 0).y = 1;

						inputVec(1, 0).x = outputMat(0).size.x;
						inputVec(1, 0).y = outputMat(0).size.y;

						SyncFakeNode(node);
					}
				);}},
				#pragma endregion
				// Operators ------------------------------------------------------------------------
				#pragma region Operators
				{ "Operator", []() -> MyNode* { return new MyNode("Reshape", 
					{	/// internal data
						{ "shape",  InnerData,		DataSizeDef(0, 1, 0), true },
						/// Input slots
						{ "in",		Slot_Matrix,	DataSizeDef(0, 0, 1) },
					}, {/// Output slots
						{ "out",	Slot_Matrix,	DataSizeDef(0, 0, 1) }
					},	/// content renderer
					[](MyNode* node, bool hideInNode) {
						if (!hideInNode) {
							node->ListName();
							if (ImGui::DragFloat2("shape", (float*)&inputVec(0, 0))) node->Backward();
							if (ImGui::DragFloat("field length", &inputVec(0, 0).z, 0.1f)) node->Backward();
							ImGui::Separator();
							if (parentSlot(1) != NULL) {
								if (inputMat(1).dimension() == inputVec(0, 0).x * inputVec(0, 0).y) {
									ImGui::Text("reshaped data:");
									if (ShowMatrix(outputMat(0), inputVec(0, 0).z)) node->Backward();
								} else {
									ImGui::Text("can't reshape input to specified shape!");
									ImGui::Text(("unsupported map: R(" + std::to_string((int)inputMat(1).size.x) + ", " + std::to_string((int)inputMat(1).size.y)
										+ ") -> R(" + std::to_string((int)inputVec(0, 0).x) + ", " + std::to_string((int)inputVec(0, 0).y) + ")").c_str());
								}
							} else {
								ImGui::Text("no input.");
							}
						} else {
							if (parentSlot(1) != NULL) {
								std::string tmp = "(" + std::to_string((int)inputVec(0, 0).x) + u8"¡Á" + std::to_string((int)inputVec(0, 0).y) + ")";
								ImGui::TextUnformatted(tmp.c_str());
							} else ImGui::TextUnformatted(ICON_FA_BAN);
						}
					},	/// process data
					[](MyNode* node) {
						// constraints
						if (inputVec(0, 0).z < 1) inputVec(0, 0).z = 1;
						if (inputVec(0, 0).y < 1) inputVec(0, 0).y = 1;
						if (inputVec(0, 0).x < 1) inputVec(0, 0).x = 1;

						// process content
						if (parentSlot(1) != NULL) {
							//if (inputMat(1).dimension() == inputVec(0, 0).x * inputVec(0, 0).y)
							outputMat(0) = inputMat(1).Reshape(inputVec(0, 0));
						}

						SyncFakeNode(node);
					}
				);}},

				{ "Operator", []() -> MyNode* { return new MyNode("Diagonal", 
					{	/// Input slots
						{ "in",		Slot_Matrix,	DataSizeDef(0, 1, 1) }
					}, {/// Output slots
						{ "out",	Slot_Matrix,	DataSizeDef(0, 0, 1) }
					},	/// content renderer
					[](MyNode* node, bool hideInNode) {
						std::string tmp = "(" + std::to_string((int)inputVec(0, 0).x) + u8"¡Á" + std::to_string((int)inputVec(0, 0).y) + ")";
						if (!hideInNode) {
							node->ListName();
							ImGui::Text(("shape: " + tmp).c_str());
							if (ImGui::DragFloat("field length", &inputVec(0, 0).z, 0.1f)) node->Backward();
							ImGui::Separator();
							if (parentSlot(0) != NULL) {
								ImGui::Text("data:");
								if (ShowMatrix(outputMat(0), inputVec(0, 0).z)) node->Backward();
							} else ImGui::Text("no input.");
						} else {
							if (parentSlot(0) != NULL) ImGui::TextUnformatted(tmp.c_str());
							else ImGui::TextUnformatted(ICON_FA_BAN);
						}
					},	/// process data
					[](MyNode* node) {
						// constraints
						if (inputVec(0, 0).z < 1) inputVec(0, 0).z = 1;

						// process content
						if (parentSlot(0) != nullptr) {
							outputMat(0) = Matrix::Diag(inputMat(0));
							inputVec(0, 0).x = outputMat(0).size.x;
							inputVec(0, 0).y = outputMat(0).size.y;
						} else {
							inputVec(0, 0).x = 0;
							inputVec(0, 0).y = 0;
						}

						SyncFakeNode(node);
					}
				);}},

				{ "Operator", []() -> MyNode* { return new MyNode("Transpose", 
					{	/// Input slots
						{ "in",		Slot_Matrix,	DataSizeDef(0, 1, 1) }
					}, {/// Output slots
						{ "out",	Slot_Matrix,	DataSizeDef(0, 0, 1) }
					},	/// content renderer
					[](MyNode* node, bool hideInNode) {
						std::string tmp = "(" + std::to_string((int)inputVec(0, 0).x) + u8"¡Á" + std::to_string((int)inputVec(0, 0).y) + ")";
						if (!hideInNode) {
							node->ListName();
							ImGui::Text(("shape: " + tmp).c_str());
							if (ImGui::DragFloat("field length", &inputVec(0, 0).z, 0.1f)) node->Backward();
							ImGui::Separator();
							if (parentSlot(0) != NULL) {
								ImGui::Text("data:");
								if (ShowMatrix(outputMat(0), inputVec(0, 0).z)) node->Backward();
							} else ImGui::Text("no input.");
						} else {
							if (parentSlot(0) != NULL) ImGui::TextUnformatted(tmp.c_str());
							else ImGui::TextUnformatted(ICON_FA_BAN);
						}
					},	/// process data
					[](MyNode* node) {
						// constraints
						if (inputVec(0, 0).z < 1) inputVec(0, 0).z = 1;

						// process content
						if (parentSlot(0) != nullptr) {
							outputMat(0) = inputMat(0).transposed();
							inputVec(0, 0).x = outputMat(0).size.x;
							inputVec(0, 0).y = outputMat(0).size.y;
						} else {
							inputVec(0, 0).x = 0;
							inputVec(0, 0).y = 0;
						}

						SyncFakeNode(node);
					}
				);}},

				{ "Operator", []() -> MyNode* { return new MyNode("Select", 
					{	/// Input slots
						{ "in",		Slot_Matrix,	DataSizeDef(0, 1, 1) },
						{ "row",	Slot_Matrix,	DataSizeDef(0, 0, 1) },
						{ "col",	Slot_Matrix,	DataSizeDef(0, 0, 1) }
					}, {/// Output slots
						{ "out",	Slot_Matrix,	DataSizeDef(0, 0, 1) }
					},	/// content renderer
					[](MyNode* node, bool hideInNode) {
						std::string tmp = "(" + std::to_string((int)inputVec(0, 0).x) + u8"¡Á" + std::to_string((int)inputVec(0, 0).y) + ")";
						if (!hideInNode) {
							node->ListName();
							ImGui::Text(("shape: " + tmp).c_str());
							if (ImGui::DragFloat("field length", &inputVec(0, 0).z, 0.1f)) node->Backward();
							ImGui::Separator();
							if (parentSlot(0) != nullptr) {
								ImGui::Text("data:");
								if (ShowMatrix(outputMat(0), inputVec(0, 0).z)) node->Backward();
							} else ImGui::Text("no input.");
						} else {
							if (parentSlot(0) != nullptr) ImGui::TextUnformatted(tmp.c_str());
							else ImGui::TextUnformatted(ICON_FA_BAN);
						}
					},	/// process data
					[](MyNode* node) {
						// constraints
						if (inputVec(0, 0).z < 1) inputVec(0, 0).z = 1;

						// process content
						if (parentSlot(0) != nullptr) {
							if (parentSlot(1) != nullptr && parentSlot(2) != nullptr) {
								outputMat(0) = inputMat(0).Select(inputMat(1), inputMat(2));
							} else if (parentSlot(1) != nullptr) {
								outputMat(0) = inputMat(0).Sel_Row(inputMat(1));
							} else if (parentSlot(2) != nullptr) {
								outputMat(0) = inputMat(0).Sel_Col(inputMat(2));
							} else outputMat(0) = inputMat(0);
							inputVec(0, 0).x = outputMat(0).size.x;
							inputVec(0, 0).y = outputMat(0).size.y;
						} else {
							inputVec(0, 0).x = 0;
							inputVec(0, 0).y = 0;
						}

						SyncFakeNode(node);
					}
				);}},

				{ "Operator", []() -> MyNode* { return new MyNode("Composite", 
					{	/// Input slots
						{ "A",		Slot_Matrix,	DataSizeDef(1, 1, 1) },
						{ "B",		Slot_Matrix,	DataSizeDef(0, 0, 1) }
					}, {/// Output slots
						{ "out",	Slot_Matrix,	DataSizeDef(0, 0, 1) }
					},	/// content renderer
					[](MyNode* node, bool hideInNode) {
						std::string tmp = "(" + std::to_string((int)inputVec(0, 0).x) + u8"¡Á" + std::to_string((int)inputVec(0, 0).y) + ")";
						if (!hideInNode) {
							node->ListName();
							const char* items[] = { "Composite Row", "Composite Col" };
							if (ImGui::Combo("type", &inputID(0, 0), items, IM_ARRAYSIZE(items))) node->Backward();
							if (ImGui::DragFloat("field length", &inputVec(0, 0).z, 0.1f)) node->Backward();
							ImGui::Separator();
							if (IsInputSlotFull(node)) {
								ImGui::Text(("shape: " + tmp).c_str());
								ImGui::Text("data:");
								if (ShowMatrix(outputMat(0), inputVec(0, 0).z)) node->Backward();
							} else ImGui::Text("no enough input.");
						} else {
							if (IsInputSlotFull(node)) ImGui::TextUnformatted(tmp.c_str());
							else ImGui::TextUnformatted(ICON_FA_BAN);
						}
					},	/// process data
					[](MyNode* node) {
						// constraints
						if (inputVec(0, 0).z < 1) inputVec(0, 0).z = 1;

						// process content
						if (IsInputSlotFull(node)) {
							if (inputID(0, 0)) Matrix::AppendCol(inputMat(0), inputMat(1), outputMat(0));
							else Matrix::AppendRow(inputMat(0), inputMat(1), outputMat(0));
							inputVec(0, 0).x = outputMat(0).size.x;
							inputVec(0, 0).y = outputMat(0).size.y;
						} else {
							inputVec(0, 0).x = 0;
							inputVec(0, 0).y = 0;
						}

						SyncFakeNode(node);
					}
				);}},
				#pragma endregion
				// Neurons --------------------------------------------------------------------------
				#pragma region Neurons
				{ "Neuron", []() -> MyNode* { return new MyNode("Input",
					{	/// Input slots
						{ "in",			Slot_Matrix,	DataSizeDef(0, 1, 1) }
					}, {/// Output slots
						{ "out",		Slot_Matrix,	DataSizeDef(0, 0, 1) },
						{ "batchProp",	InnerData,		DataSizeDef(1, 1, 0), true},
					},	
					[](MyNode* node, bool hideInNode) {
						NeuronRenderer(node, hideInNode, [](MyNode* node) -> void {
							if (ImGui::DragFloat2("shape", (float*)&inputVec(0, 0))) {
								outputID(1, 0) = 1; node->Backward();
							}
							if (ImGui::DragInt("batch", &outputID(1, 0), 1.0f)) node->Backward();
						});
						std::string tmp = "batch " + std::to_string(outputID(1, 0)) + "/" +
							std::to_string((int)(outputVec(1, 0).x * outputVec(1, 0).y));
						if (!hideInNode) {
							ImGui::Separator();
							if (parentSlot(0) != nullptr) {
								ImGui::Text((tmp + ":").c_str());
								for (int i = 0; i < outputVec(1, 0).x; i++) {
									Matrix rowMat = inputMat(0).Sel_Row(Matrix::Arange(i * inputVec(0, 0).x, (i + 1) * inputVec(0, 0).x));
									for (int j = 0; j < outputVec(1, 0).y; j++) {
										Matrix miniBatch = rowMat.Sel_Col(Matrix::Arange(j * inputVec(0, 0).y, (j + 1) * inputVec(0, 0).y));
										if (ShowMatrix(miniBatch, inputVec(0, 0).z, std::to_string(i) + "_" + std::to_string(j), inputVec(0, 0).x * 40)) node->Backward();
									}
								}
							} else ImGui::Text("0 batch.");
						} else if (parentSlot(0) != nullptr) ImGui::Text(tmp.c_str());
					},
					[](MyNode* node) {
						NVariable* neu = (NVariable*)node->attachment;

						if (inputVec(0, 0).x < 1) inputVec(0, 0).x = 1;
						if (inputVec(0, 0).y < 1) inputVec(0, 0).y = 1;

						if (parentSlot(0) != nullptr) {
							if (inputVec(0, 0).x > inputMat(0).size.x) inputVec(0, 0).x = inputMat(0).size.x;
							if (inputVec(0, 0).y > inputMat(0).size.y) inputVec(0, 0).y = inputMat(0).size.y;
							if (outputID(1, 0) < 1) outputID(1, 0) = 1;

							outputVec(1, 0).setValue((int)(inputMat(0).size.x / inputVec(0, 0).x),
								(int)(inputMat(0).size.y / inputVec(0, 0).y), 0);

							if (outputID(1, 0) > outputVec(1, 0).x * outputVec(1, 0).y) outputID(1, 0) = 1;

							int row = outputID(1, 0) / outputVec(1, 0).y;
							int col = outputID(1, 0) - row * outputVec(1, 0).y;
							if (col == 0) col = outputVec(1, 0).y; else row++;
							Matrix rowMat = inputMat(0).Sel_Row(Matrix::Arange((row - 1) * inputVec(0, 0).x, row * inputVec(0, 0).x));
							outputMat(0) = rowMat.Sel_Col(Matrix::Arange((col - 1) * inputVec(0, 0).y, col * inputVec(0, 0).y));
							neu->dimension = outputMat(0).size;
							neu->SetVal(outputMat(0));
						}
					}
				);} },

				{ "Neuron", []() -> MyNode* { return new MyNode("Variable",
					{	/// Inner data
						{ "shape",			InnerData,	DataSizeDef(0, 1, 0), true},
						{ "uniform_param",	InnerData,	DataSizeDef(0, 1, 0), true}
					}, {/// Output slots
						{ "out",		Slot_Matrix,	DataSizeDef(0, 0, 1) }
					},
					[](MyNode* node, bool hideInNode) {
						NeuronRenderer(node, hideInNode, [](MyNode* node) -> void {
							NVariable* neu = (NVariable*)node->attachment;
							if (ImGui::DragFloat2("shape", (float*)&inputVec(0, 0)) | 
								ImGui::DragFloat("location(E)", &inputVec(1, 0).x) | 
								ImGui::DragFloat("scale(V)", &inputVec(1, 0).y) | 
								ImGui::Button("re-initiate", ImVec2(ImGui::GetContentRegionAvailWidth(), 30.0f))) {
								neu->ReInit(inputVec(0, 0), inputVec(1, 0).x, inputVec(1, 0).y);
								node->Backward();
							}
						});
						Matrix* mat = ((Neuron*)node->attachment)->value;
						if (mat != nullptr && outputMat(0) != *mat) {
							outputMat(0) = *mat;
							node->Backward();
						}
					},
					[](MyNode* node) {
						if (inputVec(0, 0).x < 1) inputVec(0, 0).x = 1;
						if (inputVec(0, 0).y < 1) inputVec(0, 0).y = 1;
						SyncMatrix(node);
					}
				);} },

				{ "Neuron", []() -> MyNode* { return new MyNode("Add",
					{	/// Input slots
						{ "A",		Slot_Matrix,	DataSizeDef(0, 1, 1) },
						{ "B",		Slot_Matrix,	DataSizeDef(0, 0, 1) }
					}, {/// Output slots
						{ "out",	Slot_Matrix,	DataSizeDef(0, 0, 1) }
					},
					[](MyNode* node, bool hideInNode) {
						NeuronRenderer(node, hideInNode);
					},
					[](MyNode* node) {
						Neuron* neu = (Neuron*)node->attachment;
						if (IsInputSlotFull(node)) neu->Forward();
						SyncMatrix(node);
					}
				);} },

				{ "Neuron", []() -> MyNode* { return new MyNode("Pow",
					{	/// Input slots
						{ "A",		Slot_Matrix,	DataSizeDef(0, 1, 1) }
					}, {/// Output slots
						{ "out",	Slot_Matrix,	DataSizeDef(0, 0, 1) }
					},
					[](MyNode* node, bool hideInNode) {
						NeuronRenderer(node, hideInNode, [](MyNode* node) -> void {
							if (ImGui::DragFloat("pow", &((NPow*)node->attachment)->n, 1.0f)) node->Backward();
						});
						if (hideInNode) 
							ImGui::TextUnformatted(( "n: " + Strutil::to_string_precision(((NPow*)node->attachment)->n, 1, 3) ).c_str());
					},
					[](MyNode* node) {
						Neuron* neu = (Neuron*)node->attachment;
						if (IsInputSlotFull(node)) neu->Forward();
						SyncMatrix(node);
					}
				);} },

				{ "Neuron", []() -> MyNode* { return new MyNode("Dot",
					{	/// Input slots
						{ "A",		Slot_Matrix,	DataSizeDef(0, 1, 1) },
						{ "B",		Slot_Matrix,	DataSizeDef(0, 0, 1) }
					}, {/// Output slots
						{ "out",	Slot_Matrix,	DataSizeDef(0, 0, 1) }
					},
					[](MyNode* node, bool hideInNode) {
						NeuronRenderer(node, hideInNode);
					},
					[](MyNode* node) {
						Neuron* neu = (Neuron*)node->attachment;
						if (IsInputSlotFull(node)) {
							// check parent order:
							// A : parent[0]
							// B : parent[1]
							SortParent(node, neu);
							neu->Forward();
						}
						SyncMatrix(node);
					}
				);} },

				{ "Neuron", []() -> MyNode* { return new MyNode("Multiply",
					{	/// Input slots
						{ "A",		Slot_Matrix,	DataSizeDef(0, 1, 1) },
						{ "B",		Slot_Matrix,	DataSizeDef(0, 0, 1) }
					}, {/// Output slots
						{ "out",	Slot_Matrix,	DataSizeDef(0, 0, 1) }
					},
					[](MyNode* node, bool hideInNode) {
						NeuronRenderer(node, hideInNode);
					},
					[](MyNode* node) {
						Neuron* neu = (Neuron*)node->attachment;
						if (IsInputSlotFull(node)) {
							SortParent(node, neu);
							neu->Forward();
						}
						SyncMatrix(node);
					}
				);} },

				{ "Neuron", []() -> MyNode* { return new MyNode("Logistic",
					{	/// Input slots
						{ "in",		Slot_Matrix,	DataSizeDef(0, 1, 1) }
					}, {/// Output slots
						{ "out",	Slot_Matrix,	DataSizeDef(0, 0, 1) }
					},
					[](MyNode* node, bool hideInNode) {
						NeuronRenderer(node, hideInNode);
					},
					[](MyNode* node) {
						Neuron* neu = (Neuron*)node->attachment;
						if (IsInputSlotFull(node)) neu->Forward();
						SyncMatrix(node);
					}
				);} },

				{ "Neuron", []() -> MyNode* { return new MyNode("ReLU",
					{	/// Input slots
						{ "in",		Slot_Matrix,	DataSizeDef(0, 1, 1) }
					}, {/// Output slots
						{ "out",	Slot_Matrix,	DataSizeDef(0, 0, 1) }
					},
					[](MyNode* node, bool hideInNode) {
						NeuronRenderer(node, hideInNode);
					},
					[](MyNode* node) {
						Neuron* neu = (Neuron*)node->attachment;
						if (IsInputSlotFull(node)) neu->Forward();
						SyncMatrix(node);
					}
				);} },

				{ "Neuron", []() -> MyNode* { return new MyNode("SoftMax",
					{	/// Input slots
						{ "in",		Slot_Matrix,	DataSizeDef(0, 1, 1) }
					}, {/// Output slots
						{ "out",	Slot_Matrix,	DataSizeDef(0, 0, 1) }
					},
					[](MyNode* node, bool hideInNode) {
						NeuronRenderer(node, hideInNode);
					},
					[](MyNode* node) {
						Neuron* neu = (Neuron*)node->attachment;
						if (IsInputSlotFull(node)) neu->Forward();
						SyncMatrix(node);
					}
				);} },

				{ "Neuron", []() -> MyNode* { return new MyNode("CE",
					{	/// Input slots
						{ "pred",	Slot_Matrix,	DataSizeDef(0, 1, 1) },
						{ "label",	Slot_Matrix,	DataSizeDef(0, 0, 1) }
					}, {/// Output slots
						{ "out",	Slot_Matrix,	DataSizeDef(0, 0, 1) }
					},
					[](MyNode* node, bool hideInNode) {
						NeuronRenderer(node, hideInNode);
					},
					[](MyNode* node) {
						Neuron* neu = (Neuron*)node->attachment;
						if (IsInputSlotFull(node)) neu->Forward();
						SyncMatrix(node);
					}
				);} },

				{ "Neuron", []() -> MyNode* { return new MyNode("MSE",
					{	/// Input slots
						{ "pred",	Slot_Matrix,	DataSizeDef(0, 1, 1) },
						{ "label",	Slot_Matrix,	DataSizeDef(0, 0, 1) }
					}, {/// Output slots
						{ "out",	Slot_Matrix,	DataSizeDef(0, 0, 1) }
					},
					[](MyNode* node, bool hideInNode) {
						NeuronRenderer(node, hideInNode);
					},
					[](MyNode* node) {
						Neuron* neu = (Neuron*)node->attachment;
						if (IsInputSlotFull(node)) neu->Forward();
						SyncMatrix(node);
					}
				);} },
				#pragma endregion
				// Optimizer ------------------------------------------------------------------------
				#pragma region Optimizer
				{ "Optimizer", []() -> MyNode* { return new MyNode("GradDesc",
					{	/// Input slots
						{ "target",		Slot_Matrix,	DataSizeDef(0, 1, 1) }
					}, {},
					[](MyNode* node, bool hideInNode) {
						OptiRenderer(node, hideInNode);
					},
					[](MyNode* node) {
						Optimizer* opt = (Optimizer*)node->attachment;
						if (IsInputSlotFull(node)) opt->Forward();
					}
				);} },

				{ "Optimizer", []() -> MyNode* { return new MyNode("RMSProp",
					{	/// Input slots
						{ "target",		Slot_Matrix,	DataSizeDef(0, 1, 1) }
					}, {},
					[](MyNode* node, bool hideInNode) {
						OptiRenderer(node, hideInNode, [](MyNode* node)-> void {
							RMSProp* opt = (RMSProp*)node->attachment;
							ImGui::DragFloat("S", &opt->friction, 0.01f);
						}, [](MyNode* node)-> void {
							RMSProp* opt = (RMSProp*)node->attachment;
							ImGui::TextUnformatted(("S: " + Strutil::to_string_precision(opt->friction, 1, 3)).c_str());
						});
					},
					[](MyNode* node) {
						Optimizer* opt = (Optimizer*)node->attachment;
						if (IsInputSlotFull(node)) opt->Forward();
					}
				);} },

				{ "Optimizer", []() -> MyNode* { return new MyNode("Adam",
					{	/// Input slots
						{ "target",		Slot_Matrix,	DataSizeDef(0, 1, 1) }
					}, {},
					[](MyNode* node, bool hideInNode) {
						OptiRenderer(node, hideInNode, [](MyNode* node)-> void {
							Adam* opt = (Adam*)node->attachment;
							ImGui::DragFloat("S", &opt->friction.x, 0.01f);
							ImGui::DragFloat("V", &opt->friction.y, 0.01f);
						}, [](MyNode* node)-> void {
							Adam* opt = (Adam*)node->attachment;
							ImGui::TextUnformatted(("S: " + Strutil::to_string_precision(opt->friction.x, 1, 3)).c_str());
							ImGui::TextUnformatted(("V: " + Strutil::to_string_precision(opt->friction.y, 1, 3)).c_str());
						});
					},
					[](MyNode* node) {
						Optimizer* opt = (Optimizer*)node->attachment;
						if (IsInputSlotFull(node)) opt->Forward();
					}
				);} },
				#pragma endregion
				// Vis Nodes ------------------------------------------------------------------------
				#pragma region Visualization
				{ "Visualization", []() -> MyNode* { return new MyNode("BatchPlot",
					{	/// Input slots
						{ "prop",		InnerData,		DataSizeDef(1, 1, 0),	true},
						{ "feature",	Slot_Matrix,	DataSizeDef(1, 0, 1) },
						{ "data",		Slot_Matrix,	DataSizeDef(0, 0, 1) }
					}, {
						{ "out",		Slot_Matrix,	DataSizeDef(0, 0, 1)}
					},
					[](MyNode* node, bool hideInNode) {
						if (!hideInNode) {
							node->ListName();
							if (ImGui::Button("Reset")) {
								outputMat(0).NewMat(1, 1, 0);
								inputID(1, 0) = 0;
							}
							const char* params[] = { "Batch", "Additive" };
							if (ImGui::Combo("sequence", &inputID(0, 0), params, IM_ARRAYSIZE(params))) node->Backward();
							if (ImGui::DragFloat("field length", &inputVec(0, 0).z, 0.1f)) node->Backward();
							ImGui::Separator();
							ShowMatrix(outputMat(0), inputVec(0, 0).z);
						} else {
							std::string tmp = "(" + std::to_string((int)outputMat(0).size.x) + u8"¡Á" + std::to_string((int)outputMat(0).size.y) + ")";
							if (IsInputSlotFull(node)) ImGui::TextUnformatted(tmp.c_str());
							else ImGui::TextUnformatted(ICON_FA_BAN);
						}
					}, [](MyNode* node) {
						if (inputVec(0, 0).z < 1) inputVec(0, 0).z = 1;

						if (IsInputSlotFull(node)) {
							int batch = parentNode(1)->output_slots[1].data.col[0].y;
							int currBatch = parentNode(1)->output_slots[1].data.id[0];

							if (inputID(1, 0) != currBatch) {
								if (currBatch <= 1 || outputMat(0).size == Vector2::ONE()) {
									Matrix::AppendRow(inputMat(1), inputMat(2), outputMat(0));
								} else {
									Matrix newCol(1, 0), prev(outputMat(0));
									Matrix::AppendRow(inputMat(1), inputMat(2), newCol);
									Matrix::AppendCol(prev, newCol, outputMat(0));
								}
								inputID(1, 0) = currBatch;
							}
						}
					}
				);} },
				
				{ "Visualization", []() -> MyNode* { return new MyNode("Plot 2D",
					{	/// Input slots
						{ "prop",		InnerData,		DataSizeDef(1, 1, 0),	true}, // item number, size of dot/line
						{ "item_1",		Slot_Matrix,	DataSizeDef(1, 1, 1),	false},
						{ "item_2",		Slot_Matrix,	DataSizeDef(1, 1, 1),	true},
						{ "item_3",		Slot_Matrix,	DataSizeDef(1, 1, 1),	true},
						{ "item_4",		Slot_Matrix,	DataSizeDef(1, 1, 1),	true},
						{ "item_5",		Slot_Matrix,	DataSizeDef(1, 1, 1),	true},
						{ "item_6",		Slot_Matrix,	DataSizeDef(1, 1, 1),	true},
						{ "item_7",		Slot_Matrix,	DataSizeDef(1, 1, 1),	true},
						{ "item_8",		Slot_Matrix,	DataSizeDef(1, 1, 1),	true},
						{ "item_9",		Slot_Matrix,	DataSizeDef(1, 1, 1),	true},
						{ "item_10",	Slot_Matrix,	DataSizeDef(1, 1, 1),	true},
					}, {},
					[](MyNode* node, bool hideInNode) {
						Plotter* plt = (Plotter*)node->attachment;
						if (ImGui::IsPlotHovered() || ImGui::IsPlotFrameHovered()) {
							BlockMouseAction(node);
						}
						if (!hideInNode) {
							node->ListName();
							if (ImGui::InputInt("item num", &inputID(0, 0), 1, 1)) node->Backward();
							if (ImGui::DragFloat("line width", &inputVec(0, 0).x, 0.1f)) node->Backward();
							if (ImGui::DragFloat("dot size", &inputVec(0, 0).y, 0.1f)) node->Backward();
							if (ImGui::DragFloat("bar width", &inputVec(0, 0).z, 0.1f)) node->Backward();

							plt->Draw(ImVec2(ImGui::GetContentRegionAvailWidth(), ImGui::GetContentRegionAvailWidth() / 1.3f));

							for (int i = 1; i < 11; i++) {
								auto slot = inputSlot(i); if (slot.hideSlot) break;
								if (slot.parent != nullptr) {
									ImGui::Separator();
									const char* items[] = { "Line", "Scatter", "XBar", "YBar" };
									if (ImGui::Combo((inputSlot(i).title + "_type").c_str(),
										&inputID(i, 0), items, IM_ARRAYSIZE(items))) node->Backward();

									if (ImGui::ColorEdit3((inputSlot(i).title + "_color").c_str(),
										(float*)&inputVec(i, 0))) node->Backward();
								}
							}
						} else {
							plt->Draw(ImVec2(400 * node->zoomFactor, 300 * node->zoomFactor));
						}
					}, [](MyNode* node) {
						if (inputID(0, 0) < 1) inputID(0, 0) = 1;
						if (inputID(0, 0) > 10) inputID(0, 0) = 10;
						if (inputVec(0, 0).x < 1) inputVec(0, 0).x = 1;
						if (inputVec(0, 0).y < 1) inputVec(0, 0).y = 1;
						if (inputVec(0, 0).z < 1) inputVec(0, 0).z = 1;

						Plotter* plt = (Plotter*)node->attachment;
						plt->items.clear();
						for (int i = 1; i < 11; i++) {
							inputSlot(i).hideSlot = i <= inputID(0, 0) ? false : true;
							if (i <= inputID(0, 0) && parentNode(i) != nullptr) {
								plt->AddItem(parentNode(i)->title, inputID(i, 0), inputID(i, 0) ? 
									(inputID(i, 0) > 1 ? inputVec(0, 0).z : inputVec(0, 0).y) : 
									inputVec(0, 0).x, VecToCol(inputVec(i, 0)), MatToVec(inputMat(i)));
							}
						}
					}
				);} },

				// TODO
				{ "Visualization", []() -> MyNode* { return new MyNode("Plot 3D",
					{	/// Input slots
						{ "target",		Slot_Matrix,	DataSizeDef(0, 1, 0) }
					}, {},
					[](MyNode* node, bool hideInNode) {
						
					}, [](MyNode* node) {}
				);} }
				#pragma endregion
			};

			std::cout << std::endl;
			std::cout << "------------------------------------------------ new NGraph added ------------------------------------------------" << std::endl;
			std::cout << "- id: " + std::to_string(id) << std::endl;
			std::cout << "- name: " + name << std::endl;
		}

		void NGraph::PopMenu() {
			// Data Nodes ----------------------------------
			if (ImGui::BeginMenu("Data")) {
				ListSlots("Data");
				ImGui::EndMenu();
			}

			// Operators -----------------------------------
			if (ImGui::BeginMenu("Operator")) {
				ListSlots("Operator");
				ImGui::EndMenu();
			}

			// Neurons -------------------------------------
			if (ImGui::BeginMenu("Neuron")) {
				ListSlots("Neuron");
				ImGui::EndMenu();
			}

			// Optimizers ----------------------------------
			if (ImGui::BeginMenu("Optimizer")) {
				ListSlots("Optimizer");
				ImGui::EndMenu();
			}

			// Visualization -------------------------------
			if (ImGui::BeginMenu("Visualization")) {
				ListSlots("Visualization");
				ImGui::EndMenu();
			}

			// Other funcs ---------------------------------
			if (anythingSelected) {
				ImGui::Separator();
				if (ImGui::MenuItem("Delete")) {}
				if (ImGui::MenuItem("Copy")) {}
				if (ImGui::MenuItem("Paste")) {}
			}
		}

	}
}


