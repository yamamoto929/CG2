#include "Vector3.h"
//============================================
// たしざん
//============================================
RunaEngine::Vector3 operator+=(RunaEngine::Vector3& lhs, const RunaEngine::Vector3& rhs) {
	lhs.x += rhs.x;
	lhs.y += rhs.y;
	lhs.z += rhs.z;
	return lhs;
}

RunaEngine::Vector3 operator+(const RunaEngine::Vector3& lhs, const RunaEngine::Vector3& rhs) {
	RunaEngine::Vector3 result = lhs;
	result += rhs;
	return result;
}

//============================================
// ひきざん
//============================================
RunaEngine::Vector3 operator-=(RunaEngine::Vector3& lhs, const RunaEngine::Vector3& rhs) {
	lhs.x -= rhs.x;
	lhs.y -= rhs.y;
	lhs.z -= rhs.z;
	return lhs;
}

RunaEngine::Vector3 operator-(const RunaEngine::Vector3& lhs, const RunaEngine::Vector3& rhs) {
	RunaEngine::Vector3 result = lhs;
	result -= rhs;
	return result;
}

//============================================
// かけざん
//============================================
RunaEngine::Vector3 operator*(const RunaEngine::Vector3& v, const float& s) {
	RunaEngine::Vector3 result = v;
	result.x *= s;
	result.y *= s;
	result.z *= s;
	return result;
}

RunaEngine::Vector3 operator*(const float& s, const RunaEngine::Vector3& v) {
	RunaEngine::Vector3 result = v;
	result.x *= s;
	result.y *= s;
	result.z *= s;
	return result;
}

//============================================
// わりざん
//============================================
RunaEngine::Vector3 operator/(const RunaEngine::Vector3& v, const float& s) {
	RunaEngine::Vector3 result = v;
	result.x /= s;
	result.y /= s;
	result.z /= s;
	return result;
}

RunaEngine::Vector3 operator/(const float& s, const RunaEngine::Vector3& v) {
	RunaEngine::Vector3 result = v;
	result.x /= s;
	result.y /= s;
	result.z /= s;
	return result;
}