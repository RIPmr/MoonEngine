#pragma once
#include "Vector3.h"
#include "Quaternion.h"
#include "Matrix4x4.h"
#include "MathUtils.h"

#include <vector>

namespace MOON {
	extern class Model;
	class Transform {
	public:
		Vector3 position;
		Quaternion rotation;
		Vector3 scale;

		Vector3 localPosition;
		Quaternion localRotation;
		Vector3 localScale;

		Matrix4x4 modelMat;
		Matrix4x4 transformMat;
		Matrix4x4 localModelMat;

		Model* parent;
		std::vector<Model*> childs;

		bool changeFlag;

		Transform() : position(Vector3::ZERO()), localPosition(Vector3::ZERO()),
			rotation(Quaternion::identity()), localRotation(Quaternion::identity()),
			scale(Vector3::ONE()), localScale(Vector3::ONE()), changeFlag(true),
			parent(NULL), modelMat(Matrix4x4::identity()) {}

		Transform(const Vector3 &position, const Quaternion &rotation, const Vector3 &scale) :
			position(position), rotation(rotation), scale(scale), changeFlag(true),
			parent(NULL), modelMat(Matrix4x4::identity()) {
			localPosition = Vector3::ZERO();
			localRotation = Quaternion::identity();
			localScale = Vector3::ONE();
		}

		~Transform() {}

		inline void set(const Vector3* position, const Quaternion* rotation = NULL, const Vector3* scale = NULL) {
			if (position != NULL) this->position.setValue(*position);
			if (rotation != NULL) this->rotation.SetValue(*rotation);
			if (scale != NULL) this->scale.setValue(*scale);

			UpdateLocalTransform();
			UpdateMatrix();
		}

		inline void Rotate(const Quaternion &deltaQ, const CoordSys coordinate = CoordSys::WORLD) {
			if (coordinate == CoordSys::WORLD)
				rotation = deltaQ * rotation;
			else
				rotation *= deltaQ;

			UpdateLocalTransform();
			UpdateMatrix();
		}

		inline void Translate(const Vector3 &deltaV, const CoordSys coordinate = CoordSys::WORLD) {
			if (coordinate == CoordSys::WORLD)
				position += deltaV;
			else
				// TODO
				//position += forward() * deltaV.z + right() * deltaV.x + up() * deltaV.y;

			UpdateLocalTransform();
			UpdateMatrix();
		}

		inline void Scale(const Vector3 &scaleVec, const CoordSys coordinate = CoordSys::LOCAL) {
			if (coordinate == CoordSys::WORLD)
				;// TODO
			else
				scale.setValue(scaleVec);

			UpdateLocalTransform();
			UpdateMatrix();
		}

		inline Matrix4x4 UpdateMatrix() {
			// model = T * R * S * E
			//Matrix4x4 model = Matrix4x4::Scale(Matrix4x4::identity(), scale);
			modelMat = Matrix4x4::ScaleMat(scale);
			modelMat = Matrix4x4::Rotate(modelMat, rotation);
			modelMat = Matrix4x4::Translate(modelMat, position);

			transformMat = modelMat.inverse();

			return modelMat;
		}

		inline Matrix4x4 GetLocalMatrix() {
			localModelMat = Matrix4x4::ScaleMat(localScale);
			localModelMat = Matrix4x4::Rotate(localModelMat, localRotation);
			localModelMat = Matrix4x4::Translate(localModelMat, localPosition);

			return localModelMat;
		}

		inline void UpdateLocalTransform() {
			changeFlag = true;

			if (parent != NULL) {
				// TODO
			} else {
				localPosition = position;
				localRotation = rotation;
				localScale = scale;
			}
		}

		inline Vector3 forward() const {
			return Vector3(transformMat.x[0][2], transformMat.x[1][2], transformMat.x[2][2]);
		}
		inline Vector3 left() const {
			return Vector3(transformMat.x[0][0], transformMat.x[1][0], transformMat.x[2][0]);
		}
		inline Vector3 up() const {
			return Vector3(transformMat.x[0][1], transformMat.x[1][1], transformMat.x[2][1]);
		}
		inline Vector2 GetLocalAxis(const Direction &direction) const;
	};
}