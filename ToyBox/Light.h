#pragma once
#include <vector>
#include <string>

#include "ObjectBase.h"
#include "MathUtils.h"
#include "Texture.h"
#include "Vector3.h"
#include "Color.h"

namespace MOON {
	enum LightType {
		directional_light,
		point_light,
		spot_light,
		moon_light,
		dome_light
	};

	class Light : public MObject {
	public:
		Vector3 color;
		float power;

		Light() { wireColor = Color::YELLOW(); }
		Light(const Vector3 &_pos, const Vector4 &_color, const float &_power = 1.0f) : color(_color), power(_power) { 
			wireColor = Color::YELLOW(); 
			transform.position = _pos;
		}
		Light(const std::string &name, const Vector3 &_pos, const Vector4 &_color, const float &_power = 1.0f) :
			color(_color), power(_power), MObject(name) { 
			wireColor = Color::YELLOW();
			transform.position = _pos;
		}
		virtual ~Light() override { }

		virtual void Draw(Shader* overrideShader = NULL) override;
		virtual void ListProperties() override;
		virtual void ListLightProperties();
		virtual std::vector<Vector3>& GetLightShape();
	};

	class DirLight : public Light {
	public:
		Vector3 direction;

		DirLight() {}
		DirLight(const Vector3 &pos, const Vector3 &dir, const Vector4 &color, const float &power = 1.0f) :
			Light(pos, color, power), direction(dir) {}
		
		DirLight(const std::string &name, const Vector3 &pos, const Vector3 &dir, const Vector4 &color, const float &power = 1.0f) :
			Light(name, pos, color, power), direction(dir) {}
		~DirLight() override {}

		std::vector<Vector3>& GetLightShape() override;
		void ListLightProperties() override;
	};

	class PointLight : public Light {
	public:
		// F_attenuation = 1.0 / (K_c + K_l * d + K_q * d * d)
		float constant;
		float linear;
		float quadratic;

		PointLight() {}
		PointLight(const Vector3 &pos, const Vector4 &color, const float &power = 30.0f, 
			const float &constant = 1.0f, const float &linear = 0.09f, const float &quadratic = 0.032) :
			Light(pos, color, power), constant(constant), linear(linear), quadratic(quadratic) {}
		
		PointLight(const std::string &name, const Vector3 &pos, const Vector4 &color, const float &power = 30.0f,
			const float &constant = 1.0f, const float &linear = 0.09f, const float &quadratic = 0.032) :
			Light(name, pos, color, power), constant(constant), linear(linear), quadratic(quadratic) {}
		~PointLight() override {}

		std::vector<Vector3>& GetLightShape() override;
		void ListLightProperties() override;
	};

	class SpotLight : public Light {
	public:
		float cutOffRadiance;
		float outerCutOffRadiance;

		float constant;
		float linear;
		float quadratic;

		Vector3 direction;

		SpotLight() {}
		SpotLight(const Vector3 &pos, const Vector3 &dir, const Vector4 &color, const float &power = 1.0f, 
			const float &cutOff = 30.0f * Deg2Rad, const float &outerCutOff = 45.0f * Deg2Rad, 
			const float &constant = 1.0f, const float &linear = 0.09f, const float &quadratic = 0.032) :
			Light(pos, color, power), direction(dir), constant(constant), linear(linear),
			quadratic(quadratic), cutOffRadiance(cutOff), outerCutOffRadiance(outerCutOff) {}

		SpotLight(const std::string &name, const Vector3 &pos, const Vector3 &dir, const Vector4 &color, const float &power = 1.0f, 
			const float &cutOff = 30.0f * Deg2Rad, const float &outerCutOff = 45.0f * Deg2Rad, 
			const float &constant = 1.0f, const float &linear = 0.09f) :
			Light(name, pos, color, power), direction(dir), constant(constant), linear(linear),
			quadratic(quadratic), cutOffRadiance(cutOff), outerCutOffRadiance(outerCutOff) {}
		~SpotLight() override {}

		std::vector<Vector3>& GetLightShape() override;
		void ListLightProperties() override;
	};

	class MoonLight : public Light {
	public:
		Vector2 size;

		MoonLight() {}
		~MoonLight() override {}

		std::vector<Vector3>& GetLightShape() override;
		void ListLightProperties() override;
	};

	class DomeLight :public Light {
	public:
		Texture* HDRI;

		DomeLight() {}
		~DomeLight() override {}

		std::vector<Vector3>& GetLightShape() override;
		void ListLightProperties() override;
	};
}