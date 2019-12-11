#pragma once
#include <map>

#include "NodeEditorBase.h"
#include "Vector3.h"
#include "Vector4.h"
#include "Color.h"
#include "Matrix4x4.h"

#define DataSizeDef ImNodes::Ez::SlotData
#define inputData(slotID) node->input_slots[slotID].data
#define outputData(slotID) node->output_slots[slotID].data
#define IMGUI_DEFINE_MATH_OPERATORS

namespace MOON {

	struct MaterialEditor {
		bool anythingSelected;
		unsigned int selectedID;
		MyNode* selectedNode;
		bool flipExecute;

		std::multimap<std::string, MyNode*(*)()> available_nodes{
			// Numeric nodes -----------------------------------------------------------------
			{ "Numeric", []() -> MyNode* { return new MyNode("Color", {}, {
				/// Output slots
				{ "out", Slot_Color, DataSizeDef(0, 1) }
			}, 
				/// content renderer
				[](MyNode* node, bool hideInNode) {
					if (!hideInNode) {
						node->ListName();
						ImGui::Text("Output Color: ");
						ImGui::ColorEdit3("Color", (float*)&outputData(0).col[0],
							ImGuiColorEditFlags_NoLabel);
					} else {
						ImGui::ColorEdit3("Color", (float*)&outputData(0).col[0],
							ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel);
					}
				}
			); } },

			{ "Numeric", []() -> MyNode* { return new MyNode("Bitmap", {
				/// internal data
				{ "Offset",		InnerData,   DataSizeDef(0, 1), true },
				{ "Tiling",		InnerData,   DataSizeDef(0, 1), true },
				{ "FiltType",	InnerData,   DataSizeDef(1, 0), true }
			}, {
				/// Output slots
				{ "out", Slot_Texture, DataSizeDef(1, 0) }
			}, 
				/// content renderer
				[](MyNode* node, bool hideInNode) {
					unsigned int prevID = outputData(0).id[0] ? outputData(0).id[0] :
											MOON_TextureManager::GetItem("moon_logo_full")->ID;
					Texture* tex = dynamic_cast<Texture*>(MOON_ObjectList[prevID]);
					int width = 30 * node->zoomFactor;

					if (!hideInNode) {
						node->ListName();
						ImGui::Text("Location: ");
						ImGui::Button(tex->path.c_str(), ImVec2(ImGui::GetContentRegionAvailWidth(), 0));
						ImGui::Text("UV: ");
						ImGui::DragFloat2("Offset", (float*)&inputData(0).col[0]);
						ImGui::DragFloat2("Tiling", (float*)&inputData(1).col[0]);
						//ImGui::Text("Filtering: ");
						//ImGui::RadioButton();

						ImGui::Separator();
						ImGui::Text("Preview: ");
						width <<= 2;
					}

					ImGui::Image((void*)(intptr_t)tex->localID, ImVec2(width, tex->height / tex->width * width));
				}
			); }},

			// Operators ---------------------------------------------------------------------
			{ "Operator", []() -> MyNode* { return new MyNode("ColorMixer", {
				/// internal data
				{ "Ratio",	InnerData,  DataSizeDef(1, 0), true },
				/// Input slots
				{ "A",		Slot_Color, DataSizeDef(0, 1) },
				{ "B",		Slot_Color, DataSizeDef(0, 1) }
			}, {
				/// Output slots
				{ "Mixed",	Slot_Color, DataSizeDef(0, 1) }
			},
				/// content renderer
				[](MyNode* node, bool hideInNode) {
					if (!hideInNode) {
						node->ListName();
						ImGui::Text("Input Color: ");
						ImGui::Text("A: "); ImGui::SameLine();
						ImGui::ColorEdit3("ColorA", (float*)&inputData(1).col[0],
							ImGuiColorEditFlags_NoLabel);
						ImGui::Text("B: "); ImGui::SameLine();
						ImGui::ColorEdit3("ColorB", (float*)&inputData(2).col[0],
							ImGuiColorEditFlags_NoLabel);

						ImGui::Separator();
						ImGui::SliderInt("Mix Ratio", &inputData(0).id[0], 0, 100);

						ImGui::Separator();
						ImGui::Text("Output Color: ");
						ImGui::ColorEdit3("ColorO", (float*)&outputData(0).col[0],
							ImGuiColorEditFlags_NoLabel);
					} else {
						ImGui::ColorEdit3("ColorO", (float*)&outputData(0).col[0],
							ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel);
					}

					outputData(0).col[0].setValue(Vector3::Lerp(inputData(1).col[0], inputData(2).col[0],
						(float)inputData(0).id[0] / 100.0f));
				}
			); } },

			{ "Operator", []() -> MyNode* { return new MyNode("ColtoTex", {
				/// Input slots
				{ "Color",	 Slot_Color, DataSizeDef(0, 1) }
			}, {
				/// Output slots
				{ "Texture", Slot_Texture, DataSizeDef(1, 0) }
			},
				/// content renderer
				[](MyNode* node, bool hideInNode) {
					if (!hideInNode) node->ListName();

					// TODO : generate a new tex based on input color

				}
			); } },

			{ "Operator", []() -> MyNode* { return new MyNode("LiteTexFilter", {
				/// internal data
				{ "Invert",		InnerData,   DataSizeDef(1, 0), true },
				{ "OutAmt",		InnerData,   DataSizeDef(1, 0), true },
				{ "Level",		InnerData,   DataSizeDef(1, 0), true },
				{ "HueShift",	InnerData,   DataSizeDef(1, 0), true },
				{ "Saturation",	InnerData,   DataSizeDef(1, 0), true },
				{ "Brightness",	InnerData,   DataSizeDef(1, 0), true },
				{ "Contrast",	InnerData,   DataSizeDef(1, 0), true },
				/// Input slots
				{ "Input", Slot_Texture, DataSizeDef(1, 0) }
			}, {
				/// Output slots
				{ "Output",Slot_Texture, DataSizeDef(1, 0) }
			},
				/// content renderer
				[](MyNode* node, bool hideInNode) {
					if (!hideInNode) {
						node->ListName();

						ImGui::Text("Filters: ");

						for (auto &iter : node->input_slots) {
							ImGui::SliderInt(iter.title.c_str(), &iter.data.id[0], 0, 100);
						}
					}
				}
			); } },

			// Procedural --------------------------------------------------------------------
			{ "Procedural", []() -> MyNode* { return new MyNode("Perline", {
				/// internal data
			}, {
				/// Output slots
				{ "Out",	Slot_Texture, DataSizeDef(1, 0) }
			},
				/// content renderer
				[](MyNode* node, bool hideInNode) {
					unsigned int prevID = outputData(0).id[0] ? outputData(0).id[0] :
						MOON_TextureManager::GetItem("moon_logo_full")->ID;
					Texture* tex = dynamic_cast<Texture*>(MOON_ObjectList[prevID]);
					int width = 30 * node->zoomFactor;

					if (!hideInNode) {
						node->ListName();

						ImGui::Text("Preview: ");
						width <<= 2;
					}

					ImGui::Image((void*)(intptr_t)tex->localID, ImVec2(width, tex->height / tex->width * width));
				}
			); } },

			{ "Procedural", []() -> MyNode* { return new MyNode("Wood", {
				/// internal data
			}, {
				/// Output slots
				{ "Out",	Slot_Texture, DataSizeDef(1, 0) }
			},
				/// content renderer
				[](MyNode* node, bool hideInNode) {
					unsigned int prevID = outputData(0).id[0] ? outputData(0).id[0] :
						MOON_TextureManager::GetItem("moon_logo_full")->ID;
					Texture* tex = dynamic_cast<Texture*>(MOON_ObjectList[prevID]);
					int width = 30 * node->zoomFactor;

					if (!hideInNode) {
						node->ListName();

						ImGui::Text("Preview: ");
						width <<= 2;
					}

					ImGui::Image((void*)(intptr_t)tex->localID, ImVec2(width, tex->height / tex->width * width));
				}
			); } },

			{ "Procedural", []() -> MyNode* { return new MyNode("Worley", {
				/// internal data
			}, {
				/// Output slots
				{ "Out",	Slot_Texture, DataSizeDef(1, 0) }
			},
				/// content renderer
				[](MyNode* node, bool hideInNode) {
					unsigned int prevID = outputData(0).id[0] ? outputData(0).id[0] :
						MOON_TextureManager::GetItem("moon_logo_full")->ID;
					Texture* tex = dynamic_cast<Texture*>(MOON_ObjectList[prevID]);
					int width = 30 * node->zoomFactor;

					if (!hideInNode) {
						node->ListName();

						ImGui::Text("Preview: ");
						width <<= 2;
					}

					ImGui::Image((void*)(intptr_t)tex->localID, ImVec2(width, tex->height / tex->width * width));
				}
			); } },

			{ "Procedural", []() -> MyNode* { return new MyNode("Marble", {
				/// internal data
			}, {
				/// Output slots
				{ "Out",	Slot_Texture, DataSizeDef(1, 0) }
			},
				/// content renderer
				[](MyNode* node, bool hideInNode) {
					unsigned int prevID = outputData(0).id[0] ? outputData(0).id[0] :
						MOON_TextureManager::GetItem("moon_logo_full")->ID;
					Texture* tex = dynamic_cast<Texture*>(MOON_ObjectList[prevID]);
					int width = 30 * node->zoomFactor;

					if (!hideInNode) {
						node->ListName();

						ImGui::Text("Preview: ");
						width <<= 2;
					}

					ImGui::Image((void*)(intptr_t)tex->localID, ImVec2(width, tex->height / tex->width * width));
				}
			); } },

			{ "Procedural", []() -> MyNode* { return new MyNode("[Custom]", {
				/// internal data
			}, {
				/// Output slots
				{ "Out",	Slot_Texture, DataSizeDef(1, 0) }
			},
				/// content renderer
				[](MyNode* node, bool hideInNode) {
					unsigned int prevID = outputData(0).id[0] ? outputData(0).id[0] :
						MOON_TextureManager::GetItem("moon_logo_full")->ID;
					Texture* tex = dynamic_cast<Texture*>(MOON_ObjectList[prevID]);
					int width = 30 * node->zoomFactor;

					if (!hideInNode) {
						node->ListName();

						ImGui::Text("Preview: ");
						width <<= 2;
					}

					ImGui::Image((void*)(intptr_t)tex->localID, ImVec2(width, tex->height / tex->width * width));
				}
			); } },

			// Materials ---------------------------------------------------------------------
			{ "Material", []() -> MyNode* { return new MyNode("MoonMtl", {
				/// Input slots
				{ "Ambient",	Slot_Texture, DataSizeDef(1, 0) },
				{ "Diffuse",	Slot_Texture, DataSizeDef(1, 0) },
				{ "Specular",	Slot_Texture, DataSizeDef(1, 0) },
				{ "SpecExpo",	Slot_Texture, DataSizeDef(1, 0) },
				{ "OptiDens",	Slot_Texture, DataSizeDef(1, 0) },
				{ "Dissolve",	Slot_Texture, DataSizeDef(1, 0) },
				{ "Illumina",	Slot_Texture, DataSizeDef(1, 0) }
			}, {
				/// Output slots
				{ "MoonMtl",	Slot_Material, DataSizeDef(1, 0) }
			},
				/// content renderer
				[](MyNode* node, bool hideInNode) {
					if (!hideInNode) {
						if (outputData(0).id[0]) {
							node->title = MOON_ObjectList[outputData(0).id[0]]->name;
							MOON_ObjectList[outputData(0).id[0]]->ListProperties();
						} else {
							Material* newMat = MOON_MaterialManager::CreateMaterial("MoonMtl", "MoonMtl");
							outputData(0).id[0] = newMat->ID;
						}
					}
				}
			); } },

			{ "Material", []() -> MyNode* { return new MyNode("Light", {
				/// Input slots
				{ "Illumination",	Slot_Texture, DataSizeDef(1, 0) },
				{ "Temperature",	Slot_Texture, DataSizeDef(1, 0) }
			}, {
				/// Output slots
				{ "LightMtl",	Slot_Material, DataSizeDef(1, 0) }
			},
				/// content renderer
				[](MyNode* node, bool hideInNode) {
					if (!hideInNode) {
						if (outputData(0).id[0])
							MOON_ObjectList[outputData(0).id[0]]->ListProperties();
						else {
							// TODO : Create a new mat

						}
					}
				}
			); } },

			{ "Material", []() -> MyNode* { return new MyNode("SSS", {
				/// Input slots
				{ "Diffuse",	Slot_Texture, DataSizeDef(1, 0) },
				{ "Highlight",	Slot_Texture, DataSizeDef(1, 0) },
				{ "SSS_Color",	Slot_Texture, DataSizeDef(1, 0) },
				{ "Thickness",	Slot_Texture, DataSizeDef(1, 0) }
			}, {
				/// Output slots
				{ "SSSMtl",	Slot_Material, DataSizeDef(1, 0) }
			},
				/// content renderer
				[](MyNode* node, bool hideInNode) {
					if (!hideInNode) {
						if (outputData(0).id[0])
							MOON_ObjectList[outputData(0).id[0]]->ListProperties();
						else {
							// TODO : Create a new mat

						}
					}
				}
			); } },

			{ "Material", []() -> MyNode* { return new MyNode("Volumn", {
				/// Input slots
				{ "BaseColor",	Slot_Texture, DataSizeDef(1, 0) },
				{ "Scatter",	Slot_Texture, DataSizeDef(1, 0) }
			}, {
				/// Output slots
				{ "VolumnMtl",	Slot_Material, DataSizeDef(1, 0) }
			},
				/// content renderer
				[](MyNode* node, bool hideInNode) {
					if (!hideInNode) {
						if (outputData(0).id[0])
							MOON_ObjectList[outputData(0).id[0]]->ListProperties();
						else {
							// TODO : Create a new mat

						}
					}
				}
			); } },

			{ "Material", []() -> MyNode* { return new MyNode("Hair", {
				/// Input slots
				{ "Diffuse",	Slot_Texture, DataSizeDef(1, 0) },
				{ "Transmition",	Slot_Texture, DataSizeDef(1, 0) }
			}, {
				/// Output slots
				{ "HairMtl",	Slot_Material, DataSizeDef(1, 0) }
			},
				/// content renderer
				[](MyNode* node, bool hideInNode) {
					if (!hideInNode) {
						if (outputData(0).id[0])
							MOON_ObjectList[outputData(0).id[0]]->ListProperties();
						else {
							// TODO : Create a new mat

						}
					}
				}
			); } },

			{ "Material", []() -> MyNode* { return new MyNode("Matte", {
				/// Input slots
				{ "AffectBackground",	Slot_Number,   DataSizeDef(1, 0) }
			}, {
				/// Output slots
				{ "MatteMtl",	Slot_Material, DataSizeDef(1, 0) }
			},
				/// content renderer
				[](MyNode* node, bool hideInNode) {
					if (!hideInNode) {
						if (outputData(0).id[0])
							MOON_ObjectList[outputData(0).id[0]]->ListProperties();
						else {
							// TODO : Create a new mat

						}
					}
				}
			); } },

			{ "Material", []() -> MyNode* { return new MyNode("Cartoon", {
				/// Input slots
				{ "LineColor",	Slot_Number,   DataSizeDef(1, 0) },
				{ "Thickness",	Slot_Number,   DataSizeDef(1, 0) }
			}, {
				/// Output slots
				{ "CartoonMtl",	Slot_Material, DataSizeDef(1, 0) }
			},
				/// content renderer
				[](MyNode* node, bool hideInNode) {
					if (!hideInNode) {
						if (outputData(0).id[0])
							MOON_ObjectList[outputData(0).id[0]]->ListProperties();
						else {
							// TODO : Create a new mat

						}
					}
				}
			); } },

			{ "Material", []() -> MyNode* { return new MyNode("MatMixer", {
				/// internal data
				{ "Incremental",  InnerData,   DataSizeDef(1, 0), true },
				/// Input slots
				{ "BaseMat",	Slot_Material, DataSizeDef(1, 0) },

				{ "Coat_1",		Slot_Material, DataSizeDef(1, 0) },
				{ "Mask_1",		Slot_Texture,  DataSizeDef(1, 1), true },
				{ "Coat_2",		Slot_Material, DataSizeDef(1, 0) },
				{ "Mask_2",		Slot_Texture,  DataSizeDef(1, 1), true },
				{ "Coat_3",		Slot_Material, DataSizeDef(1, 0) },
				{ "Mask_3",		Slot_Texture,  DataSizeDef(1, 1), true },
				{ "Coat_4",		Slot_Material, DataSizeDef(1, 0) },
				{ "Mask_4",		Slot_Texture,  DataSizeDef(1, 1), true },
				{ "Coat_5",		Slot_Material, DataSizeDef(1, 0) },
				{ "Mask_5",		Slot_Texture,  DataSizeDef(1, 1), true }
			}, {
				/// Output slots
				{ "Output",		Slot_Material, DataSizeDef(1, 0) }
			},
				/// content renderer
				[](MyNode* node, bool hideInNode) {
					if (!hideInNode) node->ListName();

					int oldNum = (node->input_slots.size() - 2) >> 1;
					inputData(0).id[0] = oldNum;

					ImGui::Text("Coat Layer: ");
					ImGui::InputInt("LayerCnt", &inputData(0).id[0], 1, 1, 0, true, 1.5f);

					int diff = inputData(0).id[0] - oldNum;
					if (inputData(0).id[0] >= 1 && diff) {
						if (diff >= 0) {
							for (int i = 0; i < diff; i++) {
								node->input_slots.push_back({ std::string("Coat_") + std::to_string(++oldNum),  Slot_Material, DataSizeDef(1, 0) });
								node->input_slots.push_back({ std::string("Mask_") + std::to_string(oldNum),    Slot_Texture,  DataSizeDef(1, 1), true });
							}
						} else {
							for (int i = 0; i > diff << 1; i--) node->input_slots.pop_back();
						}
					}

					if (!hideInNode) {
						ImGui::Separator();
						ImGui::Text("Base Material: "); ImGui::SameLine();
						ImGui::Button(inputData(1).id[0] ? MOON_ObjectList[inputData(1).id[0]]->name.c_str() : "[BaseMat]", 
									  ImVec2(ImGui::GetContentRegionAvailWidth() - 10.0f, 0));
						// list all coat mats and masks
						int loopID = 1;
						for (auto iter = node->input_slots.begin() + 2; iter != node->input_slots.end(); iter++) {
							ImGui::Text(iter->title.c_str());
							ImGui::Indent(10.0f);
							ImGui::Button(iter->data.id[0] ? MOON_ObjectList[iter->data.id[0]]->name.c_str() : ("[" + iter->title + "]").c_str()); ImGui::SameLine();
							iter++;
							ImGui::Text("Mask: "); ImGui::SameLine();
							ImGui::ColorEdit3((iter->title + "_col").c_str(), (float*)&iter->data.col[0],
								ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel);  ImGui::SameLine();
							ImGui::Button("[MaskMap]", ImVec2(0, 0), loopID);
							ImGui::Unindent(10.0f);
						}
					}
				}
			); } }

		};
		std::vector<MyNode*> nodes;

