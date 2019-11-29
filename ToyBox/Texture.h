#pragma once
#include <string>

#include "Utility.h"
#include "ObjectBase.h"

namespace MOON {
	enum TexFormat {
		oneD,
		twoD,
		threeD,
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
			LoadTextureFromFile(_path, name, localID, width, height);
			if (!_name._Equal("FILENAME")) name = _name;
		}
		// for procedural texture
		Texture(const int &_width, const int &_height, const std::string &_name, const unsigned int &_ID = MOON_AUTOID, const TexType &_type = TexType::defaultType, const TexFormat &_format = TexFormat::twoD) :
			format(_format), type(_type), path("PROCEDURAL"), ObjectBase(_name, _ID), width(_width), height(_height), gammaCorrection(false) {
			// TODO: create a new texture and get an unique localID

		}
		
		~Texture() override {}

		void ListProperties() override {
			// list name
			ListName();

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
}