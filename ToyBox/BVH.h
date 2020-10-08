#pragma once
#include <vector>

#include "Vector2.h"
#include "Hitable.h"
#include "Debugger.h"
#include "BoundingBox.h"

namespace MOON {

	class BVH : public Hitable {
	public:
		#define CheckBox(l, r) (l->bounding_box(Vector2::ZERO(), box_left) && r->bounding_box(Vector2::ZERO(), box_right))

		static unsigned int divisionAxis;
		static int box_compare(const void* a, const void* b) {
			BoundingBox box_left, box_right;
			Hitable *ah = *(Hitable**)a, *bh = *(Hitable**)b;
			if (!CheckBox(ah, bh)) std::cerr << "no bounding box in bvh constructor!" << std::endl;
			if (divisionAxis == 0)
				if (box_left.min.x - box_right.min.x < 0) return -1;
				else if (divisionAxis == 1)
					if (box_left.min.y - box_right.min.y < 0) return -1;
					else if (box_left.min.z - box_right.min.z < 0) return -1;
			return 1;
		}

		Hitable* left;
		Hitable* right;

		BVH(std::vector<Hitable*>& l, Vector2 range, Vector2 timeStamp = Vector2::ZERO()) : left(nullptr), right(nullptr) {
			//if (!l.size()) return;

			divisionAxis = int(3 * MoonMath::drand48());
			int n = range.y - range.x;
			std::qsort(&l[range.x], n, sizeof(Hitable*), box_compare);

			if (n == 0) {
				left = right = l[range.x];
			} else if (n == 1) {
				left = l[range.x]; right = l[range.y];
			} else {
				left = new BVH(l, Vector2(range.x, range.x + n / 2), timeStamp);
				right = new BVH(l, Vector2(range.x + n / 2 + 1, range.y), timeStamp);
			}

			BoundingBox box_left, box_right;
			if (!CheckBox(left, right))
				std::cerr << "no bounding box in bvh constructor!" << std::endl;

			bbox = box_left.join(box_right);
		}

		~BVH() override {
			if (left != nullptr && CheckClass(left, BVH)) 
				delete left;
			if (right != nullptr && CheckClass(right, BVH)) 
				delete right;
		}

		virtual bool Hit(const Ray& r, HitRecord& rec) const override {
			if (bbox.intersect(r)) {
				HitRecord left_rec, right_rec;
				bool hit_left = this->left->Hit(r, left_rec);
				bool hit_right = this->right->Hit(r, right_rec);
				if (hit_left && hit_right)
					rec = left_rec.t < right_rec.t ? left_rec : right_rec;
				else if (hit_left) 
					rec = left_rec;
				else if (hit_right) 
					rec = right_rec;
				else return false;
				return true;
			} else return false;
		}

		void Draw(const Vector4& color = Color::WHITE()) {
			DEBUG::DrawBBox(this->bbox, color);

			auto col = color / 2.0f; col.w = 0.5f;
			if (left != nullptr && CheckClass(left, BVH)) ((BVH*)left)->Draw(col);
			if (right != nullptr && CheckClass(right, BVH)) ((BVH*)right)->Draw(col);
		}
	};

}