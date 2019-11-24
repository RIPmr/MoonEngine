#pragma once
#include <string>

#include "ObjectBase.h"

namespace MOON {
	class Operator : public ObjectBase {
	public:
		Operator(const std::string &name) : ObjectBase(name, MOON_UNSPECIFIEDID) {}
		virtual ~Operator() = default;

		virtual bool Execute() = 0;
		virtual void ListProperties() = 0;
	};
}