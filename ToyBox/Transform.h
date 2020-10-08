#pragma once
#include "Vector3.h"
#include "Quaternion.h"
#include "Matrix4x4.h"
#include "MathUtils.h"
#include "Utility.h"
#include "properties.h"

#include <vector>

namespace MOON {
	extern class MObject;
	class Transform {
	private:
		Vector3 _position;
		Quaternion _rotation;
		Vector3 _scale;

		Vector3 deltaSca;
		Quaternion deltaRot;
		Matrix4x4 parentMat;

		Matrix4x4 nativeMat;

	public:
		#pragma region propties
		PROPERTY(Vector3, position);
		GET(position) { 
			if (parent != nullptr) 
				return (parent->localToWorldMat * parentMat).multVec(_position);
			else return _position; 
		}
		SET(position) {
			if (parent != nullptr)
				_position = (parentMat.inverse() * parent->worldToLocalMat).multVec(value);
			else _position = value;
			UpdateMatrix();
		}
		PROPERTY(Vector3, localPosition);
		GET(localPosition) {
			if (parent != nullptr)
				return _position - parent->position;
			else return _position;
		}
		SET(localPosition) {
			if (parent != nullptr)
				_position = value + parent->position;
			else _position = value;
			UpdateMatrix();
		}

		PROPERTY(Quaternion, rotation);
		GET(rotation) { 
			if (parent != nullptr) 
				return parent->rotation * deltaRot * _rotation;
			else return _rotation;
		}
		SET(rotation) {
			if (parent != nullptr)
				_rotation = (parent->rotation * deltaRot).Inverse() * value;
			else _rotation = value;
			UpdateMatrix();
		}
		PROPERTY(Quaternion, localRotation);
		GET(localRotation) {
			if (parent != nullptr) return deltaRot * _rotation;
			else return _rotation;
		}
		SET(localRotation) {
			if (parent != nullptr) _rotation = value / deltaRot;
			else _rotation = value;
			UpdateMatrix();
		}

		PROPERTY(Vector3, scale);
		GET(scale) { 
			if (parent != nullptr) {
				Matrix4x4 M(localToWorldMat);
				for (int i = 0; i < 3; ++i) M[i][3] = M[3][i] = 0; M[3][3] = 1;
				Matrix4x4 sca = Matrix4x4::Rotate(M, rotation.Inverse());
				return Vector3(sca[0][0], sca[1][1], sca[2][2]);
			} else return _scale;
		}
		SET(scale) {
			if (parent != nullptr) {
				Matrix4x4 sca; sca[0][0] = value.x; sca[1][1] = value.y; sca[2][2] = value.z;
				Matrix4x4 M = (parent->localToWorldMat * parentMat).inverse() * Matrix4x4::Rotate(sca, rotation);
				for (int i = 0; i < 3; ++i) M[i][3] = M[3][i] = 0; M[3][3] = 1;
				Matrix4x4 res = Matrix4x4::Rotate(localToWorldMat, _rotation.Inverse());
				_scale.setValue(sca[0][0], sca[1][1], sca[2][2]);
			} else _scale = value;
			UpdateMatrix();
		}
		PROPERTY(Vector3, localScale);
		GET(localScale) {
			if (parent != nullptr) return _scale / deltaSca;
			else return _scale;
		}
		SET(localScale) {
			if (parent != nullptr) _scale = value * deltaSca;
			else _scale = value;
			UpdateMatrix();
		}
		#pragma endregion

		Matrix4x4 localToWorldMat; // model matrix
		Matrix4x4 worldToLocalMat; // transform matrix (Axis)

		MObject* mobject;

		Transform* parent;
		std::vector<Transform*> childs;

		bool changeFlag;

		Transform(MObject* mobject) : _position(Vector3::ZERO()),
			_rotation(Quaternion::identity()), _scale(Vector3::ONE()), changeFlag(true),
			parent(nullptr), localToWorldMat(Matrix4x4::identity()), mobject(mobject) {}

		inline void operator=(const Transform &other) {
			this->_position = const_cast<Transform&>(other).position;
			this->_rotation = const_cast<Transform&>(other).rotation;
			this->_scale = const_cast<Transform&>(other).scale;
			this->localToWorldMat = other.localToWorldMat;
			this->worldToLocalMat = other.worldToLocalMat;
			this->parent = other.parent;
			//this->childs = other.childs;
			this->changeFlag = true;
			//this->mobject = nullptr;
		}

		Transform(const Vector3 &position, const Quaternion &rotation, const Vector3 &scale) :
			_position(position), _rotation(rotation), _scale(scale), changeFlag(true),
			parent(NULL), localToWorldMat(Matrix4x4::identity()) {
			UpdateChildTransform();
			UpdateMatrix();
		}

		~Transform() {
			if (parent != nullptr) parent->RemoveChild(this);

			// TODO: delete child objects

		}

