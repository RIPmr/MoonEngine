#pragma once
#include <cmath>
#include <vector>
#include <iostream>
#include "Vector3.h"

namespace MOON {
	namespace Curves {
		class CubicBezier {
		public:
			static bool ComputeCurve(const std::vector<Vector3> &knots,
				const std::vector<Vector3> &controllers, const int &smooth,
				std::vector<Vector3> &result) {
				if (controllers.size() != (knots.size() << 1) - 2) {
					std::cout << "Bezier: controllers.size(" << controllers.size ()
						<< ") != knots.size(" << knots.size() << ") << 1 - 2" << std::endl;
					return false;
				}
				float step = 1.0f / smooth;
				for (int i = 0, ctrl = 0; i < knots.size() - 1; i++) {
					for (float u = 0; u < 1; u += step) {
						if (u >= 1) break;
						float uu = 1 - u;
						result.push_back(
							pow(uu, 3) * knots[i] +
							3 * u * pow(uu, 2) * controllers[ctrl] +
							3 * pow(u, 2) * uu * controllers[ctrl + 1] +
							pow(u, 3) * knots[i + 1]
						);
					}
					ctrl += 2;
				}
				result.push_back(knots[knots.size() - 1]);
				return true;
			}
		};

		class BSpline {
		public:
			static bool ComputeCurve(const std::vector<Vector3> &knots, const int &p,
				const bool &closed, const bool &uniform, std::vector<Vector3> &result) {
				if (closed) {
					std::vector<Vector3> cknots(knots);
					for (int i = 0; i < p + 1; i++) cknots.push_back(knots[i]);
					std::vector<float> m = CalculateDivide(cknots.size(), p, uniform);
				} else {

				}
				return true;
			}

			static std::vector<float> CalculateDivide(const int &n, const int &p, const bool &uniform) {
				std::vector<float> m;
				if (uniform) {
					int tot = n + p + 1;
					for (int i = 0; i < tot; i++) 
						m.push_back((float)i / tot);
				} else {

				}
			}
		};

		class NURBSCurve {
		public:

		};

		class CatmullRom {
		public:
			static Vector3 CatmullRom_Interpolation(const Vector3 &p0, const Vector3 &p1, const Vector3 &p2, const Vector3 &p3, const float &t) {
				return p1 + (0.5f * (p2 - p0) * t) +
					0.5f * (2.0f * p0 - 5.0f * p1 + 4.0f * p2 - p3) * t * t +
					0.5f * (-p0 + 3.0f * p1 - 3.0f * p2 + p3) * t * t * t;
			}
		};
	}
}

