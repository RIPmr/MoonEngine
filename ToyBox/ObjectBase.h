#pragma once
#include <string>
#include <vector>
#include <imgui.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include "Transform.h"
#include "IconsFontAwesome4.h"
#pragma warning(disable:4996)

#define MOON_AUTOID -1
#define MOON_UNSPECIFIEDID 0
#define MOON_FIRSTMATCH -1

#define UniquePropName(x) AppendUniquePrefix(x).c_str()
#define UniquePropNameFromParent(x) (parent->AppendUniquePrefix(x)).c_str()
#define Icon_Name_To_ID(x, y) (std::string(x) + y).c_str()

namespace MOON {
	extern class SceneManager;
	class ObjectBase {
	public:
		unsigned int ID; // ID is unique for each object
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

		virtual std::string UniquePrefix(bool skipName = true) {
			return (skipName ? "" : (name + "_")) + std::to_string(ID) + "_";
		}

		virtual std::string AppendUniquePrefix(const std::string &propName, bool skipName = true) {
			return UniquePrefix(skipName) + propName;
		}

		virtual void Rename(const std::string &newName);

		virtual void ListName();
		virtual void ListProperties();
	};

	extern class Operator;
	class MObject : public ObjectBase {
	public:
		struct OPStack {
			MObject* parent;
			bool enable;
			std::vector<Operator*> opList;

			//OPStack() : enable(true) {}
			OPStack(MObject* parent) : enable(true), parent(parent) {}
			~OPStack() { ClearStack(); }

			void ExecuteAll();
			void ListStacks();

			void AddStack(Operator* op);
			void RemoveStack(Operator* op);
			void ClearStack();
		};

		Transform transform;
		OPStack opstack;

		MObject() : ObjectBase(MOON_AUTOID), opstack(this) { name = "MObject_" + ID; }
		MObject(const int &_id) : ObjectBase(_id), opstack(this) { name = "MObject_" + _id; }
		MObject(const std::string &_name) : ObjectBase(_name, MOON_AUTOID), opstack(this) {}
		MObject(const std::string &_name, const int &_id) : ObjectBase(_name, _id), opstack(this) {}
		MObject(const std::string &_name, const Transform &transform, const int &_id) :
			ObjectBase(_name, _id), transform(transform), opstack(this) {}
		~MObject() override {}

		virtual void ListTransform();
		virtual void ListProperties() override;
	};
}