#define NOMINMAX
#include "Input.h"
#include <cassert>
#include <cstring>
#include <algorithm>
#include <cmath>
void Input::Shutdown() {
	if (keyboard_) { keyboard_->Unacquire(); }
	if (mouse_) { mouse_->Unacquire(); }
	keyboard_.Reset(); mouse_.Reset(); directInput_.Reset(); hwnd_ = nullptr;
	std::memset(key_, 0, sizeof(key_)); std::memset(preKey_, 0, sizeof(preKey_));
	mouseState_ = {}; preMouseState_ = {}; currentGamepad_ = {}; previousGamepad_ = {};
	activeGamepadIndex_ = XUSER_MAX_COUNT;
}
void Input::Initialize(HINSTANCE hInstance, HWND hwnd) {
	hwnd_ = hwnd;
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
		keyboard_->GetDeviceState(sizeof(key_), &key_);
	}

	// マウス
	preMouseState_ = mouseState_;
	mouse_->Acquire();
	hr = mouse_->GetDeviceState(sizeof(DIMOUSESTATE), &mouseState_);
	if (FAILED(hr)) {
		mouse_->Acquire();
		mouse_->GetDeviceState(sizeof(DIMOUSESTATE), &mouseState_);
	}

	UpdateMousePosition();

	// ゲームパッド
	UpdateGamepad();
}

void Input::UpdateGamepad() {
	previousGamepad_ = currentGamepad_;

	// 使用するコントローラーが未決定
	if (activeGamepadIndex_ == XUSER_MAX_COUNT) {
		FindGamepad();
		return;
	}

	XINPUT_STATE rawState{};

	DWORD result = XInputGetState(
		activeGamepadIndex_,
		&rawState
	);

	if (result == ERROR_SUCCESS) {
		currentGamepad_ =
			ConvertGamepadState(rawState);
	} else {
		// 切断された
		currentGamepad_ = {};
		previousGamepad_ = {};
		activeGamepadIndex_ = XUSER_MAX_COUNT;
	}
}

bool Input::FindGamepad() {
	for (DWORD index = 0;
		index < XUSER_MAX_COUNT;
		++index) {

		XINPUT_STATE rawState{};

		DWORD result = XInputGetState(
			index,
			&rawState
		);

		if (result == ERROR_SUCCESS) {
			activeGamepadIndex_ = index;

			currentGamepad_ =
				ConvertGamepadState(rawState);

			// 接続時に押されていたボタンを
			// Triggered扱いにしない
			previousGamepad_ = currentGamepad_;

			return true;
		}
	}

	// 1台も見つからなかった
	activeGamepadIndex_ = XUSER_MAX_COUNT;
	currentGamepad_ = {};
	previousGamepad_ = {};

	return false;
}

Input::GamepadState Input::ConvertGamepadState(
	const XINPUT_STATE& rawState) const {
		GamepadState state{};

		state.connected = true;
		state.buttons = rawState.Gamepad.wButtons;

		state.leftTrigger =
			static_cast<float>(
				rawState.Gamepad.bLeftTrigger
				) / 255.0f;

		state.rightTrigger =
			static_cast<float>(
				rawState.Gamepad.bRightTrigger
				) / 255.0f;

		state.leftStick = NormalizeStick(
			rawState.Gamepad.sThumbLX,
			rawState.Gamepad.sThumbLY,
			XINPUT_GAMEPAD_LEFT_THUMB_DEADZONE
		);

		state.rightStick = NormalizeStick(
			rawState.Gamepad.sThumbRX,
			rawState.Gamepad.sThumbRY,
			XINPUT_GAMEPAD_RIGHT_THUMB_DEADZONE
		);

		return state;
}

bool Input::IsPushkey(Key key)const {
	uint8_t keyNum = static_cast<uint8_t>(key);
	if ((key_[keyNum] & 0x80) != 0) {
		return true;
	}
	return false;
}

bool Input::IsTriggerkey(Key key)const {
	uint8_t keyNum = static_cast<uint8_t>(key);
	if ((key_[keyNum] & 0x80) != 0 && (preKey_[keyNum] & 0x80) == 0) {
		return true;
	}
	return false;
}

bool Input::IsPressMouse(MouseButton button) const {
	int buttonNum = static_cast<int>(button);
	if ((mouseState_.rgbButtons[buttonNum] & 0x80) != 0) {
		return true;
	}
	return false;
}

bool Input::IsTriggerMouse(MouseButton button)const {
	int buttonNum = static_cast<int>(button);
	if ((mouseState_.rgbButtons[buttonNum] & 0x80) != 0 &&
		(preMouseState_.rgbButtons[buttonNum] & 0x80) == 0) {
		return true;
	}
	return false;
}

bool  Input::IsGamepadConnected()const {
	return currentGamepad_.connected;
}

