#include "WVPMatrix.h"
#include <cmath>
// 透視投影行列
Matrix4x4 MakePerspectiveFovMatrix(float fovy, float aspectRatio, float nearClip, float farClip) {

	Matrix4x4 result = { {
		{(1.0f / aspectRatio) * Cot(fovy / 2.0f),0.0f,            0.0f,                                        0.0f},
		{0.0f,                                   Cot(fovy / 2.0f),0.0f,                                        0.0f},
		{0.0f,                                   0.0f,            farClip / (farClip - nearClip),              1.0f},
		{0.0f,                                   0.0f,            (-nearClip * farClip) / (farClip - nearClip),0.0f}
	} };

	return result;
}
// 正射影行列
Matrix4x4 MakeOrthographicMatrix(float left, float top, float right, float bottom, float nearClip, float farClip) {
	Matrix4x4 result = { {
		{2.0f / (right - left),           0.0f,                           0.0f,                           0.0f},
		{0.0f,                            2.0f / (top - bottom),          0.0f,                           0.0f},
		{0.0f,                            0.0f,                           1.0f / (farClip - nearClip),    0.0f},
		{ (left + right) / (left - right),(top + bottom) / (bottom - top),nearClip / (nearClip - farClip),1.0f}
	} };

	return result;
}
// ビューポート変換行列
Matrix4x4 MakeViewportMatrix(float left, float top, float width, float height, float minDepth, float maxDepth) {
	Matrix4x4 result = { {
		{width / 2.0f,       0.0f,               0.0f,               0.0f},
		{0.0f,               -height / 2.0f,     0.0f,               0.0f},
		{0.0f,               0.0f,               maxDepth - minDepth,0.0f},
		{left + width / 2.0f,top + height / 2.0f,minDepth,           1.0f}
	} };

	return result;
}

float Cot(float a) {
	return 1.0f / std::tan(a);
}