		inline void SetParent(Transform* parent) {
			if (parent == this->parent) return;
			if (this->parent != nullptr) {
				_scale = scale;
				_rotation = rotation;
				_position = position;
			}

			if (parent != nullptr) {
				auto tmp = parent;
				while (tmp->parent != nullptr) {
					if (tmp->parent == this) {
						tmp->SetParent(this->parent);
						break;
					} else tmp = tmp->parent;
				}
			}

			if (this->parent != nullptr) this->parent->RemoveChild(this);
			this->parent = parent;
			if (parent != nullptr){
				deltaRot = parent->rotation.Inverse();
				deltaSca = parent->scale;
				parentMat = parent->worldToLocalMat;
				parent->AddChild(this);
			}
			UpdateMatrix();
		}
		inline void AddChild(Transform* child) {
			this->childs.push_back(child);
		}
		inline void RemoveChild(Transform* child) {
			Utility::RemoveElem(this->childs, child);
		}

		inline void Rotate(const Quaternion &deltaQ, const CoordSys coordinate = CoordSys::WORLD) {
			if (coordinate == CoordSys::WORLD) {
				if (parent != nullptr) {
					//auto div = parent->rotation * deltaRot;
					//_rotation = div.Inverse() * deltaQ * div * _rotation;
					rotation = deltaQ * rotation;
				} else 
					_rotation = deltaQ * _rotation;
			} else {
				_rotation = deltaQ * _rotation;
			}

			UpdateMatrix();
		}

		inline void Translate(const Vector3 &deltaV, const CoordSys coordinate = CoordSys::WORLD) {
			if (coordinate == WORLD) {
				position += deltaV;
			} else {
				// TODO
			}

			UpdateMatrix();
		}

		inline void Scale(const Vector3 &scaleVec, const CoordSys coordinate = CoordSys::LOCAL) {
			if (coordinate == CoordSys::WORLD) {
				// TODO
			} else {
				_scale.setValue(scaleVec);
			}

			UpdateMatrix();
		}

		inline void UpdateMatrix() {
			// model = T * R * S * E
			localToWorldMat = Matrix4x4::ScaleMat(_scale);
			localToWorldMat = Matrix4x4::Rotate(localToWorldMat, _rotation);
			localToWorldMat = Matrix4x4::Translate(localToWorldMat, _position);
			nativeMat = localToWorldMat.inverse();

			if (parent != nullptr)
				localToWorldMat = parent->localToWorldMat * parentMat * localToWorldMat;

			worldToLocalMat = localToWorldMat.inverse();
			changeFlag = true;
			UpdateChildTransform();
		}

		inline void UpdateChildTransform() {
			for (auto child : childs) child->UpdateMatrix();
		}

		inline Vector3 forward() const {
			return Vector3::Normalize(Vector3(worldToLocalMat.x[0][2], worldToLocalMat.x[1][2], worldToLocalMat.x[2][2]));
		}
		inline Vector3 left() const {
			return Vector3::Normalize(Vector3(worldToLocalMat.x[0][0], worldToLocalMat.x[1][0], worldToLocalMat.x[2][0]));
		}
		inline Vector3 up() const {
			return Vector3::Normalize(Vector3(worldToLocalMat.x[0][1], worldToLocalMat.x[1][1], worldToLocalMat.x[2][1]));
		}
		inline Vector3 GetLocalAxis(const Direction &direction) const {
			switch (direction) {
				case Direction::UP:			return up();
				case Direction::LEFT:		return left();
				case Direction::FORWARD:	return forward();

				case Direction::DOWN:		return -up();
				case Direction::RIGHT:		return -left();
				case Direction::BACKWARD:	return -forward();

				default:					return Vector3::ZERO();
			}
		}
		inline int AxisSign(const Direction &dir) const {
			if (dir == Direction::UP && _scale.y < 0) return -1;
			else if (dir == Direction::LEFT && _scale.x < 0)  return - 1;
			else if (dir == Direction::FORWARD && _scale.z < 0)  return - 1;
			return 1;
		}

		// * for child object
		inline Vector3 GetNativeAxis(const Direction &direction) const {
			if (parent == nullptr) return GetLocalAxis(direction);
			switch (direction) {
				case Direction::UP:			return Vector3::Normalize(Vector3(nativeMat.x[0][1], nativeMat.x[1][1], nativeMat.x[2][1]));
				case Direction::LEFT:		return Vector3::Normalize(Vector3(nativeMat.x[0][0], nativeMat.x[1][0], nativeMat.x[2][0]));
				case Direction::FORWARD:	return Vector3::Normalize(Vector3(nativeMat.x[0][2], nativeMat.x[1][2], nativeMat.x[2][2]));

				default:					return Vector3::ZERO();
			}
		}
	};
}