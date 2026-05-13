#include "DebugCamera.h"
#include <dinput.h>
#include "AffineMatrix.h"
#include "transformNormal.h"
void DebugCamera::Initialize() {
	matRot_ = MakeIdentityMatrix();
}

void DebugCamera::Update(const uint8_t* keys, DIMOUSESTATE mouseState) {
	float rotateRadX = 0.0f;
	float rotateRadY = 0.0f;
	if (keys[DIK_UP]) {
		rotateRadX = -rotateSpeed_;
	} else if (keys[DIK_DOWN]) {
		rotateRadX = rotateSpeed_;
	}

	if (keys[DIK_LEFT]) {
		rotateRadY = -rotateSpeed_;
	} else if (keys[DIK_RIGHT]) {
		rotateRadY = rotateSpeed_;
	}

	// マウスによる回転処理 
	if (mouseState.rgbButtons[1] & 0x80) {
		float mouseSensitivity = 0.005f; // マウス感度（

		// マウスの移動量を回転角に変換
		rotateRadX = mouseState.lY * -mouseSensitivity;
		rotateRadY = mouseState.lX * -mouseSensitivity;
	}

	// 
	matRot_ = Multiply(MakeRotateXMatrix(rotateRadX), matRot_);
	matRot_ = Multiply(matRot_, MakeRotateYMatrix(rotateRadY));
	
	if (keys[DIK_A]) {
		Vector3 move = { -translationSpeed_,0.0f,0.0f };
		MoveTranslate(move);
	} else if (keys[DIK_D]) {
		Vector3 move = { translationSpeed_,0.0f,0.0f };
		MoveTranslate(move);
	}

	if (keys[DIK_LSHIFT]) {
		Vector3 move = { 0.0f,-translationSpeed_,0.0f };
		MoveTranslate(move);
	} else if (keys[DIK_SPACE]) {
		Vector3 move = { 0.0f,translationSpeed_,0.0f };
		MoveTranslate(move);
	}

	if (keys[DIK_W]) {
		Vector3 move = { 0.0f,0.0f,translationSpeed_ };
		MoveTranslate(move);
	} else if (keys[DIK_S]) {
		Vector3 move = { 0.0f,0.0f,-translationSpeed_ };
		MoveTranslate(move);
	}

	Matrix4x4 translationMatrix = { {
		{1.0f,        0.0f,        0.0f,        0.0f},
		{0.0f,        1.0f,        0.0f,        0.0f},
		{0.0f,        0.0f,        1.0f,        0.0f},
		{translation_.x, translation_.y, translation_.z, 1.0f}
	} };
	Matrix4x4 cameraMatrix = Multiply(Multiply(MakeIdentityMatrix(), matRot_), translationMatrix);

	viewMatrix_ = Inverse(cameraMatrix);
}

void DebugCamera::MoveTranslate(const Vector3 move) {
	Vector3 moveVector;
	moveVector = TransformNormal(move, matRot_);
	translation_ += moveVector;
}