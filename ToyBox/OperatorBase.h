#pragma once
#include <map>
#include <string>

#include "ObjectBase.h"

namespace MOON {
	#define Operators std::multimap<std::string, Operator*(*)()>

	struct Operator {
	public:
		std::string name;
		std::string type;
		std::string path;

		bool enabled = true;
		bool selected = false;
		//bool executed = false;

		MObject* processed = nullptr;

		// if anything has been changed, return true
		bool(*content)(Operator*);

		~Operator() {
			if (processed != nullptr) {
				delete processed;
			}
		}

		MObject* Execute(MObject* object) {
			if (processed != nullptr) delete processed;
			processed = execute(object);
			return processed;
		}

		explicit Operator(const std::string& type, bool(*content)(Operator*),
			MObject*(*execute)(MObject*), const std::string& path = "native") {
			this->name = this->type = type;
			this->content = content;
			this->execute = execute;
			this->path = path;
		}

	private:
		MObject*(*execute)(MObject*);
	};

	/*class Operator : public ObjectBase {
	public:
		bool isNative;
		std::string path;

		Operator(const std::string &name) : ObjectBase(name, MOON_UNSPECIFIEDID) {}
		virtual ~Operator() = default;

		virtual bool Execute() = 0;
		virtual void ListProperties() = 0;
	};*/
}