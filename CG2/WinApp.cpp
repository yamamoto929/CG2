#include "WinApp.h"
#ifdef USE_IMGUI
#include "externals/imgui/imgui.h"
#include "externals/imgui/imgui_impl_dx12.h"
#include "externals/imgui/imgui_impl_win32.h"
#include "ConvertString.h"
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
#endif // USE_IMGUI

static LRESULT CALLBACK WindowProc(HWND hwnd, UINT msg,
	WPARAM wparam, LPARAM lparam) {
#ifdef USE_IMGUI
	if (ImGui_ImplWin32_WndProcHandler(hwnd, msg, wparam, lparam)) {
		return true;
	}
#endif
	// メッセージに応じてゲーム固有の処理を行う
	switch (msg) {

		// ウィンドウが破棄された
	case WM_DESTROY:

		// OSに対して、アプリの終了を伝える
		PostQuitMessage(0);
		return 0;
	}

	// 標準のメッセージ処理を行う
	return DefWindowProc(hwnd, msg, wparam, lparam);
}

void WinApp::CreateNewWindow(int32_t width, int32_t height, const std::string& title) {
	// ウィンドウサイズを表す構造体にクライアント領域を入れる
	RECT wrc = { 0, 0, width, height };
	// クライアント領域を元に実際のサイズにwrcを変更してもらう
	AdjustWindowRect(&wrc, WS_OVERLAPPEDWINDOW, false);

	// ウィンドウプロシージャ
	windowClass_.lpfnWndProc = WindowProc;
	// ウィンドウクラス名(なんでも良い)
	windowClass_.lpszClassName = L"RunaEngine";
	// インスタンスハンドル
	windowClass_.hInstance = GetModuleHandle(nullptr);
	// カーソル
	windowClass_.hCursor = LoadCursor(nullptr, IDC_ARROW);
	// ウィンドウクラスを登録する
	RegisterClass(&windowClass_);

	// ウィンドウの生成
	hwnd_ = CreateWindow(
		windowClass_.lpszClassName,		// 利用するクラス名
		ConvertString(title).c_str(),	// タイトルバーの文字
		WS_OVERLAPPEDWINDOW,	// よく見るウィンドウスタイル
		CW_USEDEFAULT,			// 表示X座標(Windowsに任せる)
		CW_USEDEFAULT,			// 表示Y座標(Windowsに任せる)
		wrc.right - wrc.left,	// ウィンドウ横幅
		wrc.bottom - wrc.top,	// ウィンドウ縦幅
		nullptr,				// 親ウィンドウハンドル
		nullptr,				// メニューハンドル
		windowClass_.hInstance,			// インスタンスハンドル
		nullptr);				// オプション

	ShowWindow(hwnd_, SW_SHOW);
}