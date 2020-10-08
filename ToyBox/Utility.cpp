#include "Utility.h"
#include "SceneMgr.h"
#include "STB/stb_image.h"

namespace MOON {
	namespace Utility {
		// gives mouse pixel in NDC coordinates [-1, 1]
		Vector2 NormalizedMousePos() {
			return Vector2(MOON_MousePos.x / MOON_ScrSize.x * 2 - 1, (MOON_ScrSize.y - MOON_MousePos.y - 1) / MOON_ScrSize.y * 2 - 1);
		}

		void FreeImageData(void* data) {
			stbi_image_free(data);
			data = nullptr;
		}

		bool LoadHDRIFromFile(const std::string &path, void*& data, std::string &name, GLuint &textureID, int &width, int &height, GLenum& format, bool gamma) {
			name = GetPathOrURLShortName(path);

			//stbi_set_flip_vertically_on_load(true);
			int nrComponents;
			data = stbi_loadf(path.c_str(), &width, &height, &nrComponents, 0);
			if (data) {
				format = GL_RGBA16F;

				glGenTextures(1, &textureID);
				glBindTexture(GL_TEXTURE_2D, textureID);
				glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB16F, width, height, 0, GL_RGB, GL_FLOAT, data);

				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

				//stbi_image_free(data);
				return true;
			} else {
				std::cout << "Failed to load HDR image." << std::endl;
				return false;
			}
		}

		bool LoadTextureFromFileEx(const std::string &path, void*& data, std::string &name, GLuint &textureID, int &width, int &height, GLenum& format, bool gamma) {
			name = GetPathOrURLShortName(path);

			glGenTextures(1, &textureID);

			int nrComponents;
			data = stbi_load(path.c_str(), &width, &height, &nrComponents, 0);
			std::cout << std::endl;
			std::cout << "Load texture from: " << path << std::endl;
			std::cout << "- ID: " << textureID << ", width: " << width << ", height: " << height << ", format: " << nrComponents << ", gamma: " << gamma << std::endl;
			if (data) {
				if (nrComponents == 1)		format = GL_RED;
				else if (nrComponents == 3) format = GL_RGB;
				else if (nrComponents == 4) format = GL_RGBA;

				glBindTexture(GL_TEXTURE_2D, textureID);
				glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
				glGenerateMipmap(GL_TEXTURE_2D);

				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
				glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
				//stbi_image_free(data);
				return true;
			} else {
				std::cout << "Texture failed to load at: " << path << std::endl;
				//stbi_image_free(data);
				return false;
			}
		}

		bool LoadTextureFromFile(const std::string &path, std::string &name, GLuint &textureID, int &width, int &height, GLenum& format, bool gamma) {
			name = GetPathOrURLShortName(path);

			glGenTextures(1, &textureID);

			int nrComponents;
			unsigned char *data = stbi_load(path.c_str(), &width, &height, &nrComponents, 0);
			std::cout << std::endl;
			std::cout << "Load texture from: " << path << std::endl;
			std::cout << "- ID: " << textureID << ", width: " << width << ", height: " << height << ", format: " << nrComponents << ", gamma: " << gamma << std::endl;
			if (data) {
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

		bool LoadTextureFromArray(GLubyte* data, const int &image_width, const int &image_height, GLuint& textureID) {
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

		bool LoadTextureFromMemory(const Vector2 &imageSize, GLubyte* imageInMem, GLuint& textureID) {
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
	}
}