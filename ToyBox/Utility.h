#pragma once
#include <glad/glad.h> 
#include <GLFW/glfw3.h>
#include <string>
#include <iostream>

#include "Vector3.h"
#include "Vector4.h"
#include "Color.h"

namespace MOON {
	inline void string_replace(std::string &strBig, const std::string &strsrc, const std::string &strdst) {
		std::string::size_type pos = 0;
		std::string::size_type srclen = strsrc.size();
		std::string::size_type dstlen = strdst.size();

		while ((pos = strBig.find(strsrc, pos)) != std::string::npos) {
			strBig.replace(pos, srclen, strdst);
			pos += dstlen;
		}
	}

	inline std::string GetPathOrURLShortName(std::string strFullName) {
		if (strFullName.empty()) return "";

		string_replace(strFullName, "/", "\\");
		std::string::size_type iPos = strFullName.find_last_of('\\') + 1;
		return strFullName.substr(iPos, strFullName.length() - iPos);
	}

	bool LoadTextureFromFile(const std::string &path, std::string &name, GLuint &textureID, int &width, int &height, bool gamma = false);
	bool LoadTextureFromArray(GLubyte* data, const int &image_width, const int &image_height, GLuint& textureID);
	bool LoadTextureFromMemory(const Vector2 &imageSize, GLubyte* imageInMem, GLuint& textureID);

	Vector2 NormalizedMousePos();

}