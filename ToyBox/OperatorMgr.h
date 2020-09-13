#pragma once
#include <string>
#include <vector>
#include "OperatorBase.h"
#include "NativeOperators.h"

namespace MOON {
	class OperatorManager {
	public:
		static Operators* available_operators;

		inline static void ShowOperatorInspector() {

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

		inline static bool LoadNativeOperators() {
			available_operators = DefineNativeOperators();
			return true;
		}
	};
}