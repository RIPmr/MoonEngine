#pragma once
#include "ObjectBase.h"

namespace moon {
	class Operator : ObjectBase {
	public:
		Operator() {}
		~Operator() {}

		virtual bool Execute() = 0;
	};
}