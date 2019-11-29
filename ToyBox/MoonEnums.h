#pragma once

namespace MOON {
	enum Direction {
		FORWARD,
		BACKWARD,
		LEFT,
		RIGHT,
		UP,
		DOWN
	};

	enum CoordSys {
		WORLD,
		LOCAL,
		PARENT,
		SCREEN
	};

	enum class GizmoPos {
		center,
		pivot
	};

	enum class GizmoMode {
		none,
		translate,
		rotate,
		scale
	};
}