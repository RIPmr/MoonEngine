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

	extern class Model;
	class Transform {
	public:
		Vector3 position;
		Quaternion rotation;
		Vector3 scale;

		Vector3 localPosition;
		Quaternion localRotation;
		Vector3 localScale;

		Model* parent;
		std::vector<Model*> childs;

		Transform() : position(Vector3::ZERO()), localPosition(Vector3::ZERO()),
			rotation(Quaternion::identity()), localRotation(Quaternion::identity()),
			scale(Vector3::ONE()), localScale(Vector3::ONE()), parent(NULL) {}
		Transform(const Vector3 &position, const Quaternion &rotation, const Vector3 &scale) :
			position(position), rotation(rotation), scale(scale), parent(NULL) {
			localPosition = Vector3::ZERO();
			localRotation = Quaternion::identity();
			localScale = Vector3::ONE();
		}
		~Transform() {}

		void set(const Vector3* position, const Quaternion* rotation = NULL, const Vector3* scale = NULL) {
			this->position = *position;
			if (rotation != NULL) this->rotation = *rotation;
			if (scale != NULL) this->scale = *scale;
		}

		void Rotate(Quaternion q) {
			//rotation = Quaternion(Matrix4x4::Rotate(Matrix4x4(rotation), q));
			rotation = q * rotation;
		}

		Matrix4x4 GetMatrix() const;
		Vector3 forward() const;
		Vector3 right() const;
		Vector3 up() const;
	};
}