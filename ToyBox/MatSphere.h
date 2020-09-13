#pragma once
#include "Hitable.h"
#include "Material.h"
#include "Vector3.h"

#include <cmath>
#include <iostream>

namespace MOON {
	class MSphere : public Hitable {
	public:
		Vector3 pos;
		float radius;
		Material *mat;

		MSphere(const Vector3 &pos, const float &r) : pos(pos), radius(r), mat(NULL) {};
		MSphere(const Vector3 &pos, const float &r, Material *mat) : pos(pos), radius(r), mat(mat) {};

		~MSphere() {
			//if (mat) delete mat;
		}

		inline bool Hit(const Ray &r, HitRecord &rec) const {
			Vector3 oc = r.pos - this->pos;
			float dis_a = r.dir.dot(r.dir);
			float dis_b = oc.dot(r.dir);
			float dis_c = oc.dot(oc) - pow(radius, 2.0);
			float discriminant = dis_b * dis_b - dis_a * dis_c;

			if (discriminant > 0) {
				float temp = (-dis_b - sqrt(discriminant)) / dis_a;
				if (temp > EPSILON && temp < rec.t) {
					//std::cout << temp << std::endl;
					rec.t = temp;
					rec.p = r.PointAtParameter(rec.t);
					rec.normal = (rec.p - this->pos) / radius;
					rec.mat = mat;
					return true;
				}
				temp = (-dis_b + sqrt(discriminant)) / dis_a;
				if (temp > EPSILON && temp < rec.t) {
					rec.t = temp;
					rec.p = r.PointAtParameter(rec.t);
					rec.normal = (rec.p - this->pos) / radius;
					rec.mat = mat;
					return true;
				}
			}
			return false;
		}

	};
}