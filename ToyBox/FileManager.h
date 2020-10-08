#pragma once
#include "Strutil.h"

namespace MOON {

	enum FILE_TYPE {
		// mesh formats
		GMSH, OBJ, FBX, BINARY, VTK,
		// texture formats
		HDRI, JPEG, PNG, BMP, TAG, EXR,
		// fallback
		UNKNOWN
	};

	class FileManager {
	public:
		static FILE_TYPE GuessFormat(const std::string file_path) {
			Strutil::to_lower(file_path);
			if (Strutil::ends_with(file_path, ".msh")) {
				return FILE_TYPE::GMSH;
			} else if (Strutil::ends_with(file_path, ".obj")) {
				return FILE_TYPE::OBJ;
			} else if (Strutil::ends_with(file_path, ".bm")) {
				return FILE_TYPE::BINARY;
			} else if (Strutil::ends_with(file_path, ".vtk")) {
				return FILE_TYPE::VTK;
			} else if (Strutil::ends_with(file_path, ".hdr")) {
				return FILE_TYPE::HDRI;
			} else if (Strutil::ends_with(file_path, ".jpg")) {
				return FILE_TYPE::JPEG;
			} else if (Strutil::ends_with(file_path, ".png")) {
				return FILE_TYPE::PNG;
			} else {
				return FILE_TYPE::UNKNOWN;
			}
		}
	};

}
