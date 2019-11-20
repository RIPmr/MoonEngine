#pragma once
#include "Vector3.h"
#include "Quaternion.h"
#include "Matrix4x4.h"
#include "MathUtils.h"

#include <vector>

namespace moon {
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
		LOCAL
	};

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
		Matrix4x4 localModelMat;

		Transform* parent;
		std::vector<Model*> childs;

		Transform() : position(Vector3::ZERO()), localPosition(Vector3::ZERO()),
			rotation(Quaternion::identity()), localRotation(Quaternion::identity()),
			scale(Vector3::ONE()), localScale(Vector3::ONE()), 
			parent(NULL), modelMat(Matrix4x4::identity()) {}

		Transform(const Vector3 &position, const Quaternion &rotation, const Vector3 &scale) :
			position(position), rotation(rotation), scale(scale), 
			parent(NULL), modelMat(Matrix4x4::identity()) {
			localPosition = Vector3::ZERO();
			localRotation = Quaternion::identity();
			localScale = Vector3::ONE();
		}

		~Transform() {}

		void set(const Vector3* position, const Quaternion* rotation = NULL, const Vector3* scale = NULL) {
			if (position != NULL) this->position.setValue(*position);
			if (rotation != NULL) this->rotation.SetValue(*rotation);
			if (scale != NULL) this->scale.setValue(*scale);

			UpdateLocalTransform();
			UpdateMatrix();
		}

		void Rotate(const Quaternion &deltaQ, const CoordSys coordinate = CoordSys::WORLD) {
			if (coordinate == CoordSys::WORLD)
				rotation = deltaQ * rotation;
			else
				rotation *= deltaQ;

			UpdateLocalTransform();
			UpdateMatrix();
		}

		void Translate(const Vector3 &deltaV, const CoordSys coordinate = CoordSys::WORLD) {
			if (coordinate == CoordSys::WORLD)
				position += deltaV;
			else
				position += forward() * deltaV.z + right() * deltaV.x + up() * deltaV.y;

			UpdateLocalTransform();
			UpdateMatrix();
		}

		void Scale(const Vector3 &scaleVec, const CoordSys coordinate = CoordSys::LOCAL) {
			if (coordinate == CoordSys::WORLD)
				;// TODO
			else
				scale.setValue(scaleVec);

			UpdateLocalTransform();
			UpdateMatrix();
		}

		Matrix4x4 UpdateMatrix() {
			// model = T * R * S * E
			//Matrix4x4 model = Matrix4x4::Scale(Matrix4x4::identity(), scale);
			modelMat = Matrix4x4::ScaleMat(scale);
			modelMat = Matrix4x4::Rotate(modelMat, rotation);
			modelMat = Matrix4x4::Translate(modelMat, position);

			return modelMat;
		}

		Matrix4x4 GetLocalMatrix() {
			localModelMat = Matrix4x4::ScaleMat(localScale);
			localModelMat = Matrix4x4::Rotate(localModelMat, localRotation);
			localModelMat = Matrix4x4::Translate(localModelMat, localPosition);

			return localModelMat;
		}

		void UpdateLocalTransform() {
			if (parent != NULL) {
				// TODO
			} else {
				localPosition = position;
				localRotation = rotation;
				localScale = scale;
			}
		}

		Vector3 forward() const;
		Vector3 right() const;
		Vector3 up() const;
	};
}