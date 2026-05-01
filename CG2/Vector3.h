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
};