#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "MoonEnums.h"
#include "Graphics.h"
#include "PostFactory.h"
#include "Utility.h"
#include "SceneMgr.h"
#include "HotkeyMgr.h"
#include "ObjectBase.h"
#include "OperatorBase.h"
#include "UIController.h"

namespace MOON {

#pragma region parameters
	FrameBuffer					  Graphics::ppBuffer(-1, -1, "ppBuffer", MOON_UNSPECIFIEDID, GL_RGBA16F);
	std::vector<PostEffect*>	  Graphics::postStack;
	std::vector<std::string>	  Graphics::matchList;
	bool Graphics::enablePP		= true;
	bool Graphics::showList		= false;
	bool Graphics::focusKey		= false;
	char Graphics::buf[64]		= "";
	int	 Graphics::selection	= 0;
#pragma endregion

#pragma region post_effect
	Graphics::PostProcessing::PostProcessing(const std::string& name, const std::string& shaderPath) {
		this->name = name;
		this->shader = MOON_ShaderManager::CreateShader(
			shaderPath, "ScreenBuffer.vs", (shaderPath + ".fs").c_str()
		);
	}

	Graphics::PostProcessing::PostProcessing(const std::string& name, Shader*& shader) {
		this->name = name;
		this->shader = shader;
	}

	void Graphics::SearchOps_Fuzzy(const char* typeName) {
		std::map<int, std::string> fuzzyRes;

		for (auto lower = PostFactory::effectList.begin(); lower != PostFactory::effectList.end(); lower++) {
			int score = -INFINITY_INT; std::string name = (*lower).first;
			MatchTool::fuzzy_match(typeName, name.c_str(), score);
			if (score > 0) fuzzyRes.insert(std::pair<int, std::string>(score, name));
		}

		for (auto it = fuzzyRes.rbegin(); it != fuzzyRes.rend(); it++)
			matchList.push_back((*it).second);
	}

