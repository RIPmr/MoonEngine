#include "Model.h"
#include "SceneMgr.h"
#include "MoonEnums.h"

namespace MOON {

	void Model::LoadModel(const std::string& path) {
		OBJLoader::LoadFile(path, meshList, gammaCorrection);
		std::cout << "- OBJ file loaded, copying mesh list... ..." << std::endl;

		// transfer data in loader to model & building b-box
		for (auto &iter : meshList) {
			iter->parent = this;
			bbox.join(iter->bbox);
		}

		// add model to manager
		MOON_ModelManager::AddItem(this);
	}

}