		// TODO : clear canvas
		void ClearEditor() {
			std::cout << "node editor cleared." << std::endl;
		}

		void ListSlots(const std::string &typeName) {
			auto lower = available_nodes.lower_bound(typeName);
			auto upper = available_nodes.upper_bound(typeName);

			for (; lower != upper; lower++) {
				if (ImGui::MenuItem((*lower).second()->title.c_str())) {
					nodes.push_back((*lower).second());
					ImNodes::AutoPositionNode(nodes.back());
				}
			}
		}

		void CleanUp() {
			for (auto it = nodes.begin(); it != nodes.end(); it++) {
				delete (*it);
			}
			for (auto it = available_nodes.begin(); it != available_nodes.end(); it++) {
				delete (*it).second();
			}
		}

		void Draw() {
			// Canvas must be created after ImGui initializes, because constructor accesses ImGui style to configure default colors.
			static ImNodes::CanvasState canvas{};
			const ImGuiStyle& style = ImGui::GetStyle();

			ImNodes::BeginCanvas(&canvas);

			if (ImGui::IsWindowHovered()) 
				if (!ImGui::IsWindowFocused() && 
				   (ImGui::GetIO().MouseWheel != 0 ||
					ImGui::IsMouseDown(2)))
					ImGui::FocusWindow(ImGui::GetCurrentWindow());

			// restore props
			selectedID = 0;
			selectedNode = NULL;
			anythingSelected = false;
			flipExecute = !flipExecute;

			for (auto it = nodes.begin(); it != nodes.end();) {
				MyNode* node = *it;

				node->zoomFactor = canvas.zoom;

				if (node->selected) {
					anythingSelected = true;
					selectedNode = node;
					if (outputData(0).id != NULL) selectedID = outputData(0).id[0];
					/*std::cout << "- Connection: -" << std::endl;
					for (auto &iter : node->input_slots) {
						if (iter.con != NULL)
						std::cout << iter.title << " is con to: " << ((MyNode*)iter.parent)->title << std::endl;
					}*/
				}

				// Start rendering node
				if (ImNodes::Ez::BeginNode(node, node->title.c_str(), &node->pos, &node->selected)) {
					// Render input nodes first (order is important)
					ImNodes::Ez::InputSlots(node->input_slots.data(), node->input_slots.size());

					// node content go here
					//if (node->executed != flipExecute) node->Execute();
					//else node->ProcessContent(true);
					node->ProcessContent(true);

					// Render output nodes first (order is important)
					ImNodes::Ez::OutputSlots(node->output_slots.data(), node->output_slots.size());

					// Store new connections when they are created
					Connection new_connection;
					if (ImNodes::GetNewConnection(&new_connection.input_node, &new_connection.input_slot,
						&new_connection.output_node, &new_connection.output_slot)) {
						MyNode* conIn = (MyNode*)new_connection.input_node;
						MyNode* conOut = (MyNode*)new_connection.output_node;

						conIn->connections.push_back(new_connection);
						conOut->connections.push_back(new_connection);

						/// store connection ptr to input slot
						/// there is no need to store connection to output slot
						for (auto &islot : conIn->input_slots) {
							if (!strcmp(islot.title.c_str(), new_connection.input_slot)) {
								for (auto &oslot : conOut->output_slots) {
									if (!strcmp(oslot.title.c_str(), new_connection.output_slot)) {
										islot.con = &oslot;
										islot.parent = new_connection.output_node;
										break;
									}
								}
								break;
							}
						}

					}

					// Render output connections of this node
					for (const Connection& connection : node->connections) {
						// Node contains all it's connections (both from output and to input slots). This means that multiple
						// nodes will have same connection. We render only output connections and ensure that each connection
						// will be rendered once.
						if (connection.output_node != node)
							continue;

						if (!ImNodes::Connection(connection.input_node, connection.input_slot, connection.output_node,
							connection.output_slot)) {
							/// delete connection ptr in input slot
							MyNode* conIn = (MyNode*)connection.input_node;
							for (auto &islot : conIn->input_slots) {
								if (!strcmp(islot.title.c_str(), connection.input_slot)) {
									islot.con = NULL;
									islot.parent = NULL;
									break;
								}
							}

							// Remove deleted connections
							conIn->DeleteConnection(connection);
							((MyNode*)connection.output_node)->DeleteConnection(connection);
						}
					}
				}
				// Node rendering is done. This call will render node background based on size of content inside node.
				ImNodes::Ez::EndNode();

				if (node->selected && ImGui::IsKeyPressedMap(ImGuiKey_Delete)) {
					// Deletion order is critical: first we delete connections to us
					for (auto& connection : node->connections) {
						if (connection.output_node == node) {
							/// delete connection ptr in input slot
							for (auto &islot : ((MyNode*)connection.input_node)->input_slots) {
								if (!strcmp(islot.title.c_str(), connection.input_slot)) {
									islot.con = NULL;
									islot.parent = NULL;
									break;
								}
							}
							((MyNode*)connection.input_node)->DeleteConnection(connection);
						} else {
							((MyNode*)connection.output_node)->DeleteConnection(connection);
						}
					}
					// Then we delete our own connections, so we don't corrupt the list
					node->connections.clear();

					if (anythingSelected) {
						if (selectedNode == node) {
							anythingSelected = false;
							selectedNode = NULL;
							selectedID = 0;
						}
					}

					delete node;
					it = nodes.erase(it);
				} else ++it;
			}

			const ImGuiIO& io = ImGui::GetIO();
			if (ImGui::IsMouseReleased(1) && ImGui::IsWindowHovered() && !ImGui::IsMouseDragging(1)) {
				ImGui::FocusWindow(ImGui::GetCurrentWindow());
				ImGui::OpenPopup("NodesContextMenu");
			}

			ImGui::SetWindowFontScale(1.f);
			if (ImGui::BeginPopup("NodesContextMenu")) {
				// Numerics ----------------------------------
				if (ImGui::BeginMenu("Numerics")) {
					ListSlots("Numeric");
					ImGui::EndMenu();
				}

				// Operators ---------------------------------
				if (ImGui::BeginMenu("Operators")) {
					ListSlots("Operator");
					ImGui::EndMenu();
				}

				// Procedural Texture ------------------------
				if (ImGui::BeginMenu("Procedural")) {
					ListSlots("Procedural");
					ImGui::EndMenu();
				}

				ImGui::Separator();
				// Mtls --------------------------------------
				ListSlots("Material");

				// Other funcs -------------------------------
				if (anythingSelected) {
					ImGui::Separator();
					if (ImGui::MenuItem("Delete")) {}
					if (ImGui::MenuItem("Copy")) {}
					if (ImGui::MenuItem("Paste")) {}
				}
				//if (ImGui::MenuItem("Reset Zoom")) canvas.zoom = 1;

				//if (ImGui::IsAnyMouseDown() && !ImGui::IsWindowHovered()) ImGui::CloseCurrentPopup();
				ImGui::EndPopup();
			}

			ImNodes::EndCanvas();

		}
	};

}