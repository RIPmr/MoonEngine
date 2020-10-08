#pragma once
#include <string>
#include <map>
#include "Graphics.h"

namespace MOON {
	typedef PostEffect*(*EffectCreator)();

	#define REGIST_POST_EFFECT(effect)                \
    PostEffect* objectCreator##effect(){			  \
        return new effect;                            \
    }                                                 \
    RegisterAction g_creatorRegister##effect(#effect, \
	(EffectCreator)objectCreator##effect)

	class PostFactory {
	public:
		static std::map<std::string, EffectCreator> effectList;
		static PostEffect* Instantiate(std::string className);
		static void RegistClass(std::string name, EffectCreator method);
	};

	class RegisterAction {
	public:
		RegisterAction(std::string className, EffectCreator ptrCreateFn);
	};

}