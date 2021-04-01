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
		ACEScg, ColorSpaceLast = ACEScg
	};

	enum TexFilter {
		Nearest,
		Bilinear,
		Trilinear,
		Anisotropic
	};

	class Texture : public ObjectBase {
	public:
		std::string path;
		ColorSpace colorSpace;

		GLenum format;
		GLenum dataType;
		GLenum warpMode;
		GLenum filter;
		bool mipmap;

		Vector2 offset;
		Vector2 tiling;

		int width;
		int height;
		unsigned int localID;
		void* data;

		Texture(const std::string & _path, const std::string &_name = UseFileName, 
				const ColorSpace& _colorSpace = sRGB, const bool& _mipmap = true) :
				path(_path), ObjectBase(MOON_AUTOID), colorSpace(_colorSpace), mipmap(_mipmap), 
				data(nullptr), localID(0), offset(Vector2::ZERO()), tiling(Vector2::ONE()), dataType(GL_UNSIGNED_BYTE) {
			if (FileManager::GuessFormat(_path) == FILE_TYPE::HDRI) {
				Utility::LoadHDRIFromFile(_path, data, name, localID, width, height, format, mipmap);
				dataType = GL_FLOAT; colorSpace = Linear;
			} else Utility::LoadTextureFromFileEx(_path, data, name, localID, width, height, format, mipmap);
			if (!_name._Equal(UseFileName)) name = _name;
		}

		// for procedural texture
		Texture(const int &_width, const int &_height, const std::string &_name, const unsigned int &_ID = MOON_AUTOID, 
			const GLenum& _format = GL_RGBA, const GLenum& _dataType = GL_UNSIGNED_BYTE,
			const ColorSpace& _colorSpace = Linear, const bool& _mipmap = false) :
			path(PROCEDURAL), ObjectBase(_name, _ID), width(_width), height(_height), 
			colorSpace(_colorSpace), mipmap(_mipmap), data(nullptr), format(_format), dataType(_dataType), localID(0),
			offset(Vector2::ZERO()), tiling(Vector2::ONE()){}
		
		~Texture() override {
			if (localID > 0) glDeleteTextures(1, &localID);
			if (data != nullptr) Utility::FreeImageData(data);
		}

		void Replace(const std::string & _path, const std::string &_name = UseFileName) {
			this->path = _path;
			if (localID > 0) glDeleteTextures(1, &localID);
			if (data != nullptr) Utility::FreeImageData(data);
			if (FileManager::GuessFormat(_path) == FILE_TYPE::HDRI)
				Utility::LoadHDRIFromFile(_path, data, name, localID, width, height, format);
			else Utility::LoadTextureFromFileEx(_path, data, name, localID, width, height, format);
			if (!_name._Equal(UseFileName)) name = _name; Rename(name);
		}

		virtual void Reimport() {
			// update texture parameter
			// TODO

		}

		virtual void ListProperties() override {
			// list name
			ListName();
			ImGui::Text(("Size:      (" + std::to_string(width) + u8"×" + std::to_string(height) + ")").c_str());

			// list preview
			ImGui::Text("Location: "); Texture* tmp = this;
			ButtonEx::FileButtonEx(
				(void**)&tmp, path.c_str(), 
				ImVec2(ImGui::GetContentRegionAvailWidth(), 0), 
				this->ID
			); 
			ImGui::Separator();

			// list parameters
			if (ImGui::CollapsingHeader("Advanced Options", 0, ID)) {
				ImGui::Indent(10.0f);

				ImGui::Text("Mipmap:"); ImGui::SameLine(80.0f);
				if (ButtonEx::CheckboxNoLabel("mipmap", &mipmap)) Reimport();

				const static char* colorSpace[] = {
					"linear", "sRGB", "Gamma", "Cineon", "Canon_CLog",
					"AlexaV3LogC", "Panalog", "PLogLin", "RedLog",
					"Sony_SLog", "Sony_SLog2", "ViperLog", "rec709",
					"rec2020", "ACES", "ACEScg"
				};
				ImGui::AlignTextToFramePadding(); ImGui::Text("Color Space: ");
				if (ButtonEx::ComboNoLabel("colorSpace", (int*)&this->colorSpace,
					colorSpace, IM_ARRAYSIZE(colorSpace))) Reimport();

				const static char* warpMode[] = {
					"REPEAT", "MIRRORED_REPEAT", "CLAMP_TO_EDGE", "CLAMP_TO_BORDER"
				};
				ImGui::AlignTextToFramePadding(); ImGui::Text("Warp Mode: ");
				int warp = 0; if (this->warpMode == GL_REPEAT) warp = 0;
				else if (this->warpMode == GL_MIRRORED_REPEAT) warp = 1;
				else if (this->warpMode == GL_CLAMP_TO_EDGE) warp = 2;
				else if (this->warpMode == GL_CLAMP_TO_BORDER) warp = 3;
				if (ButtonEx::ComboNoLabel("warpMode", &warp,
					warpMode, IM_ARRAYSIZE(warpMode))) {
					if (warp == 0) this->warpMode = GL_REPEAT;
					else if (warp == 1) this->warpMode = GL_MIRRORED_REPEAT;
					else if (warp == 2) this->warpMode = GL_CLAMP_TO_EDGE;
					else if (warp == 3) this->warpMode = GL_CLAMP_TO_BORDER;
					Reimport();
				}

				const static char* filter[] = {
					"Bilinear", "Nearest", "Trilinear" //, "Anistropic"
				};
				ImGui::AlignTextToFramePadding(); ImGui::Text("Filter: ");
				int filt = 0; if (this->filter == GL_LINEAR) filt = 0;
				else if (this->filter == GL_NEAREST) filt = 1;
				if (ButtonEx::ComboNoLabel("filter", &filt,
					filter, IM_ARRAYSIZE(filter))) {
					if (filt == 0) this->filter = GL_LINEAR;
					else if (filt == 1) this->filter = GL_NEAREST;
					Reimport();
				}

				ImGui::Text("UV: ");
				if (ImGui::DragFloat2("Offset", (float*)&offset, 0.1f)) {
					Reimport();
				}
				if (ImGui::DragFloat2("Tiling", (float*)&tiling, 0.1f)) {
					Reimport();
				}
				ImGui::Unindent(10.0f);
			}
			ImGui::Separator();

			// list preview
			float maxPrevWidth = 124.0f;
			ImGui::Text("Preview: "); ImGui::SameLine();
			ImGui::SetNextItemWidth(ImGui::GetContentRegionAvailWidth() - 30.0f);
			ImGui::SliderFloat("size", &maxPrevWidth, 50.0f, 200.0f);
			float prevWidth = width > maxPrevWidth ? maxPrevWidth : width;
			float offset = (ImGui::GetContentRegionAvailWidth() - prevWidth) / 2.0f;
			ImGui::SetCursorPosX(offset);
			ButtonEx::ClampedImage(
				this, prevWidth + 15, true,
				ImVec2(this->offset.x, this->offset.y),
				ImVec2(this->offset.x + this->tiling.x, this->offset.y + this->tiling.y)
			);

			ImGui::Spacing();
		}

		void GetData() {
			glReadPixels(
				0, 0, width, height, 
				format, dataType, data
			);
		}

		Vector3 GetPixel(int x, int y) {
			x = MoonMath::clamp(x, 0, width - 1);
			y = MoonMath::clamp(y, 0, height - 1);

			if (dataType == GL_FLOAT) return Vector3(
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
		unsigned int fbo;
		unsigned int rbo;
		unsigned int attachment;

		FrameBuffer(const int &_width, const int &_height, const std::string &_name, const unsigned int &_ID = MOON_AUTOID, 
			const GLenum& _format = GL_RGBA, const GLenum& _dataType = GL_UNSIGNED_BYTE,
			const ColorSpace& _colorSpace = Linear, const bool& _mipmap = false) :
			Texture(_width, _height, _name, _ID, _format, _dataType, _colorSpace, _mipmap),
			rbo(0), fbo(0), attachment(0) {
			path = FRAMEBUFFER;
			CreateFrameBuffer();
		}

		FrameBuffer(const Vector2 &size, const std::string &_name, const unsigned int &_ID = MOON_AUTOID,
			const GLenum& _format = GL_RGBA, const GLenum& _dataType = GL_UNSIGNED_BYTE,
			const ColorSpace& _colorSpace = Linear, const bool& _mipmap = false) :
			Texture(size.x, size.y, _name, _ID, _format, _dataType, _colorSpace, _mipmap),
			rbo(0), fbo(0), attachment(0) {
			path = FRAMEBUFFER;
			CreateFrameBuffer();
		}

		virtual ~FrameBuffer() override {
			DeleteFrameBuffer();
		}

		virtual void Reallocate(const int &_width, const int &_height) {
			DeleteFrameBuffer();
			height = _height;
			width = _width;
			CreateFrameBuffer();
		}

		void ListProperties() override {
			// list name
			ListName();

			float availWidth = ImGui::GetContentRegionAvailWidth() / 2.5f;
			ImGui::Text("Frame Buffer Size:");
			ImGui::SetNextItemWidth(availWidth);
			if (ButtonEx::InputIntNoLabel("width", &width, 100, 1000, 0)) {
				Reallocate(width, height);
			} 
			ImGui::SameLine(); ImGui::Text(u8"×"); ImGui::SameLine();
			ImGui::SetNextItemWidth(availWidth);
			if (ButtonEx::InputIntNoLabel("height", &height, 100, 1000, 0)) {
				Reallocate(width, height);
			}
			ImGui::Separator();

			// list parameters
			if (ImGui::CollapsingHeader("Advanced Options", 0, ID)) {
				ImGui::Indent(10.0f);

				ImGui::Text("Mipmap:"); ImGui::SameLine(80.0f);
				if (ButtonEx::CheckboxNoLabel("mipmap", &mipmap)) Reimport();

				const static char* colorSpace[] = {
					"linear", "sRGB", "Gamma", "Cineon", "Canon_CLog",
					"AlexaV3LogC", "Panalog", "PLogLin", "RedLog",
					"Sony_SLog", "Sony_SLog2", "ViperLog", "rec709",
					"rec2020", "ACES", "ACEScg"
				};
				ImGui::AlignTextToFramePadding(); ImGui::Text("Color Space: ");
				if (ButtonEx::ComboNoLabel("colorSpace", (int*)&this->colorSpace,
					colorSpace, IM_ARRAYSIZE(colorSpace))) Reimport();

				const static char* warpMode[] = {
					"REPEAT", "MIRRORED_REPEAT", "CLAMP_TO_EDGE", "CLAMP_TO_BORDER"
				};
				ImGui::AlignTextToFramePadding(); ImGui::Text("Warp Mode: ");
				int warp = 0; if (this->warpMode == GL_REPEAT) warp = 0;
				else if (this->warpMode == GL_MIRRORED_REPEAT) warp = 1;
				else if (this->warpMode == GL_CLAMP_TO_EDGE) warp = 2;
				else if (this->warpMode == GL_CLAMP_TO_BORDER) warp = 3;
				if (ButtonEx::ComboNoLabel("warpMode", &warp,
					warpMode, IM_ARRAYSIZE(warpMode))) {
					if (warp == 0) this->warpMode = GL_REPEAT;
					else if (warp == 1) this->warpMode = GL_MIRRORED_REPEAT;
					else if (warp == 2) this->warpMode = GL_CLAMP_TO_EDGE;
					else if (warp == 3) this->warpMode = GL_CLAMP_TO_BORDER;
					Reimport();
				}

				const static char* filter[] = {
					"Bilinear", "Nearest", "Trilinear" //, "Anistropic"
				};
				ImGui::AlignTextToFramePadding(); ImGui::Text("Filter: ");
				int filt = 0; if (this->filter == GL_LINEAR) filt = 0;
				else if (this->filter == GL_NEAREST) filt = 1;
				if (ButtonEx::ComboNoLabel("filter", &filt,
					filter, IM_ARRAYSIZE(filter))) {
					if (filt == 0) this->filter = GL_LINEAR;
					else if (filt == 1) this->filter = GL_NEAREST;
					Reimport();
				}

				ImGui::Text("UV: ");
				if (ImGui::DragFloat2("Offset", (float*)&offset, 0.1f)) {
					Reimport();
				}
				if (ImGui::DragFloat2("Tiling", (float*)&tiling, 0.1f)) {
					Reimport();
				}
				ImGui::Unindent(10.0f);
			}
			ImGui::Separator();

			// list preview
			float maxPrevWidth = 124.0f;
			ImGui::Text("Preview: "); ImGui::SameLine();
			ImGui::SetNextItemWidth(ImGui::GetContentRegionAvailWidth() - 30.0f);
			ImGui::SliderFloat("size", &maxPrevWidth, 50.0f, 200.0f);
			float prevWidth = width > maxPrevWidth ? maxPrevWidth : width;
			float offset = (ImGui::GetContentRegionAvailWidth() - prevWidth) / 2.0f;
			ImGui::SetCursorPosX(offset);
			ButtonEx::ClampedImage(
				this, prevWidth + 15, true,
				ImVec2(this->offset.x, this->offset.y),
				ImVec2(this->offset.x + this->tiling.x, this->offset.y + this->tiling.y)
			);

			if (attachment) {
				ImGui::Text("Depth/Stencil: ");
				ImGui::SetCursorPosX(offset);
				ButtonEx::ClampedImage(
					attachment, Vector2(width, height), prevWidth + 15, 
					true, ImVec2(this->offset.x, this->offset.y),
					ImVec2(this->offset.x + this->tiling.x, this->offset.y + this->tiling.y)
				);
			}

			ImGui::Spacing();
		}

	protected:
		virtual void DeleteFrameBuffer() {
			if (attachment > 0) glDeleteTextures(1, &attachment);
			if (localID > 0) glDeleteTextures(1, &localID);
			if (rbo > 0) glDeleteRenderbuffers(1, &rbo);
			if (fbo > 0) glDeleteFramebuffers(1, &fbo);
		}

		virtual void CreateFrameBuffer() {
			if (width < 0 || height < 0) return;

			// framebuffer configuration
			glGenFramebuffers(1, &fbo);
			glBindFramebuffer(GL_FRAMEBUFFER, fbo);

			// create a color attachment texture
			glGenTextures(1, &localID);
			glBindTexture(GL_TEXTURE_2D, localID);
			glTexImage2D(
				GL_TEXTURE_2D, 0, format, width, height, 
				0, format == GL_RGBA ? GL_RGBA : GL_RGB, dataType, NULL
			);

			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, mipmap ? GL_LINEAR_MIPMAP_LINEAR : GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

			if (mipmap) {
				// set minimum/maximum mipmap level
				//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_BASE_LEVEL, 0);
				//glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAX_LEVEL, 4);
				glGenerateMipmap(GL_TEXTURE_2D);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_R, GL_CLAMP_TO_EDGE);
			}

			// Anisotropic texture filtering
			//GLfloat flargest;
			//glGetFloatv(GL_MAX_TEXTURE_MAX_ANISCTROPY_EXT, &fLargest);
			// 设置纹理参数(各向异性采样)，数量为1.0表示各向同性采样
			//glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, flargest);

			// attach texture to frame buffer object, last parameter indicates mipmap level
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, localID, 0);

			// create a renderbuffer object for depth and stencil attachment (we won't be sampling these)
			// use a single renderbuffer object for both a depth AND stencil buffer.
			//glGenRenderbuffers(1, &rbo);
			//glBindRenderbuffer(GL_RENDERBUFFER, rbo);
			//glRenderbufferStorage(GL_RENDERBUFFER, GL_DEPTH24_STENCIL8, width, height);

			// generate depth attachment
			glGenTextures(1, &attachment);
			glBindTexture(GL_TEXTURE_2D, attachment);
			/*glTexImage2D(
				GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT,
				width, height, 0, GL_DEPTH_COMPONENT,
				GL_FLOAT, NULL
			);*/

			glTexImage2D(
				GL_TEXTURE_2D, 0, GL_DEPTH24_STENCIL8, 
				width, height, 0, GL_DEPTH_STENCIL, 
				GL_UNSIGNED_INT_24_8, NULL
			);

			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

			// generate stencil attachment
			// *OpenGL 4.4+ only
			/*glGenTextures(1, &stencilAtt);
			glBindTexture(GL_TEXTURE_2D, stencilAtt);
			glTexImage2D(
				GL_TEXTURE_2D, 0, GL_STENCIL_INDEX8,
				width, height, 0, GL_STENCIL_INDEX,
				GL_UNSIGNED_BYTE, NULL
			);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);*/

			// now actually attach it
			//glFramebufferRenderbuffer(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_RENDERBUFFER, rbo);
			//glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, attachment, 0);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_STENCIL_ATTACHMENT, GL_TEXTURE_2D, attachment, 0);
			
			// now that we actually created the framebuffer and added all attachments we want to check if it is actually complete now
			if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE)
				std::cout << "ERROR::FRAMEBUFFER:: Framebuffer is not complete!" << std::endl;
			
			glBindFramebuffer(GL_FRAMEBUFFER, 0);
		}
	};

	// TODO
	class GBuffer : public FrameBuffer {
	public:
		unsigned int gPosition;
		unsigned int gNormal;
		unsigned int gAlbedoSpec;

	private:
		virtual void DeleteFrameBuffer() override {
			FrameBuffer::DeleteFrameBuffer();
			if (gPosition > 0) glDeleteTextures(1, &gPosition);
			if (gNormal > 0) glDeleteTextures(1, &gNormal);
			if (gAlbedoSpec > 0) glDeleteTextures(1, &gAlbedoSpec);
		}

		virtual void CreateFrameBuffer() override {
			if (width < 0 || height < 0) return;

			GLuint gBuffer;
			glGenFramebuffers(1, &gBuffer);
			glBindFramebuffer(GL_FRAMEBUFFER, gBuffer);
			GLuint gPosition, gNormal, gColorSpec;

			// position buffer
			glGenTextures(1, &gPosition);
			glBindTexture(GL_TEXTURE_2D, gPosition);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, NULL);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT0, GL_TEXTURE_2D, gPosition, 0);

			// normal buffer
			glGenTextures(1, &gNormal);
			glBindTexture(GL_TEXTURE_2D, gNormal);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, NULL);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT1, GL_TEXTURE_2D, gNormal, 0);

			// color + specular buffer
			glGenTextures(1, &gAlbedoSpec);
			glBindTexture(GL_TEXTURE_2D, gAlbedoSpec);
			glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, width, height, 0, GL_RGBA, GL_FLOAT, NULL);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
			glFramebufferTexture2D(GL_FRAMEBUFFER, GL_COLOR_ATTACHMENT2, GL_TEXTURE_2D, gAlbedoSpec, 0);

			// 告诉OpenGL我们将要使用(帧缓冲的)哪种颜色附件来进行渲染
			GLuint attachments[3] = { GL_COLOR_ATTACHMENT0, GL_COLOR_ATTACHMENT1, GL_COLOR_ATTACHMENT2 };
			glDrawBuffers(3, attachments);

			// 之后同样添加渲染缓冲对象(Render Buffer Object)为深度缓冲(Depth Buffer)，并检查完整性
			// TODO
		}
	};

}

/*
*NOTE:
为纹理对象生成一组完整的mipmap:
void glGenerateMipmap (GLenum target);
描述：glGenerateMipmap计算从零级数组派生的一组完整的mipmap数组。
无论先前的内容如何，最多包括1x1维度纹理图像的数组级别都将替换为派生数组。零级纹理图像保持不变（原图）。
派生的mipmap数组的内部格式都与零级纹理图像的内部格式相匹配。
通过将零级纹理图像的宽度和高度减半来计算派生数组的尺寸，然后将每个阵列级别的尺寸减半，直到达到1x1尺寸纹理图像。
*/