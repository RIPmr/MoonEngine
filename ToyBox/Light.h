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
		Light(Vector3 pos) : position(pos) {}
		virtual ~Light() {}
	};

	class DirLight : public Light {
	public:
		Vector3 direction;

		Vector3 ambient;
		Vector3 diffuse;
		Vector3 specular;

		DirLight() {}
		DirLight(Vector3 pos, Vector3 dir, Vector3 ambi, Vector3 diff, Vector3 spec) :
			Light(pos), direction(dir), ambient(ambi), diffuse(diff), specular(spec) {}
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
		PointLight(Vector3 pos, Vector3 ambi, Vector3 diff, Vector3 spec, float constant, float linear, float quadratic) :
			Light(pos), ambient(ambi), diffuse(diff), specular(spec), constant(constant), linear(linear), quadratic(quadratic) {}
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
		SpotLight(Vector3 pos, Vector3 dir, Vector3 ambi, Vector3 diff, Vector3 spec,
			float constant, float linear, float quadratic, float cutOff, float outerCutOff) :
			Light(pos), direction(dir), ambient(ambi), diffuse(diff), specular(spec),
			constant(constant), linear(linear), quadratic(quadratic), cutOff(cutOff), outerCutOff(outerCutOff) {}
		~SpotLight() {}
	};

	class MoonLight : public Light {
	public:

	};

	class DomeLight :public Light {
	public:

	};
}