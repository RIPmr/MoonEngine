#pragma once
#include <string>

#include "ObjectBase.h"

namespace moon {
	enum TexType {
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
		TexType type;
		std::string path;
		int width;
		int height;

		Texture(const TexType &_type, const std::string & _path, const unsigned int &_width,
			const unsigned int &_height, const unsigned int &_ID) :
			type(_type), path(_path), ObjectBase(_ID), width(_width), height(_height) { }
		~Texture() {}
	};
}