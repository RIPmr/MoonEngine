#include "MaterialEditor.h"

using namespace ImNodes;

namespace MOON {
	MaterialEditor::MaterialEditor() {
		//available_nodes = new Nodes{
		//// Numeric nodes -----------------------------------------------------------------
		//{ "Numeric", []() -> MyNode* { return new MyNode("Color", {}, {
		//	/// Output slots
		//	{ "out", Slot_Color, DataSizeDef(0, 1) }
		//}, 
		//	/// content renderer
		//	[](MyNode* node, bool hideInNode) {
		//		if (!hideInNode) {
		//			node->ListName();
		//			ImGui::Text("Output Color: ");
		//			ImGui::ColorEdit3("Color", (float*)&outputVec(0, 0),
		//				ImGuiColorEditFlags_NoLabel);
		//		} else {
		//			ImGui::ColorEdit3("Color", (float*)&outputVec(0, 0),
		//				ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel);
		//		}
		//	}
		//); } },

		//{ "Numeric", []() -> MyNode* { return new MyNode("Bitmap", {
		//	/// internal data
		//	{ "Offset",		InnerData,   DataSizeDef(0, 1), true },
		//	{ "Tiling",		InnerData,   DataSizeDef(0, 1), true },
		//	{ "FiltType",	InnerData,   DataSizeDef(1, 0), true }
		//}, {
		//	/// Output slots
		//	{ "out", Slot_Texture, DataSizeDef(1, 0) }
		//}, 
		//	/// content renderer
		//	[](MyNode* node, bool hideInNode) {
		//		unsigned int prevID = outputID(0, 0) ? outputID(0, 0) :
		//							MOON_TextureManager::GetItem("moon_logo_full")->ID;
		//		Texture* tex = dynamic_cast<Texture*>(MOON_ObjectList[prevID]);
		//		int width = 30;

		//		if (!hideInNode) {
		//			node->ListName();
		//			ImGui::Text("Location: ");
		//			ImGui::Button(tex->path.c_str(), ImVec2(ImGui::GetContentRegionAvailWidth(), 0));
		//			ImGui::Text("UV: ");
		//			ImGui::DragFloat2("Offset", (float*)&inputVec(0, 0));
		//			ImGui::DragFloat2("Tiling", (float*)&inputVec(1, 0));
		//			//ImGui::Text("Filtering: ");
		//			//ImGui::RadioButton();

		//			ImGui::Separator();
		//			ImGui::Text("Preview: ");
		//			width <<= 2;
		//		} else width *= node->zoomFactor;
		//		ImGui::Image((void*)(intptr_t)tex->localID, ImVec2(width, tex->height / tex->width * width));
		//	}
		//); }},

		//// Operators ---------------------------------------------------------------------
		//{ "Operator", []() -> MyNode* { return new MyNode("ColorMixer", {
		//	/// internal data
		//	{ "Ratio",	InnerData,  DataSizeDef(1, 0), true },
		//	/// Input slots
		//	{ "A",		Slot_Color, DataSizeDef(0, 1) },
		//	{ "B",		Slot_Color, DataSizeDef(0, 1) }
		//}, {
		//	/// Output slots
		//	{ "Mixed",	Slot_Color, DataSizeDef(0, 1) }
		//},
		//	/// content renderer
		//	[](MyNode* node, bool hideInNode) {
		//		if (!hideInNode) {
		//			node->ListName();
		//			ImGui::Text("Input Color: ");
		//			ImGui::Text("A: "); ImGui::SameLine();
		//			ImGui::ColorEdit3("ColorA", (float*)&inputVec(1, 0),
		//				ImGuiColorEditFlags_NoLabel);
		//			ImGui::Text("B: "); ImGui::SameLine();
		//			ImGui::ColorEdit3("ColorB", (float*)&inputVec(2, 0),
		//				ImGuiColorEditFlags_NoLabel);

		//			ImGui::Separator();
		//			ImGui::SliderInt("Mix Ratio", &inputID(0, 0), 0, 100);

		//			ImGui::Separator();
		//			ImGui::Text("Output Color: ");
		//			ImGui::ColorEdit3("ColorO", (float*)&outputVec(0, 0),
		//				ImGuiColorEditFlags_NoLabel);
		//		} else {
		//			ImGui::ColorEdit3("ColorO", (float*)&outputVec(0, 0),
		//				ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel);
		//		}

		//		outputVec(0, 0).setValue(Vector3::Lerp(inputVec(1, 0), inputVec(2, 0),
		//			(float)inputID(0, 0) / 100.0f));
		//	}
		//); } },

		//{ "Operator", []() -> MyNode* { return new MyNode("ColtoTex", {
		//	/// Input slots
		//	{ "Color",	 Slot_Color, DataSizeDef(0, 1) }
		//}, {
		//	/// Output slots
		//	{ "Texture", Slot_Texture, DataSizeDef(1, 0) }
		//},
		//	/// content renderer
		//	[](MyNode* node, bool hideInNode) {
		//		if (!hideInNode) node->ListName();

		//		// TODO : generate a new tex based on input color

		//	}
		//); } },

		//{ "Operator", []() -> MyNode* { return new MyNode("LiteTexFilter", {
		//	/// internal data
		//	{ "Invert",		InnerData,   DataSizeDef(1, 0), true },
		//	{ "OutAmt",		InnerData,   DataSizeDef(1, 0), true },
		//	{ "Level",		InnerData,   DataSizeDef(1, 0), true },
		//	{ "HueShift",	InnerData,   DataSizeDef(1, 0), true },
		//	{ "Saturation",	InnerData,   DataSizeDef(1, 0), true },
		//	{ "Brightness",	InnerData,   DataSizeDef(1, 0), true },
		//	{ "Contrast",	InnerData,   DataSizeDef(1, 0), true },
		//	/// Input slots
		//	{ "Input", Slot_Texture, DataSizeDef(1, 0) }
		//}, {
		//	/// Output slots
		//	{ "Output",Slot_Texture, DataSizeDef(1, 0) }
		//},
		//	/// content renderer
		//	[](MyNode* node, bool hideInNode) {
		//		if (!hideInNode) {
		//			node->ListName();

		//			ImGui::Text("Filters: ");

		//			for (auto &iter : node->input_slots) {
		//				ImGui::SliderInt(iter.title.c_str(), &iter.data.id[0], 0, 100);
		//			}
		//		}
		//	}
		//); } },

		//// Procedural --------------------------------------------------------------------
		//{ "Procedural", []() -> MyNode* { return new MyNode("Perline", {
		//	/// internal data
		//}, {
		//	/// Output slots
		//	{ "Out",	Slot_Texture, DataSizeDef(1, 0) }
		//},
		//	/// content renderer
		//	[](MyNode* node, bool hideInNode) {
		//		unsigned int prevID = outputID(0, 0) ? outputID(0, 0) :
		//			MOON_TextureManager::GetItem("moon_logo_full")->ID;
		//		Texture* tex = dynamic_cast<Texture*>(MOON_ObjectList[prevID]);
		//		int width = 30 * node->zoomFactor;

		//		if (!hideInNode) {
		//			node->ListName();

		//			ImGui::Text("Preview: ");
		//			width <<= 2;
		//		}

		//		ImGui::Image((void*)(intptr_t)tex->localID, ImVec2(width, tex->height / tex->width * width));
		//	}
		//); } },

		//{ "Procedural", []() -> MyNode* { return new MyNode("Wood", {
		//	/// internal data
		//}, {
		//	/// Output slots
		//	{ "Out",	Slot_Texture, DataSizeDef(1, 0) }
		//},
		//	/// content renderer
		//	[](MyNode* node, bool hideInNode) {
		//		unsigned int prevID = outputID(0, 0) ? outputID(0, 0) :
		//			MOON_TextureManager::GetItem("moon_logo_full")->ID;
		//		Texture* tex = dynamic_cast<Texture*>(MOON_ObjectList[prevID]);
		//		int width = 30 * node->zoomFactor;

		//		if (!hideInNode) {
		//			node->ListName();

		//			ImGui::Text("Preview: ");
		//			width <<= 2;
		//		}

		//		ImGui::Image((void*)(intptr_t)tex->localID, ImVec2(width, tex->height / tex->width * width));
		//	}
		//); } },

		//{ "Procedural", []() -> MyNode* { return new MyNode("Worley", {
		//	/// internal data
		//}, {
		//	/// Output slots
		//	{ "Out",	Slot_Texture, DataSizeDef(1, 0) }
		//},
		//	/// content renderer
		//	[](MyNode* node, bool hideInNode) {
		//		unsigned int prevID = outputID(0, 0) ? outputID(0, 0) :
		//			MOON_TextureManager::GetItem("moon_logo_full")->ID;
		//		Texture* tex = dynamic_cast<Texture*>(MOON_ObjectList[prevID]);
		//		int width = 30 * node->zoomFactor;

		//		if (!hideInNode) {
		//			node->ListName();

		//			ImGui::Text("Preview: ");
		//			width <<= 2;
		//		}

		//		ImGui::Image((void*)(intptr_t)tex->localID, ImVec2(width, tex->height / tex->width * width));
		//	}
		//); } },

		//{ "Procedural", []() -> MyNode* { return new MyNode("Marble", {
		//	/// internal data
		//}, {
		//	/// Output slots
		//	{ "Out",	Slot_Texture, DataSizeDef(1, 0) }
		//},
		//	/// content renderer
		//	[](MyNode* node, bool hideInNode) {
		//		unsigned int prevID = outputID(0, 0) ? outputID(0, 0) :
		//			MOON_TextureManager::GetItem("moon_logo_full")->ID;
		//		Texture* tex = dynamic_cast<Texture*>(MOON_ObjectList[prevID]);
		//		int width = 30 * node->zoomFactor;

		//		if (!hideInNode) {
		//			node->ListName();

		//			ImGui::Text("Preview: ");
		//			width <<= 2;
		//		}

		//		ImGui::Image((void*)(intptr_t)tex->localID, ImVec2(width, tex->height / tex->width * width));
		//	}
		//); } },

		//{ "Procedural", []() -> MyNode* { return new MyNode("[Custom]", {
		//	/// internal data
		//}, {
		//	/// Output slots
		//	{ "Out",	Slot_Texture, DataSizeDef(1, 0) }
		//},
		//	/// content renderer
		//	[](MyNode* node, bool hideInNode) {
		//		unsigned int prevID = outputID(0, 0) ? outputID(0, 0) :
		//			MOON_TextureManager::GetItem("moon_logo_full")->ID;
		//		Texture* tex = dynamic_cast<Texture*>(MOON_ObjectList[prevID]);
		//		int width = 30 * node->zoomFactor;

		//		if (!hideInNode) {
		//			node->ListName();

		//			ImGui::Text("Preview: ");
		//			width <<= 2;
		//		}

		//		ImGui::Image((void*)(intptr_t)tex->localID, ImVec2(width, tex->height / tex->width * width));
		//	}
		//); } },

		//// Materials ---------------------------------------------------------------------
		//{ "Material", []() -> MyNode* { return new MyNode("MoonMtl", {
		//	/// Input slots
		//	{ "Ambient",	Slot_Color, DataSizeDef(1, 0) },
		//	{ "Diffuse",	Slot_Color, DataSizeDef(0, 1) },
		//	{ "Specular",	Slot_Color, DataSizeDef(1, 0) },
		//	{ "SpecExpo",	Slot_Color, DataSizeDef(1, 0) },
		//	{ "OptiDens",	Slot_Color, DataSizeDef(1, 0) },
		//	{ "Dissolve",	Slot_Color, DataSizeDef(1, 0) },
		//	{ "Illumina",	Slot_Color, DataSizeDef(1, 0) }
		//}, {
		//	/// Output slots
		//	{ "MoonMtl",	Slot_Material, DataSizeDef(1, 0) }
		//},
		//	/// content renderer
		//	[](MyNode* node, bool hideInNode) {
		//		if (!hideInNode) {
		//			if (outputID(0, 0)) {
		//				node->title = MOON_ObjectList[outputID(0, 0)]->name;

		//				// get input data
		//				MoonMtl* mat = ((MoonMtl*)MOON_ObjectList[outputID(0, 0)]);
		//				if (node->input_slots[1].parent != NULL) {
		//					if (mat->Kd != inputVec(1,0)) {
		//						mat->Kd.setValue(inputVec(1,0));
		//						mat->prevNeedUpdate = true;
		//					}
		//				}

		//				MOON_ObjectList[outputID(0, 0)]->ListProperties();
		//			} else {
		//				Material* newMat = MOON_MaterialManager::CreateMaterial("MoonMtl", "MoonMtl");
		//				outputID(0, 0) = newMat->ID;
		//			}
		//		}
		//	}
		//); } },

		//{ "Material", []() -> MyNode* { return new MyNode("Light", {
		//	/// Input slots
		//	{ "Illumination",	Slot_Texture, DataSizeDef(1, 0) },
		//	{ "Temperature",	Slot_Texture, DataSizeDef(1, 0) }
		//}, {
		//	/// Output slots
		//	{ "LightMtl",	Slot_Material, DataSizeDef(1, 0) }
		//},
		//	/// content renderer
		//	[](MyNode* node, bool hideInNode) {
		//		if (!hideInNode) {
		//			if (outputID(0, 0))
		//				MOON_ObjectList[outputID(0, 0)]->ListProperties();
		//			else {
		//				// TODO : Create a new mat

		//			}
		//		}
		//	}
		//); } },

		//{ "Material", []() -> MyNode* { return new MyNode("SSS", {
		//	/// Input slots
		//	{ "Diffuse",	Slot_Texture, DataSizeDef(1, 0) },
		//	{ "Highlight",	Slot_Texture, DataSizeDef(1, 0) },
		//	{ "SSS_Color",	Slot_Texture, DataSizeDef(1, 0) },
		//	{ "Thickness",	Slot_Texture, DataSizeDef(1, 0) }
		//}, {
		//	/// Output slots
		//	{ "SSSMtl",	Slot_Material, DataSizeDef(1, 0) }
		//},
		//	/// content renderer
		//	[](MyNode* node, bool hideInNode) {
		//		if (!hideInNode) {
		//			if (outputID(0, 0))
		//				MOON_ObjectList[outputID(0, 0)]->ListProperties();
		//			else {
		//				// TODO : Create a new mat

		//			}
		//		}
		//	}
		//); } },

		//{ "Material", []() -> MyNode* { return new MyNode("Volumn", {
		//	/// Input slots
		//	{ "BaseColor",	Slot_Texture, DataSizeDef(1, 0) },
		//	{ "Scatter",	Slot_Texture, DataSizeDef(1, 0) }
		//}, {
		//	/// Output slots
		//	{ "VolumnMtl",	Slot_Material, DataSizeDef(1, 0) }
		//},
		//	/// content renderer
		//	[](MyNode* node, bool hideInNode) {
		//		if (!hideInNode) {
		//			if (outputID(0, 0))
		//				MOON_ObjectList[outputID(0, 0)]->ListProperties();
		//			else {
		//				// TODO : Create a new mat

		//			}
		//		}
		//	}
		//); } },

		//{ "Material", []() -> MyNode* { return new MyNode("Hair", {
		//	/// Input slots
		//	{ "Diffuse",	Slot_Texture, DataSizeDef(1, 0) },
		//	{ "Transmition",	Slot_Texture, DataSizeDef(1, 0) }
		//}, {
		//	/// Output slots
		//	{ "HairMtl",	Slot_Material, DataSizeDef(1, 0) }
		//},
		//	/// content renderer
		//	[](MyNode* node, bool hideInNode) {
		//		if (!hideInNode) {
		//			if (outputID(0, 0))
		//				MOON_ObjectList[outputID(0, 0)]->ListProperties();
		//			else {
		//				// TODO : Create a new mat

		//			}
		//		}
		//	}
		//); } },

		//{ "Material", []() -> MyNode* { return new MyNode("Matte", {
		//	/// Input slots
		//	{ "AffectBackground",	Slot_Number,   DataSizeDef(1, 0) }
		//}, {
		//	/// Output slots
		//	{ "MatteMtl",	Slot_Material, DataSizeDef(1, 0) }
		//},
		//	/// content renderer
		//	[](MyNode* node, bool hideInNode) {
		//		if (!hideInNode) {
		//			if (outputID(0, 0))
		//				MOON_ObjectList[outputID(0, 0)]->ListProperties();
		//			else {
		//				// TODO : Create a new mat

		//			}
		//		}
		//	}
		//); } },

		//{ "Material", []() -> MyNode* { return new MyNode("Cartoon", {
		//	/// Input slots
		//	{ "LineColor",	Slot_Number,   DataSizeDef(1, 0) },
		//	{ "Thickness",	Slot_Number,   DataSizeDef(1, 0) }
		//}, {
		//	/// Output slots
		//	{ "CartoonMtl",	Slot_Material, DataSizeDef(1, 0) }
		//},
		//	/// content renderer
		//	[](MyNode* node, bool hideInNode) {
		//		if (!hideInNode) {
		//			if (outputID(0, 0))
		//				MOON_ObjectList[outputID(0, 0)]->ListProperties();
		//			else {
		//				// TODO : Create a new mat

		//			}
		//		}
		//	}
		//); } },

		//{ "Material", []() -> MyNode* { return new MyNode("MatMixer", {
		//	/// internal data
		//	{ "Incremental",  InnerData,   DataSizeDef(1, 0), true },
		//	/// Input slots
		//	{ "BaseMat",	Slot_Material, DataSizeDef(1, 0) },

		//	{ "Coat_1",		Slot_Material, DataSizeDef(1, 0) },
		//	{ "Mask_1",		Slot_Texture,  DataSizeDef(1, 1), true },
		//	{ "Coat_2",		Slot_Material, DataSizeDef(1, 0) },
		//	{ "Mask_2",		Slot_Texture,  DataSizeDef(1, 1), true },
		//	{ "Coat_3",		Slot_Material, DataSizeDef(1, 0) },
		//	{ "Mask_3",		Slot_Texture,  DataSizeDef(1, 1), true },
		//	{ "Coat_4",		Slot_Material, DataSizeDef(1, 0) },
		//	{ "Mask_4",		Slot_Texture,  DataSizeDef(1, 1), true },
		//	{ "Coat_5",		Slot_Material, DataSizeDef(1, 0) },
		//	{ "Mask_5",		Slot_Texture,  DataSizeDef(1, 1), true }
		//}, {
		//	/// Output slots
		//	{ "Output",		Slot_Material, DataSizeDef(1, 0) }
		//},
		//	/// content renderer
		//	[](MyNode* node, bool hideInNode) {
		//		if (!hideInNode) node->ListName();

		//		int oldNum = (node->input_slots.size() - 2) >> 1;
		//		inputID(0, 0) = oldNum;

		//		ImGui::Text("Coat Layer: ");
		//		ImGui::InputInt("LayerCnt", &inputID(0, 0), 1, 1, 0, true, 1.5f);

		//		int diff = inputID(0, 0) - oldNum;
		//		if (inputID(0, 0) >= 1 && diff) {
		//			if (diff >= 0) {
		//				for (int i = 0; i < diff; i++) {
		//					node->input_slots.push_back({ std::string("Coat_") + std::to_string(++oldNum),  Slot_Material, DataSizeDef(1, 0) });
		//					node->input_slots.push_back({ std::string("Mask_") + std::to_string(oldNum),    Slot_Texture,  DataSizeDef(1, 1), true });
		//				}
		//			} else {
		//				for (int i = 0; i > diff << 1; i--) node->input_slots.pop_back();
		//			}
		//		}

		//		if (!hideInNode) {
		//			ImGui::Separator();
		//			ImGui::Text("Base Material: "); ImGui::SameLine();
		//			ImGui::Button(inputID(1, 0) ? MOON_ObjectList[inputID(1, 0)]->name.c_str() : "[BaseMat]",
		//						  ImVec2(ImGui::GetContentRegionAvailWidth() - 10.0f, 0));
		//			// list all coat mats and masks
		//			int loopID = 1;
		//			for (auto iter = node->input_slots.begin() + 2; iter != node->input_slots.end(); iter++) {
		//				ImGui::Text(iter->title.c_str());
		//				ImGui::Indent(10.0f);
		//				ImGui::Button(iter->data.id[0] ? MOON_ObjectList[iter->data.id[0]]->name.c_str() : ("[" + iter->title + "]").c_str()); ImGui::SameLine();
		//				iter++;
		//				ImGui::Text("Mask: "); ImGui::SameLine();
		//				ImGui::ColorEdit3((iter->title + "_col").c_str(), (float*)&iter->data.col[0],
		//					ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel);  ImGui::SameLine();
		//				ImGui::Button("[MaskMap]", ImVec2(0, 0), loopID);
		//				ImGui::Unindent(10.0f);
		//			}
		//		}
		//	}
		//); } }

		//};
	}
}