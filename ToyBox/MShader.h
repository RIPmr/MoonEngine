#pragma once
#include <glad/glad.h>
#include <imgui.h>

#include <string>
#include <fstream>
#include <sstream>
#include <iostream>

#include "Vector2.h"
#include "Vector3.h"
#include "Texture.h"
#include "Matrix4x4.h"
#include "ObjectBase.h"

namespace MOON {
	class Shader : public ObjectBase {
	public:
		unsigned int localID;
		std::string vertexPath;
		std::string fragmentPath;
		char vsbuf[1024 * 24];
		char fsbuf[1024 * 24];
		
		Shader(const std::string &name, const char* vertexPath, const char* fragmentPath) : ObjectBase(name, MOON_AUTOID) {
			this->vertexPath = vertexPath;
			this->fragmentPath = fragmentPath;
			Compile_And_Link(vertexPath, fragmentPath);
		}
		~Shader() override {
			glDeleteShader(localID);
		}

		// activate the shader
		// ------------------------------------------------------------------------
		void use() const {
			glUseProgram(localID);
		}

		// utility uniform functions
		// ------------------------------------------------------------------------
		void setBool(const std::string &name, bool value) const {
			glUniform1i(glGetUniformLocation(localID, name.c_str()), (int)value);
		}
		// ------------------------------------------------------------------------
		void setInt(const std::string &name, int value) const {
			glUniform1i(glGetUniformLocation(localID, name.c_str()), value);
		}
		// ------------------------------------------------------------------------
		void setTexture(const std::string &name, Texture* tex, int channel) const {
			glActiveTexture(GL_TEXTURE0 + channel);
			glBindTexture(GL_TEXTURE_2D, tex->localID);
			glUniform1i(glGetUniformLocation(localID, name.c_str()), channel);
		}
		void setTexture(const std::string &name, int texID, int channel) const {
			glActiveTexture(GL_TEXTURE0 + channel);
			glBindTexture(GL_TEXTURE_2D, texID);
			glUniform1i(glGetUniformLocation(localID, name.c_str()), channel);
		}
		// ------------------------------------------------------------------------
		void setFloat(const std::string &name, float value) const {
			glUniform1f(glGetUniformLocation(localID, name.c_str()), value);
		}
		// ------------------------------------------------------------------------
		void setVec2(const std::string &name, const Vector2 &value) const {
			//glUniform2fv(glGetUniformLocation(localID, name.c_str()), 1, &value[0]);
			glUniform2f(glGetUniformLocation(localID, name.c_str()), value.x, value.y);
		}
		void setVec2(const std::string &name, float x, float y) const {
			glUniform2f(glGetUniformLocation(localID, name.c_str()), x, y);
		}
		// ------------------------------------------------------------------------
		void setVec3(const std::string &name, const Vector3 &value) const {
			//glUniform3fv(glGetUniformLocation(localID, name.c_str()), 1, &value[0]);
			glUniform3f(glGetUniformLocation(localID, name.c_str()), value.x, value.y, value.z);
		}
		void setVec3(const std::string &name, float x, float y, float z) const {
			glUniform3f(glGetUniformLocation(localID, name.c_str()), x, y, z);
		}
		// ------------------------------------------------------------------------
		void setVec4(const std::string &name, const Vector4 &value) const {
			glUniform4f(glGetUniformLocation(localID, name.c_str()), value.x, value.y, value.z, value.w);
		}
		void setVec4(const std::string &name, const ImVec4 &value) const {
			glUniform4f(glGetUniformLocation(localID, name.c_str()), value.x, value.y, value.z, value.w);
		}
		void setVec4(const std::string &name, float x, float y, float z, float w) {
			glUniform4f(glGetUniformLocation(localID, name.c_str()), x, y, z, w);
		}
		// ------------------------------------------------------------------------
		/*void setMat2(const std::string &name, const Matrix2x2 &mat) const {
			glUniformMatrix2fv(glGetUniformLocation(localID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
		}*/
		// ------------------------------------------------------------------------
		/*void setMat3(const std::string &name, const Matrix3x3 &mat) const {
			glUniformMatrix3fv(glGetUniformLocation(localID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
		}*/
		// ------------------------------------------------------------------------
		void setMat4(const std::string &name, const Matrix4x4 &mat) const {
			glUniformMatrix4fv(glGetUniformLocation(localID, name.c_str()), 1, GL_FALSE, &mat[0][0]);
		}

