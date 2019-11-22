#pragma once
#include <string>
#include <vector>
#include <imgui.h>

#include "Transform.h"

#define MOON_AUTOID -1
#define MOON_UNSPECIFIEDID 0
#define MOON_FIRSTMATCH -1

namespace moon {
	extern class SceneManager;
	class ObjectBase {
	public:
		unsigned int ID; // ID is unique in every type of objects
		std::string name;
		bool visible;

		ObjectBase() : name("Object"), ID(MOON_UNSPECIFIEDID), visible(true) {}
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

		virtual void Rename(const std::string &newName);

		virtual void ListName();
		virtual void ListProperties();
	};

	extern class Operator;
	class MObject : public ObjectBase {
	public:
		struct OPStack {
			bool enable;
			std::vector<Operator*> opList;

			OPStack() : enable(true) {}
			~OPStack() { ClearStack(); }

			void ExecuteAll();
			void ListStacks();

			void AddStack(Operator* op);
			void RemoveStack(Operator* op);
			void ClearStack();
		};

		Transform transform;
		OPStack opstack;

		MObject() : ObjectBase(MOON_AUTOID) { name = "MObject_" + ID; }
		MObject(const int &_id) : ObjectBase(_id) { name = "MObject_" + _id; }
		MObject(const std::string &_name) : ObjectBase(_name, MOON_AUTOID) {}
		MObject(const std::string &_name, const int &_id) : ObjectBase(_name, _id){}
		MObject(const std::string &_name, const Transform &transform, const int &_id) :
			ObjectBase(_name, _id), transform(transform) {}
		~MObject() override {}

		virtual void ListTransform();
		virtual void ListProperties() override;
	};
}