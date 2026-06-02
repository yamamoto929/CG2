#pragma once
#include <dxcapi.h>
#pragma comment(lib,"dxcompiler.lib")
#include <wrl.h>
#include <string>
class ShaderCompiler{
private:
	Microsoft::WRL::ComPtr<IDxcUtils> dxcUtils_ = nullptr;
	Microsoft::WRL::ComPtr<IDxcCompiler3> dxcCompiler_ = nullptr;
	Microsoft::WRL::ComPtr<IDxcIncludeHandler> includeHandler_ = nullptr;
public:
	void Initialize();
	Microsoft::WRL::ComPtr<IDxcBlob> Compile(const std::wstring& filePath, const wchar_t* profile);
};

