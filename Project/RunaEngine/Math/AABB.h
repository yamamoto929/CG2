#pragma once
#include "Vector3.h"

namespace RunaEngine {
	struct AABB {
		Vector3 min;
		Vector3 max;

		bool Contains(const Vector3& point) const {
			return min.x <= point.x && point.x <= max.x
				&& min.y <= point.y && point.y <= max.y
				&& min.z <= point.z && point.z <= max.z;
		}
	};
}
