#include "AffineMatrix.h"
#include <cmath>

Matrix4x4 MakeRotateXMatrix(float radian) {
	Matrix4x4 result = { {
		{1.0f, 0.0f,               0.0f,              0.0f},
		{0.0f, std::cos(radian),  std::sin(radian), 0.0f},
		{0.0f, -std::sin(radian), std::cos(radian), 0.0f},
		{0.0f, 0.0f,               0.0f,              1.0f}
	} };
	return result;
}

Matrix4x4 MakeRotateYMatrix(float radian) {
	Matrix4x4 result = { {
		{std::cos(radian), 0.0f, -std::sin(radian), 0.0f},
		{0.0f,              1.0f, 0.0f,               0.0f},
		{std::sin(radian), 0.0f, std::cos(radian),  0.0f},
		{0.0f,              0.0f, 0.0f,               1.0f}
	} };
	return result;
}

Matrix4x4 MakeRotateZMatrix(float radian) {
	Matrix4x4 result = { {
		{std::cos(radian),  std::sin(radian), 0.0f, 0.0f},
		{-std::sin(radian), std::cos(radian), 0.0f, 0.0f},
		{0.0f,               0.0f,              1.0f, 0.0f},
		{0.0f,               0.0f,              0.0f, 1.0f}
	} };
	return result;
}

Matrix4x4 MakeAffineMatrix(const Vector3& scale, const Vector3& rotate, const Vector3& translate) {
	Matrix4x4 result{};
	//============================================================
	// 拡縮
	//============================================================
	Matrix4x4 scaleMatrix4x4 = { {
		{scale.x, 0.0f,    0.0f,    0.0f},
		{0.0f,    scale.y, 0.0f,    0.0f},
		{0.0f,    0.0f,    scale.z, 0.0f},
		{0.0f,    0.0f,    0.0f,    1.0f}
	} };

	//============================================================
	// 回転
	//============================================================
	Matrix4x4 rotateMatrix4x4{};
	rotateMatrix4x4 = Multiply(Multiply(MakeRotateXMatrix(rotate.x), MakeRotateYMatrix(rotate.y)), MakeRotateZMatrix(rotate.z));

	//============================================================
	// 移動
	//============================================================
	Matrix4x4 translateMatrix4x4 = { {
		{1.0f,        0.0f,        0.0f,        0.0f},
		{0.0f,        1.0f,        0.0f,        0.0f},
		{0.0f,        0.0f,        1.0f,        0.0f},
		{translate.x, translate.y, translate.z, 1.0f}
	} };

	//============================================================
	// W=SRT
	//============================================================
	result = Multiply(Multiply(scaleMatrix4x4, rotateMatrix4x4), translateMatrix4x4);
	return result;
}