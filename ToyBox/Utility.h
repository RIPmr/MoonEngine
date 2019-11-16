#pragma once
#include <glad/glad.h> 
//#include <GLFW/glfw3.h>
#include <string>
#include <iostream>

#include "stb_image.h"
#include "Renderer.h"

namespace moon {
	inline bool LoadTextureFromFile(const std::string &path, GLuint &textureID, int &width, int &height, bool gamma = false) {
		glGenTextures(1, &textureID);

		int nrComponents;
		unsigned char *data = stbi_load(path.c_str(), &width, &height, &nrComponents, 0);
		std::cout << "Load texture from: " << path << std::endl;
		std::cout << "ID: " << textureID << ", width: " << width << ", height: " << height << ", format: " << nrComponents << ", gamma: " << gamma << std::endl;
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

	inline bool LoadTextureFromArray(GLubyte* data, const int &image_width, const int &image_height, GLuint* out_texture) {
		// Create a OpenGL texture identifier
		GLuint image_texture;
		glGenTextures(1, &image_texture);
		glBindTexture(GL_TEXTURE_2D, image_texture);

		// Setup filtering parameters for display
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		// Upload pixels into texture
		glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, image_width, image_height, 0, GL_RGB, GL_UNSIGNED_BYTE, data);

		glBindTexture(GL_TEXTURE_2D, 0);

		*out_texture = image_texture;

		return true;
	}

	inline bool LoadTextureFromMemory(const Vector2 &imageSize, GLubyte* imageInMem, GLuint* out_texture) {
		// Create a OpenGL texture identifier
		GLuint image_texture;
		glGenTextures(1, &image_texture);
		glBindTexture(GL_TEXTURE_2D, image_texture);

		// Setup filtering parameters for display
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
		glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

		// Upload pixels into texture
		glPixelStorei(GL_UNPACK_ROW_LENGTH, 0);
		glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, (int)imageSize.x, (int)imageSize.y, 0, GL_RGB, GL_UNSIGNED_BYTE, imageInMem);

		//glBindTexture(GL_TEXTURE_2D, 0);
		*out_texture = image_texture;

		return true;
	}
}