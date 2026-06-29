#pragma once
#include "Vector3.h"
#include "Matrix4x4.h"
#ifndef DIRECTINPUT_VERSION
#define DIRECTINPUT_VERSION 0x0800
#endif
#include <dinput.h>
class DebugCamera {
private:
	RunaEngine::Vector3 rotation_ = { 0.0f,0.0f,0.0f };
	// 累積回転行列
	RunaEngine::Matrix4x4 matRot_ = {};
	RunaEngine::Vector3 translation_ = { 0.0f,0.0f,-50.0f };
	RunaEngine::Matrix4x4 viewMatrix_ = {};
	RunaEngine::Matrix4x4 projectionMatrix_ = {};
	float rotateSpeed_ = 0.01f;
	float translationSpeed_ = 0.1f;
public:
	void Initialize();
	void Update(const uint8_t* keys,DIMOUSESTATE mouseState);
	void MoveTranslate(const RunaEngine::Vector3 move);
	RunaEngine::Matrix4x4 GetViewMatrix()const { return viewMatrix_; }
	RunaEngine::Vector3 GetRotation()const { return rotation_; }
	RunaEngine::Vector3 GetTranslation()const { return translation_; }
};