	void Graphics::DrawPostProcessingStack() {
		static auto stackName = std::string(ICON_MD_CAMERA) + " Post-Processing Stack";
		ImGui::Checkbox("enablePostStack", &enablePP, true); ImGui::SameLine();

		// add operator to stack
		if (ImGui::Button(ICON_FA_PLUS, ImVec2(22, 22))) {
			showList = true;
			focusKey = true;
		} ImGui::SameLine();

		// show operator list
		if (showList) ImGui::SetNextItemOpen(true);
		if (ImGui::CollapsingHeader(stackName.c_str(), ImGuiTreeNodeFlags_DefaultOpen)) {
			if (showList) ListEffects();
			for (int i = 0; i < postStack.size(); i++) {
				ImGui::PushID(i);
				ImGui::Checkbox("enablePP", &postStack[i]->enabled, true); ImGui::SameLine();
				ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.192f, 0.192f, 0.192f, 1.0f));

				if (ImGui::CollapsingHeader(postStack[i]->name.c_str(), &postStack[i]->opened, ImGuiTreeNodeFlags_DefaultOpen, i)) {
					ImGui::Columns(2, "mycolumns");
					ImGui::SetColumnWidth(-1, 18);
					ImGui::NextColumn();
					// content
					postStack[i]->ListProperties();
					ImGui::Columns(1);
				}

				ImGui::PopStyleColor();
				if (!postStack[i]->opened) {
					delete postStack[i];
					Utility::RemoveElem(postStack, postStack[i]);
					ImGui::PopID();
					break;
				}
				ImGui::PopID();
			}
		}
	}

	void Graphics::ListEffects() {
		if (showList) {
			ImVec2 position = ImGui::GetCursorPos();
			ImVec2 availableSize = ImGui::GetContentRegionAvail();
			availableSize.y = 20 * std::min(10, (int)(matchList.empty() ? PostFactory::effectList.size() : matchList.size())) + 50;
			ImGui::BeginChild(
				"searchFrame",
				availableSize, true,
				ImGuiWindowFlags_NoScrollbar |
				ImGuiWindowFlags_NoScrollWithMouse
			);

			bool notEmpty = strcmp(buf, "");
			if (!notEmpty && focusKey) {
				focusKey = false;
				ImGui::SetKeyboardFocusHere();
			}
			ImGui::SetNextItemWidth(availableSize.x - (notEmpty ? 45 : 15));
			// if input changes, clear last search result and restart searching
			if (ImGui::InputTextWithHint("opSearcher", "type and search", buf, IM_ARRAYSIZE(buf))) {
				matchList.clear();
				selection = 0;
				SearchOps_Fuzzy(buf);
				availableSize.y = 20 * std::min(10, (int)(matchList.empty() ? PostFactory::effectList.size() : matchList.size())) + 50;
			}
			if (notEmpty) {
				ImGui::SameLine();
				if (ImGui::Button(ICON_FA_TIMES, ImVec2(22, 22))) {
					sprintf_s(buf, "");
					matchList.clear();
					selection = 0;
				}
			}

			// loop search result
			int maxNum = matchList.size() ? matchList.size() : PostFactory::effectList.size();
			if (MOON_KeyDown(KEY_DOWN) || MOON_KeyRepeat(KEY_DOWN)) {
				selection++;
				if (selection >= maxNum) selection = 0;
			} else if (MOON_KeyDown(KEY_UP) || MOON_KeyRepeat(KEY_UP)) {
				selection--;
				if (selection < 0) selection = maxNum - 1;
			}

			// showing search result
			if (ImGui::ListBoxHeader("opList", ImVec2(availableSize.x - 15, availableSize.y - 45))) {
				if (!matchList.empty()) {
					for (int i = 0; i < matchList.size(); i++) {
						if (selection == i && (MOON_KeyDown(KEY_DOWN) || MOON_KeyDown(KEY_UP)))
							ImGui::SetScrollHereY();

						ImGui::Selectable(matchList[i].c_str(), selection == i);
						if (ImGui::IsItemClicked() || (selection == i && MOON_KeyDown(KEY_ENTER))) {
							postStack.push_back(PostFactory::Instantiate(matchList[i]));
							matchList.clear();
							showList = false;
							sprintf_s(buf, "");
							break;
						}
					}
				} else if (notEmpty) {
					ImGui::Selectable("no result", false, ImGuiSelectableFlags_Disabled);
				} else {
					int loop = 0;
					for (auto lower = PostFactory::effectList.begin(); lower != PostFactory::effectList.end(); lower++) {
						if (selection == loop && (MOON_KeyDown(KEY_DOWN) || MOON_KeyDown(KEY_UP)))
							ImGui::SetScrollHereY();

						ImGui::Selectable((*lower).first.c_str(), selection == loop);
						if (ImGui::IsItemClicked() || (selection == loop++ && MOON_KeyDown(KEY_ENTER))) {
							postStack.push_back(PostFactory::Instantiate((*lower).first));
							showList = false;
							sprintf_s(buf, "");
						}
					}
				}
				ImGui::ListBoxFooter();
			}

			ImGui::EndChild();

			// blocking operations while mouse hovering rect
			ImVec2 rectmin = ImVec2(
				position.x + ImGui::GetWindowPos().x,
				position.y + ImGui::GetWindowPos().y - ImGui::GetScrollY()
			);
			if (!ImGui::IsMouseHoveringRect(rectmin,
				ImVec2(rectmin.x + availableSize.x, rectmin.y + availableSize.y)) &&
				ImGui::IsAnyMouseDown()) {
				showList = false;
				sprintf_s(buf, "");
				matchList.clear();
			}
		}
	}
#pragma endregion

