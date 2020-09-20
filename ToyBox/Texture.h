#pragma once
#include <string>

#include "Utility.h"
#include "ObjectBase.h"

namespace MOON {
	enum TexFormat {
		oneD,
		twoD,
		threeD,
		fbo,
		procedural,
		Cube,
		HDRI
	};

	enum TexType {
		defaultType,
		ambient,
		diffuse,
		specular,
		normal,
		height,
		highlight,
		alpha
	};

	class Texture : public ObjectBase {
	public:
		TexFormat format;
		TexType type;
		std::string path;
		int width;
		int height;
		bool gammaCorrection;
		unsigned int localID;

		Texture(const std::string & _path, const std::string &_name = "FILENAME", const TexType &_type = TexType::defaultType, const TexFormat &_format = TexFormat::twoD) :
				format(_format), type(_type), path(_path), ObjectBase(MOON_AUTOID), gammaCorrection(false) {
			Utility::LoadTextureFromFile(_path, name, localID, width, height);
			if (!_name._Equal("FILENAME")) name = _name;
		}
		// for procedural texture
		Texture(const int &_width, const int &_height, const std::string &_name, const unsigned int &_ID = MOON_AUTOID, const TexType &_type = TexType::defaultType, const TexFormat &_format = TexFormat::twoD) :
			format(_format), type(_type), path("[PROCEDURAL]"), ObjectBase(_name, _ID), width(_width), height(_height), gammaCorrection(false) {
			// TODO: create a new texture and get an unique localID
		}
		
		~Texture() override {
			if (localID > 0) glDeleteTextures(1, &localID);
		}

		void ListProperties() override {
			// list name
			ListName();
			ImGui::Text(("Size:      (" + std::to_string(width) + u8"¡Á" + std::to_string(height) + ")").c_str());

			// list preview
			ImGui::Text("Location: ");
			ImGui::Button(path.c_str(), ImVec2(ImGui::GetContentRegionAvailWidth(), 0));

			ImGui::Separator();

			float maxPrevWidth = 124.0f;
			ImGui::Text("Preview: "); ImGui::SameLine();
			ImGui::SetNextItemWidth(ImGui::GetContentRegionAvailWidth() - 30.0f);
			ImGui::SliderFloat("size", &maxPrevWidth, 50.0f, 200.0f);
			float prevWidth = width > maxPrevWidth ? maxPrevWidth : width;
			float centering = (ImGui::GetContentRegionAvailWidth() - prevWidth) / 2.0f;
			ImGui::Indent(centering);
			ImGui::Image((void*)(intptr_t)localID, ImVec2(prevWidth, height * prevWidth / width));
			ImGui::Unindent(centering);

			ImGui::Spacing();
		}
	};

	class FrameBuffer : public Texture {
	public:
		unsigned int rbo;
		unsigned int fbo;

		FrameBuffer(const int &_width, const int &_height, const std::string &_name, const unsigned int &_ID = MOON_AUTOID, const TexType &_type = TexType::defaultType, const TexFormat &_format = TexFormat::twoD) :
			Texture(_width, _height, _name, _ID, TexType::defaultType, TexFormat::twoD) {
			path = "[FBO]";
			gammaCorrection = false;
			CreateFrameBuffer();
		}

		~FrameBuffer() override {
			DeleteFrameBuffer();
		}

		void Reallocate(const int &_width, const int &_height) {
			DeleteFrameBuffer();
			this->height = _height;
			this->width = _width;
			CreateFrameBuffer();
		}

	private:
		void DeleteFrameBuffer() {
			if (localID > 0) glDeleteTextures(1, &localID);
			if (rbo > 0) glDeleteRenderbuffers(1, &rbo);
			if (fbo > 0) glDeleteFramebuffers(1, &fbo);
		}

		void CreateFrameBuffer() {
			// framebuffer configuration
			glGenFramebuffers(1, &fbo);
			glBindFramebuffer(GL_FRAMEBUFFER, fbo);
			// create a color attachment texture
			glGenTextures(1, &localID);
			glBindTexture(GL_TEXTURE_2D, localID);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, localID, 0);
			// create a renderbuffer object for depth and stencil attachment (we won't be sampling these)
			glGenRenderbuffers(1, &rbo);
			glBindRenderbuffer(GL_RENDERBUFFER, rbo);
			// use a single renderbuffer object for both a depth AND stencil buffer.
			glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);
			// now actually attach it
			glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);
			// now that we actually created the framebuffer and added all attachments we want to check if it is actually complete now
			if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
				std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
		}
	};
}