#include <vector>

#include "Debugger.h"
#include "SceneMgr.h"
#include "Gizmo.h"
#include "Model.h"

namespace MOON {

	void DEBUG::DrawBBox(const Model* object, const Vector4 &color, const float &lineWidth) {
		std::vector<Vector3> corners;
		
		object->bbox_world.GetCorners(&corners);
		Line(corners[0], corners[1], color, lineWidth);
		Line(corners[1], corners[2], color, lineWidth);
		Line(corners[2], corners[3], color, lineWidth);
		Line(corners[3], corners[0], color, lineWidth);

		Line(corners[4], corners[5], color, lineWidth);
		Line(corners[5], corners[6], color, lineWidth);
		Line(corners[6], corners[7], color, lineWidth);
		Line(corners[7], corners[4], color, lineWidth);

		Line(corners[0], corners[6], color, lineWidth);
		Line(corners[5], corners[3], color, lineWidth);
		Line(corners[2], corners[4], color, lineWidth);
		Line(corners[7], corners[1], color, lineWidth);
	}

	void DEBUG::Line(const Vector3 &start, const Vector3 &end, const Vector4 &color, const float &lineWidth) {
		Gizmo::DrawLine(start, end, color, lineWidth);
	}

}