#pragma region pipeline_implementation
	void Graphics::SetShadingMode(ShadingMode shading, LightModel model) {
		Graphics::shading = shading; Graphics::lightModel = model;
		if (shading == ShadingMode::WIRE) glPolygonMode(GL_FRONT_AND_BACK, GL_LINE);
		else glPolygonMode(GL_FRONT_AND_BACK, GL_FILL);
	}

	void Graphics::SetDrawTarget(SceneView view, const bool& depthTest = true) {
		auto buffer = MOON_TextureManager::SCENEBUFFERS[view];
		if (buffer->width == -1) return;

		glViewport(0, 0, buffer->width, buffer->height);
		//auto* cam = MOON_ActiveCamera;
		MOON_ActiveCamera = MOON_SceneCameras[view];

		Graphics::SetShadingMode(
			SceneManager::splitShading[view], 
			SceneManager::lightModel[view]
		);

		glBindFramebuffer(GL_FRAMEBUFFER, buffer->fbo);

		glEnable(GL_BLEND);
		//glEnable(GL_LINE_SMOOTH);
		if (depthTest) glEnable(GL_DEPTH_TEST);
		else glDisable(GL_DEPTH_TEST);
		//glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
	}

	void Graphics::DrawSceneView(SceneView view) {
		MOON_DrawTarget = view;
		auto buffer = MOON_TextureManager::SCENEBUFFERS[view];
		if (buffer->width == -1) return;

		glViewport(0, 0, buffer->width, buffer->height);
		auto* cam = MOON_ActiveCamera;
		MOON_ActiveCamera = MOON_SceneCameras[view];

		// draw ID LUT
		Graphics::SetShadingMode(
			SceneManager::splitShading[view],
			SceneManager::lightModel[view]
		);
		if (view == MOON_ActiveView) {
			glEnable(GL_DEPTH_TEST);
			if (MOON_TextureManager::IDLUT->width != buffer->width || MOON_TextureManager::IDLUT->height != buffer->height)
				MOON_TextureManager::IDLUT->Reallocate(buffer->width, buffer->height);
			if (MOON_ViewportState == EDIT) {
				Graphics::DrawIDLUT_EditMode();
			} else {
				Graphics::DrawIDLUT();
				MOON_InputManager::GetIDFromLUT(MOON_MousePos);
				if (!MOON_InputManager::isHoverUI && MOON_InputManager::IsMouseDown(0) && !Gizmo::hoverGizmo)
					MOON_InputManager::Select(MOON_InputManager::hoverID);
			}
		}

		// clear background of framebuffer
		glBindFramebuffer(GL_FRAMEBUFFER, buffer->fbo);
		glClearColor(Graphics::clearColor.x, Graphics::clearColor.y, Graphics::clearColor.z, Graphics::clearColor.w);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT | GL_STENCIL_BUFFER_BIT);

		// rendering objects -------------------------------------------------------
		/// enable color blend and anti-aliasing
		glEnable(GL_BLEND);
		glEnable(GL_DEPTH_TEST);
		//glEnable(GL_LINE_SMOOTH);
		//glHint(GL_LINE_SMOOTH_HINT, GL_NICEST);
		glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

		// draw objects
		Graphics::DrawSky();
		Graphics::DrawModels();
		Graphics::DrawShapes();
		Graphics::DrawHelpers();
		Graphics::DrawLights();
		Graphics::DrawCameras();

		// draw ground
		Gizmo::DrawLinePrototype(ground, Color::WHITE(), 1.0f, false);

		if (Graphics::shading == DEFWIRE) {
			Graphics::SetShadingMode(WIRE);
			//glDisable(GL_DEPTH_TEST);
			glEnable(GL_CULL_FACE);
			glCullFace(GL_BACK);
			glLineWidth(2.0f);
			Graphics::DrawModels();
			glDisable(GL_CULL_FACE);
			//glEnable(GL_DEPTH_TEST);
			Graphics::SetShadingMode(DEFWIRE);
		}

		// drawing Overlays ------------------------------------------------------------
		if (Graphics::shading != WIRE) DrawVolumnObjects(view);

		// highlight selected
		Graphics::HighlightSelection();

		// do post-processing
		if (Graphics::shading != WIRE && enablePP) ApplyPostStack(buffer);

		// draw gizmos
		// *we do not want to apply post-processing to
		glDisable(GL_DEPTH_TEST);
		if (view == MOON_ActiveView) SceneManager::DrawGizmos();

		// restore settings
		//glDisable(GL_LINE_SMOOTH);
		glDisable(GL_BLEND);
		MOON_ActiveCamera = cam;
		glViewport(0, 0, MOON_WndSize.x, MOON_WndSize.y);
	}

	void Graphics::DrawVolumnObjects(SceneView view) {
		auto buffer = MOON_TextureManager::SCENEBUFFERS[view];
		auto rayMarchingShader = MOON_ShaderManager::GetItem("RayMarching");
		rayMarchingShader->use();

		// matrix & buffers ----------------------------------------------
		Matrix4x4 view_proj_inverse = (MOON_SceneCameras[view]->projection * MOON_SceneCameras[view]->view).inverse();
		rayMarchingShader->setMat4("invPVMat", view_proj_inverse);
		rayMarchingShader->setTexture("depthBuffer", buffer->attachment, 2);

		// lights --------------------------------------------------------
		rayMarchingShader->setInt("lightNum", MOON_LightManager::CountItem());
		auto end = MOON_LightManager::itemMap.end(); unsigned int i = 0;
		for (auto it = MOON_LightManager::itemMap.begin(); it != end; it++) {
			rayMarchingShader->setVec3("lightPositions[" + std::to_string(i) + "]", it->second->transform.position);
			rayMarchingShader->setVec3("lightColors[" + std::to_string(i++) + "]", it->second->color * it->second->power);
		}

		// drawing -------------------------------------------------------
		for (auto& v : MOON_VolumeManager::itemMap) {
			if (v.second->visible) {
				rayMarchingShader->setInt("lightNum", v.second->useLight ? MOON_LightManager::CountItem() : 0);
				v.second->Draw(rayMarchingShader);

				// blit volume object to scene buffer
				Blit(buffer, rayMarchingShader);
			}
		}
	}

	void Graphics::DrawIDLUT() {
		// clear background of framebuffer
		glBindFramebuffer(GL_FRAMEBUFFER, MOON_TextureManager::IDLUT->fbo);
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// encode id to color
		for (auto &obj : MOON_ObjectList) {
			if (obj == nullptr || !obj->visible) continue;
			if (SuperClassOf(obj)._Equal("MObject")) {
				MOON_ShaderManager::lineShader->use();
				MOON_ShaderManager::lineShader->setVec4("lineColor", Color::IDEncoder(obj->ID));

				dynamic_cast<MObject*>(obj)->DrawDeliver(MOON_ShaderManager::lineShader);
			}
		}
	}

	void Graphics::DrawIDLUT_EditMode() {
		if (MOON_EditTarget == nullptr) return;

		// clear background of framebuffer
		glBindFramebuffer(GL_FRAMEBUFFER, MOON_TextureManager::IDLUT->fbo);
		glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

		// encode element id to color
		/// draw model depth first (ignore background verts)
		MOON_ShaderManager::lineShader->use();
		MOON_ShaderManager::lineShader->setVec4("lineColor", Vector4(0.0f, 0.0f, 0.0f, 1.0f));
		MOON_EditTarget->Draw(MOON_ShaderManager::lineShader);

		/// then draw point id
		auto shader = MOON_ShaderManager::GetItem("VertexID");
		shader->use();
		shader->setMat4("model", MOON_EditTarget->transform.localToWorldMat);
		shader->setMat4("view", MOON_ActiveCamera->view);
		shader->setMat4("projection", MOON_ActiveCamera->projection);

		if (CheckType(MOON_EditTarget, "Model")) {
			Model* md = dynamic_cast<Model*>(MOON_EditTarget);
			for (int i = 0, base = 0; i < md->meshList.size(); i++) {
				shader->setInt("offset", base);
				base += md->meshList[i]->vertices.size();
				glBindVertexArray(md->meshList[i]->VAO);
				glPointSize(edit_mode_point_size);
				glDrawArrays(GL_POINTS, 0, md->meshList[i]->vertices.size());
				glBindVertexArray(0);
			}
		} else if (CheckType(MOON_EditTarget, "Shape")) {
			
		}
	}

	void Graphics::DrawShadowMap() {

	}

	void Graphics::DrawSky() {
		if (MOON_Enviroment != env_pure_color && shading != WIRE) {
			MOON_ModelManager::skyDome->transform.position = MOON_ActiveCamera->transform.position;

			glDepthMask(GL_FALSE);
			auto shader = MOON_ShaderManager::GetItem("EnvSphere"); shader->use();
			shader->setTexture("HDRI", MOON_TextureManager::HDRI, 31);
			shader->setInt("type", Graphics::enviroment);
			MOON_ModelManager::skyDome->Draw(shader);
			glDepthMask(GL_TRUE);
		}
	}

	void Graphics::DrawCameras() {
		for (auto &obj : MOON_CameraManager::itemMap) {
			if (obj.second->visible) obj.second->Draw();
		}
	}

	void Graphics::DrawLights() {
		for (auto &obj : MOON_LightManager::itemMap) {
			if (obj.second->visible) obj.second->Draw();
		}
	}

	void Graphics::DrawModels() {
		/*glEnable(GL_STENCIL_TEST);
		glStencilFunc(GL_ALWAYS, 1, 0xFF);
		glStencilMask(0xFF);*/

		Shader* overrideShader = nullptr;
		if (Graphics::lightModel == LightModel::PHONG) {
			overrideShader = MOON_ShaderManager::GetItem(
				Graphics::shading == ShadingMode::FACET ? "FlatShader" : "BlinnPhong"
			);
		} else if (Graphics::shading == ShadingMode::WIRE) {
			overrideShader = MOON_ShaderManager::lineShader;
		}
		for (auto &obj : MOON_ModelManager::itemMap) {
			if (obj.second->visible) {
				if (Graphics::shading == ShadingMode::WIRE)
					MOON_ShaderManager::lineShader->setVec4("lineColor", obj.second->wireColor);
				obj.second->DrawDeliver(overrideShader);
				if (SceneManager::debug) DEBUG::DrawBBox(obj.second->bbox);
			}
		}
		if (Graphics::shading == ShadingMode::DEFWIRE) {
			for (auto &obj : MOON_ModelManager::itemMap) {
				if (obj.second->visible) {
					MOON_ShaderManager::lineShader->setVec4("lineColor", obj.second->wireColor);
					obj.second->DrawDeliver(overrideShader);
				}
			}
		}

		//glStencilFunc(GL_NOTEQUAL, 1, 0xFF);
		//glStencilMask(0x00);
		////glDisable(GL_DEPTH_TEST);
		//for (auto &obj : itemMap) {
		//	if (obj.second->visible)
		//		obj.second->Draw(ShaderManager::lineShader);
		//}
		//glStencilMask(0xFF);
		////glEnable(GL_DEPTH_TEST);
		//glDisable(GL_STENCIL_TEST);
	}

	void Graphics::DrawShapes() {
		for (auto &obj : MOON_ShapeManager::itemMap) {
			if (obj.second->visible) {
				obj.second->DrawDeliver();
				if (SceneManager::debug) obj.second->DebugVectors();
			}
		}
	}

	void Graphics::DrawHelpers() {
		for (auto &obj : MOON_HelperManager::itemMap) {
			if (obj.second->visible) obj.second->DrawDeliver();
		}
	}

	void Graphics::HighlightSelection() {
		glDisable(GL_DEPTH_TEST);
		glEnable(GL_STENCIL_TEST);
		glStencilFunc(GL_EQUAL, 0, 1);
		glStencilOp(GL_KEEP, GL_KEEP, GL_INCR);

		MOON_ShaderManager::lineShader->use();
		MOON_ShaderManager::lineShader->setVec4("lineColor", Vector4(0.8, 0.8, 0.0, 0.2));
		for (auto &id : MOON_SelectionID) { // selected
			if (SuperClassOf(MOON_ObjectList[id])._Equal("MObject")) {
				dynamic_cast<MObject*>(MOON_ObjectList[id])->DrawDeliver(MOON_ShaderManager::lineShader);
			}
		}
		if (HotKeyManager::state != EDIT) { // hovered
			MOON_ShaderManager::lineShader->setVec4("lineColor", Vector4(0.529, 0.808, 1.0, 0.25) * 0.8);
			if (MOON_InputManager::hoverID > 0) {
				dynamic_cast<MObject*>(MOON_ObjectList[MOON_InputManager::hoverID])->DrawDeliver(MOON_ShaderManager::lineShader);
			}
		}
		glDisable(GL_STENCIL_TEST);
		glEnable(GL_DEPTH_TEST);
	}
