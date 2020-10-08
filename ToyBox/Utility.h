#pragma once
#include <glad/glad.h> 
#include <GLFW/glfw3.h>
#include <iostream>
#include <string>
#include <sstream>
#include <iomanip>
#include <vector>
#include <utility>
#include <map>

#include "Strutil.h"
#include "FuzzyMatch.h"
#include "STB/stb_image.h"
#include "Vector3.h"
#include "Vector4.h"
#include "Color.h"

namespace MOON {
	namespace Utility {
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
		void RemoveElemAt(std::vector<T*>& set, const int& id) {
			int cnt = 0;
			for (auto p = set.begin(); p != set.end(); p++, cnt++) {
				if (cnt == id) {
					set.erase(p);
					break;
				}
			}
		}

		template <class T>
		void RemoveElem(std::vector<T>& set, T target) {
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
		inline void safe_delete_void_ptr(void *&target) {
			if (nullptr != target) {
				T* temp = static_cast<T*>(target);
				delete temp;
				temp = nullptr;
				target = nullptr;
			}
		}

		inline std::string GetPathOrURLShortName(std::string strFullName) {
			if (strFullName.empty()) return "";

			Strutil::replace_all(strFullName, "/", "\\");
			std::string::size_type iPos = strFullName.find_last_of('\\') + 1;
			return strFullName.substr(iPos, strFullName.length() - iPos);
		}

		template <class T, typename F>
		void FuzzySearch(const char* pattern, const std::vector<T*>& matchList,
			F GetName, std::vector<T*>& container, const bool& discardNegative = true) {

			std::map<int, T*> fuzzyRes;
			for (auto lower = matchList.begin(); lower != matchList.end(); lower++) {
				if ((*lower) == nullptr) continue;
				int score = -INFINITY_INT;
				MatchTool::fuzzy_match(pattern, GetName(*lower).c_str(), score);
				if (score > 0 || !discardNegative)
					fuzzyRes.insert(std::pair<int, T*>(score, (*lower)));
			}

			for (auto it = fuzzyRes.rbegin(); it != fuzzyRes.rend(); it++)
				container.push_back((*it).second);
		}

		template <class T, typename F>
		std::vector<T> FuzzySearch(const char* pattern, const std::vector<T>& matchList,
			F GetName, const bool& discardNegative = true) {

			std::map<int, T> fuzzyRes; std::vector<T> result;
			for (auto lower = matchList.begin(); lower != matchList.end(); lower++) {
				int score = -INFINITY_INT;
				MatchTool::fuzzy_match(pattern, GetName(*lower).c_str(), score);
				if (score > 0 || !discardNegative)
					fuzzyRes.insert(std::pair<int, T>(score, (*lower)));
			}

			for (auto it = fuzzyRes.rbegin(); it != fuzzyRes.rend(); it++)
				result.push_back((*it).second);

			return result;
		}

		void FreeImageData(void* data);
		bool LoadTextureFromFile(const std::string &path, std::string &name, GLuint &textureID, int &width, int &height, GLenum& format, bool gamma = false);
		bool LoadHDRIFromFile(const std::string &path, void*& data, std::string &name, GLuint &textureID, int &width, int &height, GLenum& format, bool gamma = false);
		bool LoadTextureFromFileEx(const std::string &path, void*& data, std::string &name, GLuint &textureID, int &width, int &height, GLenum& format, bool gamma = false);
		bool LoadTextureFromArray(GLubyte* data, const int &image_width, const int &image_height, GLuint& textureID);
		bool LoadTextureFromMemory(const Vector2 &imageSize, GLubyte* imageInMem, GLuint& textureID);

		Vector2 NormalizedMousePos();
	}
}