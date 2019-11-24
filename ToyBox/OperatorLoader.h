#pragma once
#include <string>

namespace MOON {
	class OPLoader {
	public:
		OPLoader() = default;
		~OPLoader() = default;

		inline static bool LoadAllPlugins(const std::string &path) {
			// TODO

			return true;
		}
	};
}