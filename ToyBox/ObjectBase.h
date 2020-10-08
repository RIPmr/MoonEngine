#pragma once
#include <string>
#include <vector>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_opengl3.h>

#include "Color.h"
#include "Transform.h"
#include "Icons.h"
#pragma warning(disable:4996)

#define MOON_AUTOID -1
#define MOON_UNSPECIFIEDID 0
#define MOON_FIRSTMATCH -1

#define UniquePropName(x) AppendUniquePrefix(x).c_str()
#define UniquePropNameFromParent(x) (parent->AppendUniquePrefix(x)).c_str()
#define Icon_Name_To_ID(x, y) (std::string(x) + y).c_str()

namespace MOON {
#pragma region ObjectBase
	extern class SceneManager;
	class ObjectBase {
	public:
		unsigned int ID; // ID is unique for each object
		std::string name;
		bool visible;
		bool selected;

		ObjectBase() : name("Object"), ID(MOON_UNSPECIFIEDID), visible(true), selected(false) {}
		ObjectBase(const int &_id);
		ObjectBase(const std::string &_name, const int &_id);
		virtual ~ObjectBase() {}

		friend bool operator==(const ObjectBase &o1, const ObjectBase &o2) {
			if (typeid(o1) == typeid(o2) && o1.ID == o2.ID) return true;
			else return false;
		}

		bool equal(const ObjectBase* o2) {
			// The parameters of typeid can be pointers, objects, common variables, etc
			if (typeid(*this) == typeid(*o2) && this->ID == o2->ID) return true;
			else return false;
		}

		static bool IsEqual(const ObjectBase &o1, const ObjectBase &o2) {
			if (typeid(o1) == typeid(o2) && o1.ID == o2.ID) return true;
			else return false;
		}

		static bool IsEqual(const ObjectBase *o1, const ObjectBase *o2) {
			if (typeid(*o1) == typeid(*o2) && o1->ID == o2->ID) return true;
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
#pragma endregion

#pragma region MObject
	extern class Shader;
	extern class Operator;
	class MObject : public ObjectBase {
	public:
		struct OPStack {
			bool enable;
			MObject* parent;
			MObject* deliver;
			std::vector<Operator*> opList;

			OPStack(const OPStack& other) {
				this->enable = other.enable;
				this->parent = other.parent;
				this->opList = other.opList;
				this->deliver = new MObject(*other.deliver);
			}
			OPStack(MObject* parent) : parent(parent), enable(true), deliver(nullptr) {}
			~OPStack() { ClearStack(); }

			void ExecuteAll();
			void ListStacks();
			void UpdateFrom(const int& id);

			Operator* Begin() { return opList[0]; }
			Operator* End() { return opList[opList.size() - 1]; }

			void Add(Operator* op);
			void RemoveStack(Operator* op);
			void ClearStack();

			bool IsEmpty() { 
				if (opList.size()) return true; else return false; 
			}
			bool HasDeliver() {
				if (deliver == nullptr) return false; else return true;
			}
			MObject* GetDeliver() {
				if (deliver == nullptr) return parent; else return deliver;
			}
		};

		bool freezed;
		Vector4 wireColor;
		Transform transform;
		OPStack opstack;

		MObject() : ObjectBase(MOON_AUTOID), transform(this), opstack(this), wireColor(Color::WHITE()), freezed(false) { name = "MObject_" + ID; }
		MObject(const int &_id) : 
			ObjectBase(_id), transform(this), opstack(this), wireColor(Color::WHITE()), freezed(false) { name = "MObject_" + _id; }
		MObject(const std::string &_name) : 
			ObjectBase(_name, MOON_AUTOID), transform(this), opstack(this), wireColor(Color::WHITE()), freezed(false) {}
		MObject(const std::string &_name, const int &_id) : 
			ObjectBase(_name, _id), transform(this), opstack(this), wireColor(Color::WHITE()), freezed(false) {}
		MObject(const std::string &_name, const Transform &transform, const int &_id) :
			ObjectBase(_name, _id), transform(transform), opstack(this), wireColor(Color::WHITE()), freezed(false) {}
		MObject(const MObject& other) : transform(other.transform), opstack(other.opstack) {}
		~MObject() override {}

		inline void operator=(const MObject &other) {
			this->transform = other.transform;
			this->opstack = other.opstack;
			this->wireColor = other.wireColor;
		}

		// deliver is the result which have been processed by all operators
		inline void DrawDeliver(Shader* overrideShader = NULL) {
			if (opstack.deliver == nullptr && opstack.opList.size()) opstack.ExecuteAll();
			if (opstack.deliver == nullptr) Draw(overrideShader);
			else {
				opstack.deliver->transform = this->transform;
				opstack.deliver->Draw(overrideShader);
			}
		}

		// draw base object
		virtual void Draw(Shader* overrideShader = NULL) {};
		virtual void ListName() override;
		virtual void ListTransform();
		virtual void ListProperties() override;
	};
#pragma endregion

}