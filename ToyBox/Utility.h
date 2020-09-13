#pragma once
#include <glad/glad.h> 
#include <GLFW/glfw3.h>
#include <iostream>
#include <string>
#include <sstream>
#include <iomanip>

#include "Vector3.h"
#include "Vector4.h"
#include "Color.h"

namespace MOON {

	template <typename T>
	void Constraint(T& num, const T& min, const T& max) {
		if (num < min) num = min;
		else if (num > max) num = max;
	}

	// remove first matched target
	template <class T>
	void RemoveElem(std::vector<T*>& set, T* target) {
		for (auto p = set.begin(); p != set.end(); p++) {
			if ((*p) == target) {
				set.erase(p);
				break;
			}
		}
	}

	template <class T>
	void ReleaseVector(std::vector<T*>& set) {
		for (auto it = set.begin(); it != set.end(); it++) {
			delete *it;
		}
		set.clear();
	}

	template <typename T>
	std::string to_string_precision(const T a_value, const int precision = 6, const int width = 6) {
		std::ostringstream out;
		std::ios_base::fmtflags oldFlags = out.flags();
		out.precision(precision);
		out.setf(std::ios_base::fixed);

		out << std::setw(width) << a_value;

		out.flags(oldFlags);
		return out.str();
	}

	template <typename T>
	inline void safe_delete_void_ptr(void *&target) {
		if (nullptr != target) {
			T* temp = static_cast<T*>(target);
			delete temp;
			temp = nullptr;
			target = nullptr;
		}
	}

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