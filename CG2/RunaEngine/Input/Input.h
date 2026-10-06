#pragma once
#ifndef DIRECTINPUT_VERSION
#define DIRECTINPUT_VERSION 0x0800
#endif
#include <dinput.h>
#pragma comment(lib,"dinput8.lib")
#include <Xinput.h>
#pragma comment(lib,"Xinput.lib")
#include <wrl.h>
#include <cstdint>
#include "RunaEngine/Math/Vector2.h"
#include "InputTypes.h"
using namespace RunaEngine;
struct MousePosition {
	int32_t x = 0;
	int32_t y = 0;
};
class Input {
public:
	~Input();
	// 初期化
	void Initialize(HINSTANCE hInstance, HWND hwnd);
	void Shutdown();
	// 更新
	void Update();
	
	//=============================================
	// キー入力
	//=============================================
	bool IsPushkey(Key key)const;
	bool IsTriggerkey(Key key)const;
	//=============================================
	// マウス
	//=============================================
	const DIMOUSESTATE& GetMouseState() const { return mouseState_; }
	bool IsPressMouse(MouseButton button) const;
	bool IsTriggerMouse(MouseButton button) const;
	LONG GetMouseMoveX() const { return mouseState_.lX; }
	LONG GetMouseMoveY() const { return mouseState_.lY; }
	LONG GetMouseWheel() const { return mouseState_.lZ; }
	MousePosition GetMousePosition() const {
		return mousePosition_;
	}

	bool IsMouseInsideClient() const {
		return isMouseInsideClient_;
	}

	//=============================================
	// ゲームパッド
	//=============================================
	struct StickState {
		float horizontal;
		float vertical;
	};

	struct GamepadState {
		bool connected;
		StickState leftStick;
		StickState rightStick;
		float leftTrigger;
		float rightTrigger;
		uint16_t buttons;
	};
	void UpdateGamepad();
	bool FindGamepad();
	
	GamepadState ConvertGamepadState(
		const XINPUT_STATE& rawState)const;
	bool IsGamepadConnected()const;
	bool IsGamepadButtonDown( GamepadButton button);
	bool IsGamepadButtonTriggered( GamepadButton button);
	bool IsGamepadButtonReleased( GamepadButton button);
	StickState GetLeftStick() const;
	StickState GetRightStick() const;
	float GetLeftTrigger()const;
	float GetRightTrigger()const;

private:
	
	// DirectInput本体
	Microsoft::WRL::ComPtr<IDirectInput8> directInput_ = nullptr;
	// キー情報
	BYTE key_[256]{};
	BYTE preKey_[256]{};
	Microsoft::WRL::ComPtr<IDirectInputDevice8> keyboard_ = nullptr;
	// マウス情報
	DIMOUSESTATE mouseState_{};
	DIMOUSESTATE preMouseState_{};
	Microsoft::WRL::ComPtr<IDirectInputDevice8> mouse_ = nullptr;
	HWND hwnd_ = nullptr;
	MousePosition mousePosition_{};
	bool isMouseInsideClient_ = false;
	void UpdateMousePosition();
	
	// ゲームパッド情報
	GamepadState currentGamepad_{};
	GamepadState previousGamepad_{};
	DWORD activeGamepadIndex_ = XUSER_MAX_COUNT;
	static StickState NormalizeStick(
		SHORT rawX,
		SHORT rawY,
		SHORT deadZone
	);
	// XInputのボタンからエンジン内のボタンに変換
	uint16_t ToXInputMask(GamepadButton button);
};

// using namespace RunaEngine;
