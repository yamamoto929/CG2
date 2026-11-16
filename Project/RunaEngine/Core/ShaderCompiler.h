#pragma once
#include <Windows.h>
#include <wrl.h>
#include <dxcapi.h>
#pragma comment(lib, "dxcompiler.lib")
#include <string>
class ShaderCompiler{
private:
	Microsoft::WRL::ComPtr<IDxcUtils> dxcUtils_ = nullptr;
	Microsoft::WRL::ComPtr<IDxcCompiler3> dxcCompiler_ = nullptr;
	Microsoft::WRL::ComPtr<IDxcIncludeHandler> includeHandler_ = nullptr;
public:
	void Initialize();
	void Shutdown() { includeHandler_.Reset(); dxcCompiler_.Reset(); dxcUtils_.Reset(); }
	Microsoft::WRL::ComPtr<IDxcBlob> Compile(const std::wstring& filePath, const wchar_t* profile);
};