#pragma endregion

#pragma region deferred_functions
	void Graphics::ApplyPostStack(FrameBuffer*& buffer) {
		for (auto& fx : postStack) if (fx->enabled) Blit(buffer, fx);
	}
	void Graphics::Blit(FrameBuffer*& buffer, PostProcessing* renderer) {
		if (!quadVAO) ConfigureScreenQuad();

		// save states
		GLint m_viewport[4], drawFboId = 0;
		glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &drawFboId);
		glGetIntegerv(GL_VIEWPORT, m_viewport);
		auto shading = Graphics::shading;
		Graphics::SetShadingMode(DEFAULT);

		glDisable(GL_DEPTH_TEST);
		glViewport(0, 0, buffer->width, buffer->height);

		// blit frame buffer
		if (buffer->width != ppBuffer.width || buffer->height != ppBuffer.height)
			ppBuffer.Reallocate(buffer->width, buffer->height);
		glBindFramebuffer(GL_READ_FRAMEBUFFER, buffer->fbo);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, ppBuffer.fbo);
		glBlitFramebuffer(
			0, 0, buffer->width, buffer->height,
			0, 0, buffer->width, buffer->height,
			GL_COLOR_BUFFER_BIT, GL_NEAREST
		);
		if (buffer->attachment) {
			glBlitFramebuffer(
				0, 0, buffer->width, buffer->height,
				0, 0, buffer->width, buffer->height,
				GL_DEPTH_BUFFER_BIT, GL_NEAREST
			);
		}
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
		glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);

		if (!renderer->PostBehaviour(&ppBuffer, buffer)) {
			renderer->shader->use();
			renderer->ConfigureProps();
			Blit(&ppBuffer, buffer, renderer->shader);
		}

		// restore states
		Graphics::SetShadingMode(shading);
		glBindFramebuffer(GL_FRAMEBUFFER, drawFboId);
		glViewport(0, 0, m_viewport[2], m_viewport[3]);
		glEnable(GL_DEPTH_TEST);
	}

	void Graphics::Blit(FrameBuffer*& buffer, const Shader* shader) {
		if (!quadVAO) ConfigureScreenQuad();

		// save states
		GLint m_viewport[4], drawFboId = 0;// readFboId = 0;
		glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &drawFboId);
		//glGetIntegerv(GL_READ_FRAMEBUFFER_BINDING, &readFboId);
		glGetIntegerv(GL_VIEWPORT, m_viewport);
		auto shading = Graphics::shading;
		Graphics::SetShadingMode(DEFAULT);

		glDisable(GL_DEPTH_TEST);
		glViewport(0, 0, buffer->width, buffer->height);

		// blit frame buffer
		if (buffer->width != ppBuffer.width || buffer->height != ppBuffer.height)
			ppBuffer.Reallocate(buffer->width, buffer->height);
		glBindFramebuffer(GL_READ_FRAMEBUFFER, buffer->fbo);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, ppBuffer.fbo);
		//glReadBuffer(GL_COLOR_ATTACHMENT0);
		//glDrawBuffer(GL_COLOR_ATTACHMENT0);
		glBlitFramebuffer(
			0, 0, buffer->width, buffer->height,
			0, 0, buffer->width, buffer->height,
			GL_COLOR_BUFFER_BIT, GL_NEAREST
		);
		if (buffer->attachment) {
			glBlitFramebuffer(
				0, 0, buffer->width, buffer->height,
				0, 0, buffer->width, buffer->height,
				GL_DEPTH_BUFFER_BIT, GL_NEAREST
			);
		}
		//glReadBuffer(GL_NONE);
		//glDrawBuffer(GL_NONE);
		glBindFramebuffer(GL_DRAW_FRAMEBUFFER, 0);
		glBindFramebuffer(GL_READ_FRAMEBUFFER, 0);

		shader->use();
		shader->setTexture("screenBuffer", &ppBuffer, 0);
		shader->setVec2("bufferSize", buffer->width, buffer->height);

		glBindFramebuffer(GL_FRAMEBUFFER, buffer->fbo);
		glBindVertexArray(quadVAO);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		glBindVertexArray(0);

		// restore states
		Graphics::SetShadingMode(shading);
		glBindFramebuffer(GL_FRAMEBUFFER, drawFboId);
		glViewport(0, 0, m_viewport[2], m_viewport[3]);
		glEnable(GL_DEPTH_TEST);
	}

	void Graphics::ApplyPostStack(FrameBuffer*& src, FrameBuffer*& dst) {
		for (auto& fx : postStack) if (fx->enabled) Blit(src, dst, fx);
	}

	void Graphics::Blit(FrameBuffer* src, FrameBuffer*& dst, PostProcessing* renderer) {
		if (src == nullptr || dst == nullptr) {
			std::cout << "[PostProcessing]: src or dst buffer is null!" << std::endl;
			return;
		}
		if (!quadVAO) ConfigureScreenQuad();

		// execute post behaviour
		if (!renderer->PostBehaviour(src, dst)) {
			renderer->shader->use();
			renderer->ConfigureProps();
			Blit(src, dst, renderer->shader);
		}
	}

	void Graphics::Blit(FrameBuffer* src, FrameBuffer*& dst, const Shader* shader) {
		if (src == nullptr || dst == nullptr) {
			std::cout << "[PostProcessing]: src or dst buffer is null!" << std::endl;
			return;
		}
		if (!quadVAO) ConfigureScreenQuad();

		// save states
		GLint m_viewport[4], drawFboId = 0;
		glGetIntegerv(GL_DRAW_FRAMEBUFFER_BINDING, &drawFboId);
		glGetIntegerv(GL_VIEWPORT, m_viewport);

		glDisable(GL_DEPTH_TEST);
		glViewport(0, 0, dst->width, dst->height);
		auto shading = Graphics::shading;
		Graphics::SetShadingMode(DEFAULT);

		// blit frame buffer
		shader->use();
		shader->setTexture("screenBuffer", src, 0);
		shader->setVec2("bufferSize", src->width, src->height);

		glBindFramebuffer(GL_FRAMEBUFFER, dst->fbo);
		glBindVertexArray(quadVAO);
		glDrawArrays(GL_TRIANGLE_STRIP, 0, 4);
		glBindVertexArray(0);

		// restore states
		Graphics::SetShadingMode(shading);
		glBindFramebuffer(GL_FRAMEBUFFER, drawFboId);
		glViewport(0, 0, m_viewport[2], m_viewport[3]);
		glEnable(GL_DEPTH_TEST);
	}

	void Graphics::ConfigureScreenQuad() {
		glGenVertexArrays(1, &quadVAO);
		glGenBuffers(1, &quadVBO);
		glBindVertexArray(quadVAO);
		glBindBuffer(GL_ARRAY_BUFFER, quadVBO);
		glBufferData(GL_ARRAY_BUFFER, sizeof(quadVertices), &quadVertices, GL_STATIC_DRAW);
		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)0);
		glEnableVertexAttribArray(1);
		glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(GLfloat), (GLvoid*)(3 * sizeof(GLfloat)));
	}
#pragma endregion

	void Graphics::Clear() {
		Utility::ReleaseVector(postStack);
	}

}