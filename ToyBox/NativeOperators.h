#pragma once
#include <imgui.h>
#include <string>
#include <vector>

#include "ButtonEx.h"
#include "StackWindow.h"
#include "ObjectBase.h"
#include "OperatorBase.h"
#include "Icons.h"

namespace MOON {
	static Operators* DefineNativeOperators() {
		return new Operators{
			// Mesh Operators -----------------------------------------------------------------------
			{ "Model", []() -> Operator* { return new Operator("EditModel",
				/// content renderer
				[](Operator* node) -> bool {
					
					return false;
				},	/// process data
				[](MObject* input) -> MObject* {
					
				}
			); }}
		};
	}
}
