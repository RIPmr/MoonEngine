#pragma once
#include <string>
#include <vector>

#include "Utility.h"
#include "OperatorBase.h"
#include "PluginManager.h"

namespace MOON {
	extern struct MObject::OPStack;
	class OperatorManager {
	public:
		static Operators* available_operators;
		static std::vector<Operator*> matchList;

		static bool showList;
		static bool focusKey;
		static char buf[64];
		static int selection;
		static unsigned int parentID;

	#pragma region operator rendering & searching
		static void SearchOps_Fuzzy(const char* typeName);

		static void Instantiate(Operator* op, MObject::OPStack& opstack);

		static void ListCategory(const std::string &typeName, MObject::OPStack& opstack, unsigned int& loop);

		static void ListOperators(MObject::OPStack& opstack);
	#pragma endregion

	#pragma region load_operators
		static Operators* DefineNativeOperators();

		inline static bool LoadNativeOperators() {
			available_operators = DefineNativeOperators();
			return true;
		}

		inline static bool RemoveOperator(const std::string &type) {

			return true;
		}

		inline static bool LoadOperatorAtPath(const std::string &path) {

			return true;
		}

		inline static bool LoadExternalOperators(const std::string &path) {

			return true;
		}
	#pragma endregion
	};
}