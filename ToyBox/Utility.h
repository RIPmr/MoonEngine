#pragma once
#include <glad/glad.h> 
#include <GLFW/glfw3.h>
#include <string>
#include <iostream>

#include "stb_image.h"
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

	inline bool LoadTextureFromFile(const std::string &path, std::string &name, GLuint &textureID, int &width, int &height, bool gamma = false) {
		name = GetPathOrURLShortName(path);

		glGenTextures(1, &textureID);

		int nrComponents;
		unsigned char *data = stbi_load(path.c_str(), &width, &height, &nrComponents, 0);
		std::cout << std::endl;
		std::cout << "Load texture from: " << path << std::endl;
		std::cout << "- ID: " << textureID << ", width: " << width << ", height: " << height << ", format: " << nrComponents << ", gamma: " << gamma << std::endl;
		if (data) {
			GLenum format;
			if (nrComponents == 1)
				format = GL_RED;
			else if (nrComponents == 3)
				format = GL_RGB;
			else if (nrComponents == 4)
				format = GL_RGBA;

			glBindTexture(GL_TEXTURE_2D, textureID);
			glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
			glGenerateMipmap(GL_TEXTURE_2D);

			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
			glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
			stbi_image_free(data);
			return true;
		} else {
			std::cout << "Texture failed to load at: " << path << std::endl;
			stbi_image_free(data);
			return false;
		}
	}

	inline bool LoadTextureFromArray(GLubyte* data, const int &image_width, const int &image_height, GLuint& textureID) {
		// Create a OpenGL texture identifier
		glGenTextures(1, &textureID);
		glBindTexture(GL_TEXTURE_2D, textureID);

		// Setup filtering parameters for display
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		// Upload pixels into texture
		glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image_width, image_height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);

		glBindTexture(GL_TEXTURE_2D, 0);

		return true;
	}

	inline bool LoadTextureFromMemory(const Vector2 &imageSize, GLubyte* imageInMem, GLuint& textureID) {
		// Create a OpenGL texture identifier
		glGenTextures(1, &textureID);
		glBindTexture(GL_TEXTURE_2D, textureID);

		// Setup filtering parameters for display
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		// Upload pixels into texture
		glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, (int)imageSize.x, (int)imageSize.y, 0, GL_RGB, GL_UNSIGNED_BYTE, imageInMem);

		//glBindTexture(GL_TEXTURE_2D, 0);

		return true;
	}

	void DebugLine(const Vector3 &start, const Vector3 &end, const Vector4 &color = Color::WHITE());

}