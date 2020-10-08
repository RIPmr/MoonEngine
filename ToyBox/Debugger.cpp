#include <vector>

#include "Debugger.h"
#include "SceneMgr.h"
#include "Matrix4x4.h"
#include "BoundingBox.h"
#include "Gizmo.h"

namespace MOON {

	void DEBUG::DrawBBox(const BoundingBox& bbox, const Vector4 &color, const float &lineWidth, const Matrix4x4& modelMat) {
		std::vector<Vector3> corners;
		bbox.GetCorners(&corners);

		std::vector<Vector3> drawList{
			corners[0], corners[1],
			corners[1], corners[2],
			corners[2], corners[3],
			corners[3], corners[0],

			corners[4], corners[5],
			corners[5], corners[6],
			corners[6], corners[7],
			corners[7], corners[4],

			corners[0], corners[6],
			corners[5], corners[3],
			corners[2], corners[4],
			corners[7], corners[1]
		};

		Gizmo::DrawLines(drawList, color, lineWidth, true, false, modelMat);
	}

	void DEBUG::Line(const Vector3 &start, const Vector3 &end, const Vector4 &color, const float &lineWidth) {
		Gizmo::DrawLine(start, end, color, lineWidth);
	}

}