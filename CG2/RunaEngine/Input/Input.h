#pragma once
#ifndef DIRECTINPUT_VERSION
#define DIRECTINPUT_VERSION 0x0800
#endif
#include <dinput.h>
#pragma comment(lib,"dinput8.lib")
#include <wrl.h>
#include <cstdint>
class Input{
private:
	Microsoft::WRL::ComPtr<IDirectInput8> directInput_ = nullptr;
	Microsoft::WRL::ComPtr<IDirectInputDevice8> keyboard_ = nullptr;
	Microsoft::WRL::ComPtr<IDirectInputDevice8> mouse_ = nullptr;
	// キー情報
	BYTE key_[256]{};
	BYTE preKey_[256]{};
	// マウス情報
	DIMOUSESTATE mouseState_{};
	DIMOUSESTATE preMouseState_{};
public:
	void Initialize(HINSTANCE hInstance, HWND hwnd);
	void Update();
	const BYTE* GetKey() const { return key_; }
	const DIMOUSESTATE& GetMouseState() const { return mouseState_; }
	bool IsPushkey(uint8_t keyNum)const;
	bool IsTriggerkey(uint8_t keyNum)const;
	bool IsPressMouse(int button) const;
	bool IsTriggerMouse(int button) const;
	LONG GetMouseMoveX() const { return mouseState_.lX; }
	LONG GetMouseMoveY() const{return mouseState_.lY;}
	LONG GetMouseWheel() const { return mouseState_.lZ; }
};

