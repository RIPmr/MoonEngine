#include "VFB.h"
#include "ButtonEx.h"
#include "Graphics.h"
#include "Renderer.h"

namespace MOON {

	bool VFB::show					= false;
	bool VFB::show_history			= false;
	bool VFB::show_postfx			= false;

	std::vector<PostEffect*>		  VFB::postStack;

	void VFB::Draw() {
		auto width = MOON_OutputSize.x < 100 ? 100 : MOON_OutputSize.x + 18;
		auto size = ImVec2(
			MOON_OutputSize.x < 100 ? 100 : MOON_OutputSize.x + 38,
			MOON_OutputSize.y < 100 ? 100 : MOON_OutputSize.y + 135
		);
		if (show_history) size.x += 290.0f;
		if (show_postfx) size.x += 300.0f;
		ImGui::SetNextWindowSize(size);

		ImGui::Begin(Icon_Name_To_ID(ICON_FA_FILM, " VFB"), &show, 
			ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoResize);

		if (show_history && show_postfx) ImGui::Columns(3, "vbf_col");
		else if (show_history || show_postfx) ImGui::Columns(2, "vbf_col");

		if (show_history) {
			ImGui::SetColumnWidth(-1, 290.0f);
			ImGui::BeginChild("vfb_history", ImVec2(280.0f, size.y - 40.0f), true);
			VFB_History();
			ImGui::EndChild();
			ImGui::NextColumn();
		}
		if (show_history || show_postfx) ImGui::SetColumnWidth(-1, width + 10.0f);
		ImGui::BeginChild("vfb_main", ImVec2(width, size.y - 40.0f), true);
		VFB_Main();
		ImGui::EndChild();
		if (show_postfx) {
			ImGui::NextColumn();
			ImGui::BeginChild("vfb_post", ImVec2(290.0f, size.y - 40.0f), true);
			VFB_PostFX();
			ImGui::EndChild();
			ImGui::Columns(1);
		}
		ImGui::End();
	}

	void VFB::VFB_Main() {
		auto rightCorner = ImGui::GetContentRegionAvailWidth() - 22;
		ButtonEx::SwitchButton(ICON_FA_CLOCK_O, ICON_FA_CLOCK_O, show_history);
		ImGui::SameLine(); ImGui::Text(u8"|"); ImGui::SameLine();
		ImGui::Button(ICON_FA_OBJECT_GROUP, ImVec2(22, 22)); ImGui::SameLine();
		ImGui::Button(ICON_FA_ADJUST, ImVec2(22, 22)); ImGui::SameLine();
		ButtonEx::SwitchButton(ICON_FA_COG, ICON_FA_COG, show_postfx);
		ImGui::SameLine(rightCorner);
		if (ImGui::Button(ICON_FA_CAMERA, ImVec2(22, 22))) {
			Renderer::StartRendering();
		}
		ImGui::Separator();

		std::string sep = Renderer::timeStamp._Equal("") ? "" : " | ";
		ImGui::Text((
			u8"%.0f ¡Á %.0f" + sep +
			Renderer::timeStamp).c_str(),
			MOON_OutputSize.x, MOON_OutputSize.y
		);

		ImVec2 pos = ImGui::GetCursorScreenPos();
		ImVec2 mousePos = ImGui::GetMousePos();
		if (Renderer::output) {
			ImGui::Image(
				(void*)(intptr_t)MOON_OutputTexID,
				ImVec2(MOON_OutputSize.x, MOON_OutputSize.y)
			);
		}

		if (ImGui::IsItemHovered() && ImGui::IsMouseDown(1)) {
			ImGui::BeginTooltip();
			float region_sz = 32.0f, zoom = 4.0f;
			Vector2 region(mousePos.x - pos.x - region_sz * 0.5f, mousePos.y - pos.y - region_sz * 0.5f);
			region.x = MoonMath::clamp(region.x, 0, MOON_OutputSize.x - region_sz);
			region.y = MoonMath::clamp(region.y, 0, MOON_OutputSize.y - region_sz);

			ImGui::Text("Min: (%.2f, %.2f)", region.x, region.y);
			ImGui::Text("Max: (%.2f, %.2f)", region.x + region_sz, region.y + region_sz);
			ImVec2 uv0 = ImVec2(region.x / MOON_OutputSize.x, region.y / MOON_OutputSize.y);
			ImVec2 uv1 = ImVec2((region.x + region_sz) / MOON_OutputSize.x, (region.y + region_sz) / MOON_OutputSize.y);
			ImGui::Image((void*)(intptr_t)MOON_OutputTexID, ImVec2(region_sz * zoom, region_sz * zoom), uv0, uv1, ImVec4(1.0f, 1.0f, 1.0f, 1.0f), ImVec4(1.0f, 1.0f, 1.0f, 0.5f));
			ImGui::EndTooltip();
		}
		if (Renderer::progress > 0 && !Renderer::prevInQueue) {
			if (ImGui::Button("Abort")) {
				Renderer::isAbort = true;
			}
			ImGui::SameLine();
			ImGui::ProgressBar(Renderer::progress);
		}
		if (Renderer::end != -1) {
			float timeInSec = (float)(Renderer::end - Renderer::start) / CLOCKS_PER_SEC;
			int timeInMin = timeInSec / 60.0f;
			if (timeInMin > 0) {
				timeInSec -= timeInMin * 60.0f;
				ImGui::Text("Total time: %d m: %.1f s", timeInMin, timeInSec);
			} else {
				ImGui::Text("Total time: %.3f s", timeInSec);
			}
		}
	}

	void VFB::VFB_History() {

	}

	void VFB::UpdateOutput() {
		glBindTexture(GL_TEXTURE_2D, MOON_OutputTexID);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, MOON_OutputSize.x,
			MOON_OutputSize.y, 0, GL_RGB, GL_FLOAT, Renderer::outputRAW);
		auto shading = Graphics::shading;
		Graphics::SetShadingMode(DEFAULT);
		for (auto& fx : postStack) {
			if (fx->enabled) Graphics::ApplyPostProcessing(Renderer::output, fx);
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
		}
		Graphics::SetShadingMode(shading);
	}

	void VFB::VFB_PostFX() {
		if (!postStack.size()) InitPostStack();

		if (ImGui::Button("Manual Update")) UpdateOutput();
		ImGui::SameLine(); ImGui::Button("Save");
		ImGui::SameLine(); ImGui::Button("Load");
		ImGui::SameLine(); ImGui::Button("Reset");
		ImGui::Separator();

		bool flag = false;
		for (auto& fx : postStack) {
			if (ButtonEx::CheckboxNoLabel(("en_" + fx->name).c_str(), &fx->enabled)) {
				flag = true;
			} ImGui::SameLine();
			if (ImGui::CollapsingHeader(fx->name.c_str())) flag |= fx->ListProperties();
		} if (flag) UpdateOutput();
	}

}