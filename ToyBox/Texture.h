#pragma once
#include <string>

#include "Utility.h"
#include "ObjectBase.h"
#include "ButtonEx.h"
#include "FileManager.h"

namespace MOON {
	enum ColorSpace {
		Linear,
		sRGB,
		Gamma,
		Cineon,
		Canon_CLog,
		AlexaV3LogC,
		Panalog,
		PLogLin,
		RedLog,
		Sony_SLog,
		Sony_SLog2,
		ViperLog,
		rec709,
		rec2020,
		ACES,
		ACEScg,
		ColorSpaceLast = ACEScg
	};

	enum TexType {
		defaultType,
		ambient,
		diffuse,
		reflect,
		refract,
		glossiness,
		normal,
		height,
		displacement,
		metallic,
		translucent,
		illuminant,
		alpha,
		TexTypeLast = alpha
	};

	enum TexFilter {
		Nearest,
		Bilinear,
		Trilinear,
		Anisotropic,
		TexFilterLast = Anisotropic
	};

	class Texture : public ObjectBase {
	public:
		GLenum format;
		TexType type;
		std::string path;
		int width;
		int height;
		bool gammaCorrection;
		unsigned int localID;
		void* data;

		Texture(const std::string & _path, const std::string &_name = UseFileName, const TexType &_type = TexType::defaultType) :
				type(_type), path(_path), ObjectBase(MOON_AUTOID), gammaCorrection(false), data(nullptr) {
			if (FileManager::GuessFormat(_path) == FILE_TYPE::HDRI) 
				Utility::LoadHDRIFromFile(_path, data, name, localID, width, height, format);
			else Utility::LoadTextureFromFileEx(_path, data, name, localID, width, height, format);
			if (!_name._Equal(UseFileName)) name = _name;
		}

		// for procedural texture
		Texture(const int &_width, const int &_height, const std::string &_name, const unsigned int &_ID = MOON_AUTOID, 
			const GLenum& _format = GL_RGBA, const TexType &_type = TexType::defaultType) :
			type(_type), path(PROCEDURAL), ObjectBase(_name, _ID), width(_width), height(_height), 
			gammaCorrection(false), data(nullptr), format(_format) {
			// TODO: create a new texture and get an unique localID
		}
		
		~Texture() override {
			if (localID > 0) glDeleteTextures(1, &localID);
			if (data != nullptr) Utility::FreeImageData(data);
		}

		void Replace(const std::string & _path, const std::string &_name = UseFileName, const TexType &_type = TexType::defaultType) {
			if (localID > 0) glDeleteTextures(1, &localID);
			if (data != nullptr) Utility::FreeImageData(data);
			if (FileManager::GuessFormat(_path) == FILE_TYPE::HDRI)
				Utility::LoadHDRIFromFile(_path, data, name, localID, width, height, format);
			else Utility::LoadTextureFromFileEx(_path, data, name, localID, width, height, format);
			if (!_name._Equal(UseFileName)) name = _name; Rename(name);
		}

		void ListProperties() override {
			// list name
			ListName();
			ImGui::Text(("Size:      (" + std::to_string(width) + u8"¡Á" + std::to_string(height) + ")").c_str());

			// list preview
			ImGui::Text("Location: "); Texture* tmp = this;
			ButtonEx::FileButtonEx(
				(void**)&tmp, path.c_str(), 
				ImVec2(ImGui::GetContentRegionAvailWidth(), 0), 
				this->ID
			); ImGui::Separator();

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

		Vector3 GetPixel(int x, int y) {
			x = MoonMath::clamp(x, 0, width - 1);
			y = MoonMath::clamp(y, 0, height - 1);

			if (format == GL_RGBA16F) return Vector3(
				((float*)data)[3 * x + 3 * width * y],
				((float*)data)[3 * x + 3 * width * y + 1],
				((float*)data)[3 * x + 3 * width * y + 2]
			); else return Vector3(
				((unsigned int*)data)[3 * x + 3 * width * y] / 255.0f,
				((unsigned int*)data)[3 * x + 3 * width * y + 1] / 255.0f,
				((unsigned int*)data)[3 * x + 3 * width * y + 2] / 255.0f
			);
		}

		Vector3 SamplingColor(const Vector2& uv, const TexFilter& filter = Nearest) {
			Vector3 col;

			if (filter == Nearest) {
				int i = MoonMath::clamp(uv.x * width, 0, width - 1);
				int j = MoonMath::clamp((1.0f - uv.y) * height, 0, height - 1);
				col = GetPixel(i, j);
			} else if (filter == Bilinear) {
				double u = uv.x * width - 0.5, v = (1.0f - uv.y) * height - 0.5;
				int x = floor(u), y = floor(v);

				double u_ratio = u - x, v_ratio = v - y;
				double u_opposite = 1.0 - u_ratio, v_opposite = 1.0 - v_ratio;
				col = (GetPixel(x, y) * u_opposite + GetPixel(x + 1, y) * u_ratio) * v_opposite + 
					(GetPixel(x, y + 1) * u_opposite + GetPixel(x + 1, y + 1) * u_ratio) * v_ratio;
			} else if (filter == Trilinear) {

			} else if (filter == Anisotropic) {

			}

			return col;
		}
	};

	class FrameBuffer : public Texture {
	public:
		unsigned int rbo;
		unsigned int fbo;

		FrameBuffer(const int &_width, const int &_height, const std::string &_name, const unsigned int &_ID = MOON_AUTOID, 
			const GLenum& _format = GL_RGBA, const TexType &_type = TexType::defaultType) :
			Texture(_width, _height, _name, _ID, _format, TexType::defaultType) {
			path = FRAMEBUFFER;
			gammaCorrection = false;
			CreateFrameBuffer(_format);
		}

		~FrameBuffer() override {
			DeleteFrameBuffer();
		}

		void Reallocate(const int &_width, const int &_height) {
			DeleteFrameBuffer();
			height = _height;
			width = _width;
			CreateFrameBuffer(format);
		}

	private:
		void DeleteFrameBuffer() {
			if (localID > 0) glDeleteTextures(1, &localID);
			if (rbo > 0) glDeleteRenderbuffers(1, &rbo);
			if (fbo > 0) glDeleteFramebuffers(1, &fbo);
		}

		void CreateFrameBuffer(int format) {
			if (width < 0 || height < 0) return;
			// framebuffer configuration
			glGenFramebuffers(1, &fbo);
			glBindFramebuffer(GL_FRAMEBUFFER, fbo);
			// create a color attachment texture
			glGenTextures(1, &localID);
			glBindTexture(GL_TEXTURE_2D, localID);
			glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, GL_RGBA, 
				format == GL_RGBA ? GL_UNSIGNED_BYTE : GL_FLOAT, NULL);
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