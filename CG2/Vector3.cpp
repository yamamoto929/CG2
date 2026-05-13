#include "Vector3.h"
//============================================
// たしざん
//============================================
Vector3 operator+=(Vector3& lhs, const Vector3& rhs) {
	lhs.x += rhs.x;
	lhs.y += rhs.y;
	lhs.z += rhs.z;
	return lhs;
}

Vector3 operator+(const Vector3& lhs, const Vector3& rhs) {
	Vector3 result = lhs;
	result += rhs;
	return result;
}

//============================================
// ひきざん
//============================================
Vector3 operator-=(Vector3& lhs, const Vector3& rhs) {
	lhs.x -= rhs.x;
	lhs.y -= rhs.y;
	lhs.z -= rhs.z;
	return lhs;
}

Vector3 operator-(const Vector3& lhs, const Vector3& rhs) {
	Vector3 result = lhs;
	result -= rhs;
	return result;
}

//============================================
// かけざん
//============================================
Vector3 operator*(const Vector3& v, const float& s) {
	Vector3 result = v;
	result.x *= s;
	result.y *= s;
	result.z *= s;
	return result;
}

Vector3 operator*(const float& s, const Vector3& v) {
	Vector3 result = v;
	result.x *= s;
	result.y *= s;
	result.z *= s;
	return result;
}

//============================================
// わりざん
//============================================
Vector3 operator/(const Vector3& v, const float& s) {
	Vector3 result = v;
	result.x /= s;
	result.y /= s;
	result.z /= s;
	return result;
}

Vector3 operator/(const float& s, const Vector3& v) {
	Vector3 result = v;
	result.x /= s;
	result.y /= s;
	result.z /= s;
	return result;
}