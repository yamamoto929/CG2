#pragma once
#include <Windows.h>
#include <cstdint>
#include <string>

class WinApp{
private:
	HWND hwnd_;
	WNDCLASS windowClass_;
public:
	HWND GetHwnd() const{ return hwnd_; }
	HINSTANCE GetHInstance()const { return windowClass_.hInstance; }
	void CreateNewWindow(int32_t width,int32_t height, const std::string& title);
};