#pragma once
#include <cmath>
struct Vector3 {
	float x;
	float y;
	float z;

	float Length()const {
		return  std::sqrt(x * x + y * y + z * z);
	}

	void Normalize() {
		float length = Length();
		if (length > 0.0001f) {
			x /= length;
			y /= length;
			z /= length;
		} else {
			x = 0.0f; y = 0.0f; z = 0.0f;
		}
	}

	float Length(const Vector3& v) {
		return std::sqrt(v.x * v.x + v.y * v.y + v.z * v.z);
	}

	Vector3 Normalize(const Vector3& v) {
		float length = Length(v);
		if (length == 0.0f) { return v; }
		return Vector3{ v.x / length,v.y / length, v.z / length, };
	}

	float Dot(const Vector3& v1, const Vector3& v2) {
		return v1.x * v2.x
			+ v1.y * v2.y
			+ v1.z * v2.z;
	}

	Vector3 Cross(const Vector3& v1, const Vector3& v2) {
		Vector3 result{};
		result.x = v1.y * v2.z - v1.z * v2.y;
		result.y = v1.z * v2.x - v1.x * v2.z;
		result.z = v1.x * v2.y - v1.y * v2.x;
		return result;
	}

};
//============================================
// たしざん
//============================================
Vector3 operator+=(Vector3& lhs, const Vector3& rhs);
Vector3 operator+(const Vector3& lhs, const Vector3& rhs);
//============================================
// ひきざん
//============================================
Vector3 operator-=(Vector3& lhs, const Vector3& rhs);

Vector3 operator-(const Vector3& lhs, const Vector3& rhs);
//============================================
// かけざん
//============================================
Vector3 operator*(const Vector3& v, const float& s);

Vector3 operator*(const float& s, const Vector3& v);

//============================================
// わりざん
//============================================
Vector3 operator/(const Vector3& v, const float& s);

Vector3 operator/(const float& s, const Vector3& v);