		void RevertCodes();
		void ListProperties() override;

	private:
		// utility function for checking shader compilation/linking errors.
		// ------------------------------------------------------------------------
		void checkCompileErrors(unsigned int shader, std::string type) {
			int success;
			char infoLog[1024];
			if (type != "PROGRAM") {
				glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
				if (!success) {
					glGetShaderInfoLog(shader, 1024, NULL, infoLog);
					std::cout << std::endl << "[Error]: shader: " << this->name << std::endl;
					std::cout << "ERROR::SHADER_COMPILATION_ERROR of type: " << type << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
				}
			} else {
				glGetProgramiv(shader, GL_LINK_STATUS, &success);
				if (!success) {
					glGetProgramInfoLog(shader, 1024, NULL, infoLog);
					std::cout << "ERROR::PROGRAM_LINKING_ERROR of type: " << type << "\n" << infoLog << "\n -- --------------------------------------------------- -- " << std::endl;
				}
			}
		}

		void ReadShaderSource(std::string& vertexCode, std::string& fragmentCode) {
			std::ifstream vShaderFile;
			std::ifstream fShaderFile;

			// ensure ifstream objects can throw exceptions:
			vShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);
			fShaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);

			try {
				// open files
				vShaderFile.open(vertexPath);
				fShaderFile.open(fragmentPath);
				std::stringstream vShaderStream, fShaderStream;
				// read file's buffer contents into streams
				vShaderStream << vShaderFile.rdbuf();
				fShaderStream << fShaderFile.rdbuf();
				// close file handlers
				vShaderFile.close();
				fShaderFile.close();
				// convert stream into string
				vertexCode = vShaderStream.str();
				fragmentCode = fShaderStream.str();
			} catch (std::ifstream::failure e) {
				std::cout << "ERROR::SHADER::FILE_NOT_SUCCESFULLY_READ" << std::endl;
			}
		}

		// TODO
		void SaveShaders() {
			std::cout << "save test" << std::endl;
		}

		void ReCompileShaders(const char* vShaderCode, const char* fShaderCode) {
			unsigned int vertex, fragment;

			// vertex shader
			vertex = glCreateShader(GL_VERTEX_SHADER);
			glShaderSource(vertex, 1, &vShaderCode, NULL);
			glCompileShader(vertex);
			checkCompileErrors(vertex, "VERTEX");
			// fragment Shader
			fragment = glCreateShader(GL_FRAGMENT_SHADER);
			glShaderSource(fragment, 1, &fShaderCode, NULL);
			glCompileShader(fragment);
			checkCompileErrors(fragment, "FRAGMENT");
			// shader Program
			localID = glCreateProgram();
			glAttachShader(localID, vertex);
			glAttachShader(localID, fragment);
			glLinkProgram(localID);
			checkCompileErrors(localID, "PROGRAM");
			// delete the shaders as they're linked into our program now and no longer necessary
			glDeleteShader(vertex);
			glDeleteShader(fragment);
		}

		// constructor generates the shader on the fly
		void Compile_And_Link(const char* vertexPath, const char* fragmentPath) {
			std::string vertexCode;
			std::string fragmentCode;

			// 1. retrieve the vertex/fragment source code from filePath
			ReadShaderSource(vertexCode, fragmentCode);
			RevertCodes();

			// 2. compile shaders
			ReCompileShaders(vertexCode.c_str(), fragmentCode.c_str());
		}

	};
}