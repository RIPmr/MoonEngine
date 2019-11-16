#pragma once
#include <cmath>
#include <vector>
#include <string>

#include "ObjectBase.h"
#include "Vector3.h"

namespace moon {
	class Light : public MObject {
	public:
		std::string name;
		Vector3 position;

		Light() : position(Vector3::ZERO()){}
		Light(const Vector3 &pos) : position(pos) {}
		Light(const std::string &name, const Vector3 &pos) : position(pos), MObject(name) {}
		virtual ~Light() {}
	};

	class DirLight : public Light {
	public:
		Vector3 direction;

		Vector3 ambient;
		Vector3 diffuse;
		Vector3 specular;

		DirLight() {}
		DirLight(const Vector3 &pos, const Vector3 &dir, const Vector3 &ambi, const Vector3 &diff, const Vector3 &spec) :
			Light(pos), direction(dir), ambient(ambi), diffuse(diff), specular(spec) {}
		
		DirLight(const std::string &name, const Vector3 &pos, const Vector3 &dir, const Vector3 &ambi, const Vector3 &diff, const Vector3 &spec) :
			Light(name, pos), direction(dir), ambient(ambi), diffuse(diff), specular(spec) {}
		~DirLight() {}
	};

	class PointLight : public Light {
	public:
		float constant;
		float linear;
		float quadratic;

		Vector3 ambient;
		Vector3 diffuse;
		Vector3 specular;

		PointLight() {}
		PointLight(const Vector3 &pos, const Vector3 &ambi, const Vector3 &diff, const Vector3 &spec, const float &constant, const float &linear, const float &quadratic) :
			Light(pos), ambient(ambi), diffuse(diff), specular(spec), constant(constant), linear(linear), quadratic(quadratic) {}
		
		PointLight(const std::string &name, const Vector3 &pos, const Vector3 &ambi, const Vector3 &diff, const Vector3 &spec, const float &constant, const float &linear, const float &quadratic) :
			Light(name, pos), ambient(ambi), diffuse(diff), specular(spec), constant(constant), linear(linear), quadratic(quadratic) {}
		~PointLight() {}
	};

	class SpotLight : public Light {
	public:
		float cutOff;
		float outerCutOff;

		float constant;
		float linear;
		float quadratic;

		Vector3 direction;
		Vector3 ambient;
		Vector3 diffuse;
		Vector3 specular;

		SpotLight() {}
		SpotLight(const Vector3 &pos, const Vector3 &dir, const Vector3 &ambi, const Vector3 &diff, const Vector3 &spec,
			const float &constant, const float &linear, const float &quadratic, const float &cutOff, const float &outerCutOff) :
			Light(pos), direction(dir), ambient(ambi), diffuse(diff), specular(spec), constant(constant), linear(linear), 
			quadratic(quadratic), cutOff(cutOff), outerCutOff(outerCutOff) {}

		SpotLight(const std::string &name, const Vector3 &pos, const Vector3 &dir, const Vector3 &ambi, const Vector3 &diff, const Vector3 &spec,
			const float &constant, const float &linear, const float &quadratic, const float &cutOff, const float &outerCutOff) :
			Light(name, pos), direction(dir), ambient(ambi), diffuse(diff), specular(spec), constant(constant), linear(linear), 
			quadratic(quadratic), cutOff(cutOff), outerCutOff(outerCutOff) {}
		~SpotLight() {}
	};

	class MoonLight : public Light {
	public:

	};

	class DomeLight :public Light {
	public:

	};
}