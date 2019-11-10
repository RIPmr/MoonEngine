#pragma once
#include "Transform.h"
#include "Mesh.h"
#include "Material.h"
#include "Hitable.h"
#include "ObjectBase.h"
#include "OBJLoader.h"

#include <vector>
#include <string>

namespace moon {
	class Model : public Hitable, public MObject {
	public:
		std::vector<Mesh> meshList;
		std::string directory;
		bool gammaCorrection;

		Model(const std::string &name, const std::string &path, const int id = MOON_AUTOID, const bool gamma = false) :
			gammaCorrection(gamma), MObject(id, name){
			LoadModel(path);
		}
		Model(const std::string &path, const int id = MOON_AUTOID, const bool gamma = false) :
			gammaCorrection(gamma), MObject(id, "Model") {
			LoadModel(path);
		}
		~Model() {}

		void Draw(Shader shader) {
			for (int i = 0; i < meshList.size(); i++)
				meshList[i].Draw(dynamic_cast<MoonMtl*>(meshList[i].material)->shader);
		}

		void LoadModel(std::string const &path) {
			
		}

	};
}