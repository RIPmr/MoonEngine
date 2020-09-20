#pragma once
#include "Strutil.h"

namespace MOON {

	enum FILE_TYPE {
		GMSH,
		OBJ,
		BINARY,
		VTK,
		UNKNOWN
	};

	class FileManager {
	public:
		FILE_TYPE GuessFormat(const std::string file_path) {
			Strutil::to_lower(file_path);
			if (Strutil::ends_with(file_path, ".msh")) {
				return FILE_TYPE::GMSH;
			} else if (Strutil::ends_with(file_path, ".obj")) {
				return FILE_TYPE::OBJ;
			} else if (Strutil::ends_with(file_path, ".bm")) {
				return FILE_TYPE::BINARY;
			} else if (Strutil::ends_with(file_path, ".vtk")) {
				return FILE_TYPE::VTK;
			} else {
				return FILE_TYPE::UNKNOWN;
			}
		}
	};

}
