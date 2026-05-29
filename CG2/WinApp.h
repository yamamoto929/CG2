#pragma once
#include <Windows.h>
#include <cstdint>

class WinApp{
private:
	HWND hwnd_;
	int32_t clientWidth_;
	int32_t clientHeight_;
public:
	HWND GetHwnd() const{ return hwnd_; }
	void CreateNewWindow();
};