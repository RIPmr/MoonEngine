#include <utility>
#include "PostFactory.h"

namespace MOON {

	PostEffect* PostFactory::Instantiate(std::string className) {
		auto iter = effectList.find(className);
		if (iter == effectList.end()) return NULL;
		else return iter->second();
	}

	void PostFactory::RegistClass(std::string name, EffectCreator method) {
		effectList.insert(std::pair<std::string, EffectCreator>(name, method));
	}

	RegisterAction::RegisterAction(std::string className, EffectCreator ptrCreateFn) {
		PostFactory::RegistClass(className, ptrCreateFn);
	}

}