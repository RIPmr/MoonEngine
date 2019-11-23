#pragma once
#include <string>

namespace moon {
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