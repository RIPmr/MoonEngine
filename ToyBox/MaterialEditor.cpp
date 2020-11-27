#include "MaterialEditor.h"
#include "ButtonEx.h"
#include "SceneMgr.h"

using namespace ImNodes;

namespace MOON {
	MaterialEditor::MaterialEditor() {
		available_nodes = new Nodes {

			// Numeric nodes -----------------------------------------------------------------
			{ "Numeric", []() -> MyNode* { return new MyNode("Color", 
				{	/// internal data

				}, 
				{	/// Output slots
					{ "out", Slot_Color, DataSizeDef(0, 1, 0) }
				},  /// content renderer
				[](MyNode* node, bool hideInNode) {
					if (!hideInNode) {
						node->ListName();
						ImGui::Text("Output Color: ");
						if (ImGui::ColorEdit3("Color", (float*)&outputVec(0, 0),
							ImGuiColorEditFlags_NoLabel)) node->Backward();
					} else {
						if (ImGui::ColorEdit3("Color", (float*)&outputVec(0, 0),
							ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel)) 
							node->Backward();
					}
				},	/// process data
				[](MyNode* node) {
					
				}
			);}},

			{ "Numeric", []() -> MyNode* { return new MyNode("Bitmap", 
				{	/// internal data

				}, 
				{	/// Output slots
					{ "out",	Slot_Texture,	DataSizeDef(1, 0, 0) }
				},	/// content renderer
				[](MyNode* node, bool hideInNode) {
					Texture* tex = dynamic_cast<Texture*>(MOON_ObjectList[outputID(0, 0)]);
					int width = 30;

					if (!hideInNode) {
						node->ListName();
						ImGui::Text("Location: ");
						if (ButtonEx::TexFileBtnWithPrev(
							tex, defaultMap, 
							ImVec2(ImGui::GetContentRegionAvailWidth(), 0), 
							tex->ID
						)) node->Backward();
						
						const static char* texType[] = { 
							"default", "ambient", "albedo", "reflect", "refract",
							"roughness", "glossiness", "normal", "height", "displace",
							"metallic", "translucent", "alpha"
						};
						ImGui::AlignTextToFramePadding(); ImGui::Text("Type: ");
						if (ButtonEx::ComboNoLabel("texType", (int*)&tex->type, texType, 
							IM_ARRAYSIZE(texType))) node->Backward();

						const static char* colorSpace[] = {
							"linear", "sRGB", "Gamma", "Cineon", "Canon_CLog",
							"AlexaV3LogC", "Panalog", "PLogLin", "RedLog",
							"Sony_SLog", "Sony_SLog2", "ViperLog", "rec709",
							"rec2020", "ACES", "ACEScg"
						};
						ImGui::AlignTextToFramePadding(); ImGui::Text("Color Space: ");
						if (ButtonEx::ComboNoLabel("colorSpace", (int*)&tex->colorSpace, 
							colorSpace, IM_ARRAYSIZE(colorSpace))) node->Backward();

						const static char* warpMode[] = {
							"REPEAT", "MIRRORED_REPEAT", "CLAMP_TO_EDGE", "CLAMP_TO_BORDER"
						};
						ImGui::AlignTextToFramePadding(); ImGui::Text("Warp Mode: ");
						int warp = 0; if (tex->warpMode == GL_REPEAT) warp = 0;
						else if (tex->warpMode == GL_MIRRORED_REPEAT) warp = 1;
						else if (tex->warpMode == GL_CLAMP_TO_EDGE) warp = 2;
						else if (tex->warpMode == GL_CLAMP_TO_BORDER) warp = 3;
						if (ButtonEx::ComboNoLabel("warpMode", &warp,
							warpMode, IM_ARRAYSIZE(warpMode))) {
							if (warp == 0) tex->warpMode = GL_REPEAT;
							else if (warp == 1) tex->warpMode = GL_MIRRORED_REPEAT;
							else if (warp == 2) tex->warpMode = GL_CLAMP_TO_EDGE;
							else if (warp == 3) tex->warpMode = GL_CLAMP_TO_BORDER;
							node->Backward();
						}

						const static char* filter[] = {
							"Bilinear", "Nearest"
						};
						ImGui::AlignTextToFramePadding(); ImGui::Text("Filter: ");
						int filt = 0; if (tex->filter == GL_LINEAR) filt = 0;
						else if (tex->filter == GL_NEAREST) filt = 1;
						if (ButtonEx::ComboNoLabel("filter", &filt,
							filter, IM_ARRAYSIZE(filter))) {
							if (filt == 0) tex->filter = GL_LINEAR;
							else if (filt == 1) tex->filter = GL_NEAREST;
							node->Backward();
						}

						ImGui::Text("UV: ");
						if (ImGui::DragFloat2("Offset", (float*)&tex->offset, 0.1f)) {
							node->Backward();
						}
						if (ImGui::DragFloat2("Tiling", (float*)&tex->tiling, 0.1f)) {
							node->Backward();
						}

						ImGui::Separator();
						ImGui::Text("Preview: ");
						width <<= 2;
						ImGui::SetCursorPosX((ImGui::GetContentRegionAvailWidth() - width) / 2.0f);
					} else width *= node->zoomFactor;

					ButtonEx::ClampedImage(
						tex, width, true, 
						ImVec2(tex->offset.x, tex->offset.y),
						ImVec2(
							tex->offset.x + tex->tiling.x,
							tex->offset.y + tex->tiling.y
						)
					);
				},	/// process data
				[](MyNode* node) {
					if (!outputID(0, 0)) {
						Texture* tex = new Texture(-1, -1, "bitmap");
						MOON_TextureManager::AddItem(tex);
						outputID(0, 0) = tex->ID;
					}
				}
			);}},

			// Operators ---------------------------------------------------------------------
			{ "Operator", []() -> MyNode* { return new MyNode("ColorMixer", 
				{	/// internal data
					{ "Ratio",	InnerData,  DataSizeDef(1, 0, 0), true },
					/// Input slots
					{ "A",		Slot_Color, DataSizeDef(0, 1, 0) },
					{ "B",		Slot_Color, DataSizeDef(0, 1, 0) }
				}, 
				{	/// Output slots
					{ "Mixed",	Slot_Color, DataSizeDef(0, 1, 0) }
				},	/// content renderer
				[](MyNode* node, bool hideInNode) {
					if (!hideInNode) {
						node->ListName();
						ImGui::Text("Input Color: ");
						ImGui::Text("A: "); ImGui::SameLine();
						ImGui::ColorEdit3(
							"ColorA", (float*)&inputVec(1, 0),
							ImGuiColorEditFlags_NoLabel
						);
						ImGui::Text("B: "); ImGui::SameLine();
						ImGui::ColorEdit3(
							"ColorB", (float*)&inputVec(2, 0),
							ImGuiColorEditFlags_NoLabel
						);

						ImGui::Separator();
						if (ImGui::SliderInt("Mix Ratio", &inputID(0, 0), 0, 100)) {
							node->Backward();
						}

						ImGui::Separator();
						ImGui::Text("Output Color: ");
						ImGui::ColorEdit3(
							"ColorO", (float*)&outputVec(0, 0),
							ImGuiColorEditFlags_NoLabel
						);
					} else {
						ImGui::ColorEdit3(
							"ColorO", (float*)&outputVec(0, 0),
							ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel
						);
					}
				},	/// process data
				[](MyNode* node) {
					outputVec(0, 0).setValue(
						Vector3::Lerp(
							inputVec(1, 0),
							inputVec(2, 0),
							(float)inputID(0, 0) / 100.0f
						)
					);
				}
			);}},

			{ "Operator", []() -> MyNode* { return new MyNode("TexFilter", 
				{	/// internal data
					{ "Invert",		InnerData,   DataSizeDef(1, 0, 0), true },
					{ "OutAmt",		InnerData,   DataSizeDef(1, 0, 0), true },
					{ "Level",		InnerData,   DataSizeDef(1, 0, 0), true },
					{ "HueShift",	InnerData,   DataSizeDef(1, 0, 0), true },
					{ "Saturation",	InnerData,   DataSizeDef(1, 0, 0), true },
					{ "Brightness",	InnerData,   DataSizeDef(1, 0, 0), true },
					{ "Contrast",	InnerData,   DataSizeDef(1, 0, 0), true },
					/// Input slots
					{ "Input", Slot_Texture, DataSizeDef(1, 0, 0) }
				},  
				{	/// Output slots
					{ "Output",Slot_Texture, DataSizeDef(1, 0, 0) }
				},	/// content renderer
				[](MyNode* node, bool hideInNode) {
					if (!hideInNode) {
						node->ListName();

						ImGui::Text("Filters: ");
						for (auto &iter : node->input_slots) {
							ImGui::SliderInt(iter.title.c_str(), &iter.data.id[0], 0, 100);
						}
					}
				},	/// process data
				[](MyNode* node) {

				}
			);}},

			// Procedural --------------------------------------------------------------------
			{ "Procedural", []() -> MyNode* { return new MyNode("Checker",
				{	/// internal data
					{ "Size",	InnerData,	 DataSizeDef(0, 1, 0), true },
					{ "Scale",	InnerData,	 DataSizeDef(0, 1, 0), true }
				},
				{	/// Output slots
					{ "out",	Slot_Texture, DataSizeDef(1, 0, 0) }
				},	/// content renderer
				[](MyNode* node, bool hideInNode) {
					FrameBuffer* tex = dynamic_cast<FrameBuffer*>(MOON_ObjectList[outputID(0, 0)]);
					int width = 30;

					if (!hideInNode) {
						node->ListName();

						float availWidth = ImGui::GetContentRegionAvailWidth() / 2.5f;
						ImGui::Text("Size:"); ImGui::SetNextItemWidth(availWidth);
						if (ButtonEx::InputFloatNoLabel("width", &inputVec(0, 0).x, 100, 1000, "%.0f")) {
							tex->Reallocate(inputVec(0, 0).x, inputVec(0, 0).y);
							node->Backward();
						}
						ImGui::SameLine(); ImGui::Text(u8"¡Á"); ImGui::SameLine();
						ImGui::SetNextItemWidth(availWidth);
						if (ButtonEx::InputFloatNoLabel("height", &inputVec(0, 0).y, 100, 1000, "%.0f")) {
							tex->Reallocate(inputVec(0, 0).x, inputVec(0, 0).y);
							node->Backward();
						}
						ImGui::Separator();

						ImGui::Text("Scale: ");
						if (ButtonEx::DragFloatNoLabel("Scale", &inputVec(1, 0).x, 0.1f)) {
							node->Backward();
						}

						ImGui::Text("UV: ");
						if (ImGui::DragFloat2("Offset", (float*)&tex->offset, 0.1f)) {
							node->Backward();
						}
						if (ImGui::DragFloat2("Tiling", (float*)&tex->tiling, 0.1f)) {
							node->Backward();
						}

						ImGui::Text("Preview: ");
						width <<= 2;
						ImGui::SetCursorPosX((ImGui::GetContentRegionAvailWidth() - width) / 2.0f);
					} else width *= node->zoomFactor;

					ButtonEx::ClampedImage(
						tex, width, true, ImVec2(0, 0), ImVec2(tex->tiling.x, tex->tiling.y)
					);
				},	/// process data
				[](MyNode* node) {
					if (!outputID(0, 0)) {
						inputVec(0, 0).setValue(256, 256, 0);
						inputVec(1, 0).x = 2.0f;
						FrameBuffer* buffer = new FrameBuffer(inputVec(0, 0).x, inputVec(0, 0).y, "noise");
						MOON_TextureManager::AddItem(buffer);
						outputID(0, 0) = buffer->ID;
					}
					FrameBuffer* tex = dynamic_cast<FrameBuffer*>(MOON_ObjectList[outputID(0, 0)]);
					ProceduralMapGenerator::GPUMapMaker(
						ProceduralMapGenerator::ProceduralMapType::Checker,
						tex, tex->offset, inputVec(1, 0).x, inputVec(1, 0).y, inputVec(1, 0).z
					);
				}
			);}},

			{ "Procedural", []() -> MyNode* { return new MyNode("Disco",
				{	/// internal data
					{ "Size",		InnerData,	 DataSizeDef(0, 1, 0), true },
					{ "Scale/Time",	InnerData,	 DataSizeDef(0, 1, 0), true }
				},
				{	/// Output slots
					{ "out",	Slot_Texture, DataSizeDef(1, 0, 0) }
				},	/// content renderer
				[](MyNode* node, bool hideInNode) {
					FrameBuffer* tex = dynamic_cast<FrameBuffer*>(MOON_ObjectList[outputID(0, 0)]);
					int width = 30;

					if (!hideInNode) {
						node->ListName();

						float availWidth = ImGui::GetContentRegionAvailWidth() / 2.5f;
						ImGui::Text("Size:"); ImGui::SetNextItemWidth(availWidth);
						if (ButtonEx::InputFloatNoLabel("width", &inputVec(0, 0).x, 100, 1000, "%.0f")) {
							tex->Reallocate(inputVec(0, 0).x, inputVec(0, 0).y);
							node->Backward();
						}
						ImGui::SameLine(); ImGui::Text(u8"¡Á"); ImGui::SameLine();
						ImGui::SetNextItemWidth(availWidth);
						if (ButtonEx::InputFloatNoLabel("height", &inputVec(0, 0).y, 100, 1000, "%.0f")) {
							tex->Reallocate(inputVec(0, 0).x, inputVec(0, 0).y);
							node->Backward();
						}
						ImGui::Separator();

						ImGui::Text("Scale: ");
						if (ButtonEx::DragFloatNoLabel("scale", &inputVec(1, 0).x, 0.1f)) {
							node->Backward();
						}

						ImGui::Text("Gamma: ");
						if (ButtonEx::DragFloatNoLabel("gamma", &inputVec(1, 0).y, 0.1f, 0.0f, 1024.0f)) {
							node->Backward();
						}

						ImGui::Text("Time: ");
						if (ButtonEx::DragFloatNoLabel("time", &inputVec(1, 0).z, 0.1f)) {
							node->Backward();
						}

						ImGui::Text("UV: ");
						if (ImGui::DragFloat2("Offset", (float*)&tex->offset, 0.1f)) {
							node->Backward();
						}
						if (ImGui::DragFloat2("Tiling", (float*)&tex->tiling, 0.1f)) {
							node->Backward();
						}

						ImGui::Text("Preview: ");
						width <<= 2;
						ImGui::SetCursorPosX((ImGui::GetContentRegionAvailWidth() - width) / 2.0f);
					} else width *= node->zoomFactor;

					ButtonEx::ClampedImage(
						tex, width, true, ImVec2(0, 0), ImVec2(tex->tiling.x, tex->tiling.y)
					);
				},	/// process data
				[](MyNode* node) {
					if (!outputID(0, 0)) {
						inputVec(0, 0).setValue(256, 256, 0);
						inputVec(1, 0).x = 1.0f;
						inputVec(1, 0).y = 2.2f;
						FrameBuffer* buffer = new FrameBuffer(inputVec(0, 0).x, inputVec(0, 0).y, "noise");
						MOON_TextureManager::AddItem(buffer);
						outputID(0, 0) = buffer->ID;
					}
					FrameBuffer* tex = dynamic_cast<FrameBuffer*>(MOON_ObjectList[outputID(0, 0)]);
					ProceduralMapGenerator::GPUMapMaker(
						ProceduralMapGenerator::ProceduralMapType::Disco,
						tex, tex->offset, inputVec(1, 0).x, inputVec(1, 0).y, inputVec(1, 0).z
					);
				}
			);}},

			{ "Procedural", []() -> MyNode* { return new MyNode("Noise", 
				{	/// internal data
					{ "Type",			InnerData,	  DataSizeDef(1, 0, 0), true },
					{ "Size",			InnerData,	  DataSizeDef(0, 1, 0), true },
					{ "Sca/Dim/Time",	InnerData,	  DataSizeDef(0, 1, 0), true }
				}, 
				{	/// Output slots
					{ "out",			Slot_Texture, DataSizeDef(1, 0, 0) }
				},	/// content renderer
				[](MyNode* node, bool hideInNode) {
					FrameBuffer* tex = dynamic_cast<FrameBuffer*>(MOON_ObjectList[outputID(0, 0)]);
					int width = 30;

					if (!hideInNode) {
						node->ListName();
						
						const static char* noiseType[] = {
							"WhiteNoise", "ValueNoise", "PerlinNoise",
							"SimplexNoise", "WorleyNoise", "VoronoiNoise",
							"TysonPolygon", "CloudNoise"
						};
						ImGui::AlignTextToFramePadding(); ImGui::Text("Noise Type: ");
						if (ButtonEx::ComboNoLabel("noiseType", &inputID(0, 0),
							noiseType, IM_ARRAYSIZE(noiseType))) node->Backward();

						float availWidth = ImGui::GetContentRegionAvailWidth() / 2.5f;
						ImGui::Text("Size:"); ImGui::SetNextItemWidth(availWidth);
						if (ButtonEx::InputFloatNoLabel("width", &inputVec(1, 0).x, 100, 1000, "%.0f")) {
							tex->Reallocate(inputVec(1, 0).x, inputVec(1, 0).y);
							node->Backward();
						}
						ImGui::SameLine(); ImGui::Text(u8"¡Á"); ImGui::SameLine();
						ImGui::SetNextItemWidth(availWidth);
						if (ButtonEx::InputFloatNoLabel("height", &inputVec(1, 0).y, 100, 1000, "%.0f")) {
							tex->Reallocate(inputVec(1, 0).x, inputVec(1, 0).y);
							node->Backward();
						}
						ImGui::Separator();

						ImGui::Text("Scale: ");
						if (ButtonEx::DragFloatNoLabel("Scale", &inputVec(2, 0).x, 0.1f)) {
							node->Backward();
						}
						ImGui::Text("Dimension: ");
						if (ButtonEx::DragFloatNoLabel("Dim", &inputVec(2, 0).y, 0.1f)) {
							node->Backward();
						}
						ImGui::Text("Time: ");
						if (ButtonEx::DragFloatNoLabel("Time", &inputVec(2, 0).z, 0.1f)) {
							node->Backward();
						}

						ImGui::Text("UV: ");
						if (ImGui::DragFloat2("Offset", (float*)&tex->offset, 0.1f)) {
							node->Backward();
						}
						if (ImGui::DragFloat2("Tiling", (float*)&tex->tiling, 0.1f)) {
							node->Backward();
						}

						ImGui::Text("Preview: ");
						width <<= 2;
						ImGui::SetCursorPosX((ImGui::GetContentRegionAvailWidth() - width) / 2.0f);
					} else width *= node->zoomFactor;

					ButtonEx::ClampedImage(
						tex, width, true, ImVec2(0, 0), ImVec2(tex->tiling.x, tex->tiling.y)
					);
				},	/// process data
				[](MyNode* node) {
					if (!outputID(0, 0)) {
						inputVec(1, 0).setValue(256, 256, 0);
						inputVec(2, 0).x = 1.0f;
						FrameBuffer* buffer = new FrameBuffer(inputVec(1, 0).x, inputVec(1, 0).y, "noise");
						MOON_TextureManager::AddItem(buffer);
						outputID(0, 0) = buffer->ID;
					}
					FrameBuffer* tex = dynamic_cast<FrameBuffer*>(MOON_ObjectList[outputID(0, 0)]);
					ProceduralMapGenerator::GPUMapMaker(
						(ProceduralMapGenerator::ProceduralMapType)inputID(0, 0),
						tex, tex->offset, inputVec(2, 0).x, inputVec(2, 0).y, inputVec(2, 0).z
					);
				}
			);}},

			{ "Procedural", []() -> MyNode* { return new MyNode("[Custom]", 
				{	/// internal data

				}, 
				{	/// Output slots
					{ "out",	Slot_Texture, DataSizeDef(1, 0, 0) }
				},	/// content renderer
				[](MyNode* node, bool hideInNode) {
					unsigned int prevID = outputID(0, 0) ? outputID(0, 0) :
						MOON_TextureManager::GetItem("moon_logo_full")->ID;
					Texture* tex = dynamic_cast<Texture*>(MOON_ObjectList[prevID]);
					int width = 30;

					if (!hideInNode) {
						node->ListName();

						ImGui::Text("Preview: ");
						width <<= 2;
					} else width *= node->zoomFactor;

					ImGui::Image((void*)(intptr_t)tex->localID, ImVec2(width, tex->height / tex->width * width));
				},	/// process data
				[](MyNode* node) {

				}
			);}},

			// Materials ---------------------------------------------------------------------
			{ "Material", []() -> MyNode* { return new MyNode("MoonMtl", 
				{	/// Input slots
					{ "Ambient",	Slot_Color, DataSizeDef(1, 0, 0) },
					{ "Diffuse",	Slot_Color, DataSizeDef(0, 1, 0) },
					{ "Specular",	Slot_Color, DataSizeDef(1, 0, 0) },
					{ "SpecExpo",	Slot_Color, DataSizeDef(1, 0, 0) },
					{ "OptiDens",	Slot_Color, DataSizeDef(1, 0, 0) },
					{ "Dissolve",	Slot_Color, DataSizeDef(1, 0, 0) },
					{ "Illumina",	Slot_Color, DataSizeDef(1, 0, 0) }
				}, 
				{	/// Output slots
					{ "MoonMtl",	Slot_Material, DataSizeDef(1, 0, 0) }
				},	/// content renderer
				[](MyNode* node, bool hideInNode) {
					if (!hideInNode) {
						if (outputID(0, 0)) {
							node->title = MOON_ObjectList[outputID(0, 0)]->name;

							// get input data
							MoonMtl* mat = ((MoonMtl*)MOON_ObjectList[outputID(0, 0)]);
							if (node->input_slots[1].parent != NULL) {
								if (mat->diffuseC != inputVec(1,0)) {
									mat->diffuseC.setValue(inputVec(1,0));
									mat->prevNeedUpdate = true;
								}
							}

							MOON_ObjectList[outputID(0, 0)]->ListProperties();
						} else {
							Material* newMat = MOON_MaterialManager::CreateMaterial("MoonMtl", "MoonMtl");
							outputID(0, 0) = newMat->ID;
						}
					}
				},	/// process data
				[](MyNode* node) {

				}
			);}},

			{ "Material", []() -> MyNode* { return new MyNode("Light", 
				{	/// Input slots
					{ "Illumination",	Slot_Texture, DataSizeDef(1, 0, 0) },
					{ "Temperature",	Slot_Texture, DataSizeDef(1, 0, 0) }
				}, {
					/// Output slots
					{ "LightMtl",	Slot_Material, DataSizeDef(1, 0, 0) }
				},	/// content renderer
				[](MyNode* node, bool hideInNode) {
					if (!hideInNode) {
						if (outputID(0, 0))
							MOON_ObjectList[outputID(0, 0)]->ListProperties();
						else {
							// TODO : Create a new mat

						}
					}
				},	/// process data
				[](MyNode* node) {

				}
			);}},

			{ "Material", []() -> MyNode* { return new MyNode("SSS", 
				{	/// Input slots
					{ "Diffuse",	Slot_Texture, DataSizeDef(1, 0, 0) },
					{ "Highlight",	Slot_Texture, DataSizeDef(1, 0, 0) },
					{ "SSS_Color",	Slot_Texture, DataSizeDef(1, 0, 0) },
					{ "Thickness",	Slot_Texture, DataSizeDef(1, 0, 0) }
				}, 
				{	/// Output slots
					{ "SSSMtl",	Slot_Material, DataSizeDef(1, 0, 0) }
				},	/// content renderer
				[](MyNode* node, bool hideInNode) {
					if (!hideInNode) {
						if (outputID(0, 0))
							MOON_ObjectList[outputID(0, 0)]->ListProperties();
						else {
							// TODO : Create a new mat

						}
					}
				},	/// process data
				[](MyNode* node) {

				}
			);}},

			{ "Material", []() -> MyNode* { return new MyNode("Volumn", 
				{	/// Input slots
					{ "BaseColor",	Slot_Texture, DataSizeDef(1, 0, 0) },
					{ "Scatter",	Slot_Texture, DataSizeDef(1, 0, 0) }
				}, 
				{	/// Output slots
					{ "VolumnMtl",	Slot_Material, DataSizeDef(1, 0, 0) }
				},	/// content renderer
				[](MyNode* node, bool hideInNode) {
					if (!hideInNode) {
						if (outputID(0, 0))
							MOON_ObjectList[outputID(0, 0)]->ListProperties();
						else {
							// TODO : Create a new mat

						}
					}
				},	/// process data
				[](MyNode* node) {

				}
			);}},

			{ "Material", []() -> MyNode* { return new MyNode("Hair", 
				{	/// Input slots
					{ "Diffuse",		Slot_Texture, DataSizeDef(1, 0, 0) },
					{ "Transmition",	Slot_Texture, DataSizeDef(1, 0, 0) }
				}, 
				{	/// Output slots
					{ "HairMtl",	Slot_Material, DataSizeDef(1, 0, 0) }
				},	/// content renderer
				[](MyNode* node, bool hideInNode) {
					if (!hideInNode) {
						if (outputID(0, 0))
							MOON_ObjectList[outputID(0, 0)]->ListProperties();
						else {
							// TODO : Create a new mat

						}
					}
				},	/// process data
				[](MyNode* node) {

				}
			);}},

			{ "Material", []() -> MyNode* { return new MyNode("Matte", 
				{	/// Input slots
					{ "AffectBackground",	Slot_Number,   DataSizeDef(1, 0) }
				}, 
				{	/// Output slots
					{ "MatteMtl",	Slot_Material, DataSizeDef(1, 0) }
				},
				/// content renderer
				[](MyNode* node, bool hideInNode) {
					if (!hideInNode) {
						if (outputID(0, 0))
							MOON_ObjectList[outputID(0, 0)]->ListProperties();
						else {
							// TODO : Create a new mat

						}
					}
				},	/// process data
				[](MyNode* node) {

				}
			);}},

			{ "Material", []() -> MyNode* { return new MyNode("Cartoon", 
				{	/// Input slots
					{ "LineColor",	Slot_Number,   DataSizeDef(1, 0) },
					{ "Thickness",	Slot_Number,   DataSizeDef(1, 0) }
				}, 
				{	/// Output slots
					{ "CartoonMtl",	Slot_Material, DataSizeDef(1, 0) }
				},
				/// content renderer
				[](MyNode* node, bool hideInNode) {
					if (!hideInNode) {
						if (outputID(0, 0))
							MOON_ObjectList[outputID(0, 0)]->ListProperties();
						else {
							// TODO : Create a new mat

						}
					}
				},	/// process data
				[](MyNode* node) {

				}
			);}},

			{ "Material", []() -> MyNode* { return new MyNode("MatMixer", 
				{	/// internal data
					{ "Incremental",  InnerData,   DataSizeDef(1, 0, 0), true },
					/// Input slots
					{ "BaseMat",	Slot_Material, DataSizeDef(1, 0, 0) },

					{ "Coat_1",		Slot_Material, DataSizeDef(1, 0, 0) },
					{ "Mask_1",		Slot_Texture,  DataSizeDef(1, 1, 0), true },
					{ "Coat_2",		Slot_Material, DataSizeDef(1, 0, 0) },
					{ "Mask_2",		Slot_Texture,  DataSizeDef(1, 1, 0), true },
					{ "Coat_3",		Slot_Material, DataSizeDef(1, 0, 0) },
					{ "Mask_3",		Slot_Texture,  DataSizeDef(1, 1, 0), true },
					{ "Coat_4",		Slot_Material, DataSizeDef(1, 0, 0) },
					{ "Mask_4",		Slot_Texture,  DataSizeDef(1, 1, 0), true },
					{ "Coat_5",		Slot_Material, DataSizeDef(1, 0, 0) },
					{ "Mask_5",		Slot_Texture,  DataSizeDef(1, 1, 0), true }
				}, 
				{	/// Output slots
					{ "Output",		Slot_Material, DataSizeDef(1, 0, 0) }
				},
				/// content renderer
				[](MyNode* node, bool hideInNode) {
					if (!hideInNode) node->ListName();

					int oldNum = (node->input_slots.size() - 2) >> 1;
					inputID(0, 0) = oldNum;

					ImGui::Text("Coat Layer: ");
					ImGui::InputInt("LayerCnt", &inputID(0, 0), 1, 1, 0, true, 1.5f);

					int diff = inputID(0, 0) - oldNum;
					if (inputID(0, 0) >= 1 && diff) {
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
						ImGui::Button(inputID(1, 0) ? MOON_ObjectList[inputID(1, 0)]->name.c_str() : "[BaseMat]",
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
				},	/// process data
				[](MyNode* node) {

				}
			);}}

		};
	}
}