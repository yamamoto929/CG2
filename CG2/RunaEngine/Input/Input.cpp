#include "Input.h"
#include <cassert>
#include <cstring>
void Input::Initialize(HINSTANCE hInstance, HWND hwnd) {
	// DirectInput
	HRESULT hr = DirectInput8Create(hInstance, DIRECTINPUT_VERSION, IID_IDirectInput8, (void**)&directInput_, nullptr);
	assert(SUCCEEDED(hr));
	// キーボード
	hr = directInput_->CreateDevice(GUID_SysKeyboard, &keyboard_, NULL);
	assert(SUCCEEDED(hr));

	hr = keyboard_->SetDataFormat(&c_dfDIKeyboard);
	assert(SUCCEEDED(hr));

	hr = keyboard_->SetCooperativeLevel(hwnd, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE | DISCL_NOWINKEY);
	assert(SUCCEEDED(hr));
	// マウス
	hr = directInput_->CreateDevice(GUID_SysMouse, &mouse_, NULL);
	assert(SUCCEEDED(hr));

	hr = mouse_->SetDataFormat(&c_dfDIMouse);
	assert(SUCCEEDED(hr));

	hr = mouse_->SetCooperativeLevel(hwnd, DISCL_FOREGROUND | DISCL_NONEXCLUSIVE);
	assert(SUCCEEDED(hr));
}

void Input::Update() {
	// キーボート情報の取得
	std::memcpy(preKey_, key_, sizeof(key_));
	keyboard_->Acquire();
	HRESULT hr = keyboard_->GetDeviceState(sizeof(key_), key_);
	if (FAILED(hr)) {
		keyboard_->Acquire();
		keyboard_->GetDeviceState(sizeof(BYTE), &key_);
	}

	// マウス
	preMouseState_ = mouseState_;
	mouse_->Acquire();
	hr = mouse_->GetDeviceState(sizeof(DIMOUSESTATE), &mouseState_);
	if (FAILED(hr)) {
		mouse_->Acquire();
		mouse_->GetDeviceState(sizeof(DIMOUSESTATE), &mouseState_);
	}
}

bool Input::IsPushkey(uint8_t key) {
	if ((key & 0x80) != 0) {
		return true;
	}
	return false;
}

bool Input::IsTriggerkey(uint8_t key, uint8_t preKey) {
	if ((key & 0x80) != 0 && (preKey & 0x80) == 0) {
		return true;
	}
	return false;
}

bool Input::IsPressMouse(int button) const {
	if ((mouseState_.rgbButtons[button] & 0x80) != 0) {
		return true;
	}
	return false;
}

bool Input::IsTriggerMouse(int button)const {
	if ((mouseState_.rgbButtons[button] & 0x80) != 0 &&
		(preMouseState_.rgbButtons[button] & 0x80) == 0) {
		return true;
	}
	return false;
}