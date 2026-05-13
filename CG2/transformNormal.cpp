#include "transformNormal.h"
Vector3 TransformNormal(Vector3 v, Matrix4x4 m) {
	Vector3 result = {};
	result.x = (v.x * m.m[0][0]) + (v.y * m.m[1][0]) + (v.z * m.m[2][0]);
	result.y = (v.x * m.m[0][1]) + (v.y * m.m[1][1]) + (v.z * m.m[2][1]);
	result.z = (v.x * m.m[0][2]) + (v.y * m.m[1][2]) + (v.z * m.m[2][2]);
	return result;
}