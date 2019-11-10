#pragma once
#include <string>
#include <vector>

#include "Transform.h"

#define MOON_AUTOID -1
#define MOON_UNSPECIFIEDID -2
#define MOON_FIRSTMATCH -1

namespace moon {
	extern class SceneManager;
	class ObjectBase {
	public:
		unsigned int ID; // ID is unique in every type of objects
		std::string name;

		ObjectBase() : ID(MOON_UNSPECIFIEDID), name("") {}
		ObjectBase(const int &_id);
		ObjectBase(const std::string &_name, const int &_id);
		virtual ~ObjectBase() {}

		friend bool operator==(const ObjectBase &o1, const ObjectBase &o2) {
			if (typeid(o1) == typeid(o2) && o1.ID == o2.ID) return true;
			else return false;
		}

		bool equal(const ObjectBase* o2) {
			// The parameters of typeid can be pointers, objects, common variables, etc
			if (typeid(this) == typeid(o2) && this->ID == o2->ID) return true;
			else return false;
		}

		static bool IsEqual(const ObjectBase &o1, const ObjectBase &o2) {
			if (typeid(o1) == typeid(o2) && o1.ID == o2.ID) return true;
			else return false;
		}

		static bool IsEqual(const ObjectBase *o1, const ObjectBase *o2) {
			if (typeid(o1) == typeid(o2) && o1->ID == o2->ID) return true;
			else return false;
		}
	};

	extern class Operator;
	class MObject : public ObjectBase {
	public:
		Transform transform;

		struct OPStack {
			std::vector<Operator*> opList;

			void ExecuteAll();
			void AddStack(const Operator &op);
			void RemoveStack(const Operator &op);
			void RemoveStack(const int &opID);
			void ClearStack();
		};

		MObject() : ObjectBase("EmptyObject", MOON_UNSPECIFIEDID) {}
		MObject(const int &_id, const std::string &_name) : ObjectBase(_name, _id){}
		MObject(const int &_id, const std::string &_name, const Transform &transform) :
			ObjectBase(_name, _id), transform(transform) {}
		~MObject() {}
	};
}