bool  Input::IsGamepadButtonDown(GamepadButton button) {
	if (!currentGamepad_.connected) {
		return false;
	}

	const uint16_t mask = ToXInputMask(button);

	return (currentGamepad_.buttons & mask);
}

bool  Input::IsGamepadButtonTriggered(GamepadButton button) {
	if (!currentGamepad_.connected) {
		return false;
	}

	const uint16_t mask = ToXInputMask(button);
	const bool current = (currentGamepad_.buttons & mask) != 0;
	const bool previous = (previousGamepad_.buttons & mask) != 0;
	return current && !previous;
}

bool  Input::IsGamepadButtonReleased(GamepadButton button) {
	if (!currentGamepad_.connected) {
		return false;
	}

	const uint16_t mask = ToXInputMask(button);
	const bool current = (currentGamepad_.buttons & mask) != 0;
	const bool previous = (previousGamepad_.buttons & mask) != 0;
	return !current && previous;
}

Input::StickState Input::GetLeftStick() const {
	return currentGamepad_.leftStick;
}

Input::StickState Input::GetRightStick() const {
	return currentGamepad_.rightStick;
}

float  Input::GetLeftTrigger() const {
	return currentGamepad_.leftTrigger;
}

float  Input::GetRightTrigger() const {
	return currentGamepad_.rightTrigger;
}

uint16_t Input::ToXInputMask(GamepadButton button) {
	switch (button) {
	case GamepadButton::A:
		return XINPUT_GAMEPAD_A;

	case GamepadButton::B:
		return XINPUT_GAMEPAD_B;

	case GamepadButton::X:
		return XINPUT_GAMEPAD_X;

	case GamepadButton::Y:
		return XINPUT_GAMEPAD_Y;

	case GamepadButton::DPadUp:
		return XINPUT_GAMEPAD_DPAD_UP;

	case GamepadButton::DPadDown:
		return XINPUT_GAMEPAD_DPAD_DOWN;

	case GamepadButton::DPadLeft:
		return XINPUT_GAMEPAD_DPAD_LEFT;

	case GamepadButton::DPadRight:
		return XINPUT_GAMEPAD_DPAD_RIGHT;

	case GamepadButton::LeftShoulder:
		return XINPUT_GAMEPAD_LEFT_SHOULDER;

	case GamepadButton::RightShoulder:
		return XINPUT_GAMEPAD_RIGHT_SHOULDER;

	case GamepadButton::LeftStick:
		return XINPUT_GAMEPAD_LEFT_THUMB;

	case GamepadButton::RightStick:
		return XINPUT_GAMEPAD_RIGHT_THUMB;

	case GamepadButton::Start:
		return XINPUT_GAMEPAD_START;

	case GamepadButton::Back:
		return XINPUT_GAMEPAD_BACK;
	}

	return 0;
}

void Input::UpdateMousePosition() {
	if (hwnd_ == nullptr) {
		isMouseInsideClient_ = false;
		return;
	}

	POINT point{};

	// デスクトップ全体を基準としたマウス座標を取得
	if (!GetCursorPos(&point)) {
		isMouseInsideClient_ = false;
		return;
	}

	// ゲームウィンドウのクライアント座標に変換
	if (!ScreenToClient(hwnd_, &point)) {
		isMouseInsideClient_ = false;
		return;
	}

	mousePosition_.x =
		static_cast<int32_t>(point.x);

	mousePosition_.y =
		static_cast<int32_t>(point.y);

	RECT clientRect{};

	if (!GetClientRect(hwnd_, &clientRect)) {
		isMouseInsideClient_ = false;
		return;
	}

	isMouseInsideClient_ =
		point.x >= clientRect.left &&
		point.x < clientRect.right &&
		point.y >= clientRect.top &&
		point.y < clientRect.bottom;
}

Input::StickState Input::NormalizeStick(
	SHORT rawX,
	SHORT rawY,
	SHORT deadZone) {

	StickState result{};

	const float x = static_cast<float>(rawX);
	const float y = static_cast<float>(rawY);

	// スティックを倒している距離
	const float magnitude = std::hypot(x, y);

	// デッドゾーン内なら(0, 0)
	if (magnitude <= static_cast<float>(deadZone)) {
		return result;
	}

	// スティックの方向
	const float directionX = x / magnitude;
	const float directionY = y / magnitude;

	// 斜め入力で最大値を超える場合に制限
	const float clampedMagnitude =
		std::min(magnitude, 32767.0f);

	// デッドゾーン直後を0、最大入力を1へ変換
	const float normalizedMagnitude = std::clamp(
		(clampedMagnitude - static_cast<float>(deadZone)) /
		(32767.0f - static_cast<float>(deadZone)),
		0.0f,
		1.0f
	);

	result.horizontal =
		directionX * normalizedMagnitude;

	result.vertical =
		directionY * normalizedMagnitude;

	return result;
}
