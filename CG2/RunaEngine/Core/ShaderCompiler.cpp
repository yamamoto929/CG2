#include "ShaderCompiler.h"
#include "ConvertString.h"
#include "EngineError.h"
#include <filesystem>
#include <format>

void ShaderCompiler::Initialize() {
    CheckHR(DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&dxcUtils_)), "DxcCreateInstance(utils)");
    CheckHR(DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&dxcCompiler_)), "DxcCreateInstance(compiler)");
    CheckHR(dxcUtils_->CreateDefaultIncludeHandler(&includeHandler_), "CreateDefaultIncludeHandler");
}

Microsoft::WRL::ComPtr<IDxcBlob> ShaderCompiler::Compile(const std::wstring& filePath, const wchar_t* profile) {
    Require(dxcUtils_ && dxcCompiler_ && profile, "ShaderCompiler requires Initialize and a profile");
    const std::string path = ConvertString(filePath);
    Log("Compile shader: " + path);
    Microsoft::WRL::ComPtr<IDxcBlobEncoding> source;
    CheckHR(dxcUtils_->LoadFile(filePath.c_str(), nullptr, &source),
        "Load shader: " + path + " (working directory: " + std::filesystem::current_path().string() + ")");
    DxcBuffer input{source->GetBufferPointer(), source->GetBufferSize(), DXC_CP_UTF8};
    LPCWSTR arguments[] = {filePath.c_str(), L"-E", L"main", L"-T", profile,
        L"-Zi", L"-Qembed_debug", L"-Od", L"-Zpr"};
    Microsoft::WRL::ComPtr<IDxcResult> result;
    CheckHR(dxcCompiler_->Compile(&input, arguments, _countof(arguments), includeHandler_.Get(),
        IID_PPV_ARGS(&result)), "DXC Compile: " + path);
    Microsoft::WRL::ComPtr<IDxcBlobUtf8> diagnostics;
    CheckHR(result->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&diagnostics), nullptr), "DXC diagnostics: " + path);
    const std::string message = diagnostics && diagnostics->GetStringLength()
        ? std::string(diagnostics->GetStringPointer(), diagnostics->GetStringLength()) : "";
    if (!message.empty()) { Log(message); }
    HRESULT status = E_FAIL;
    CheckHR(result->GetStatus(&status), "DXC GetStatus: " + path);
    Require(SUCCEEDED(status), "Shader compilation failed: " + path + "\n" + message);
    Microsoft::WRL::ComPtr<IDxcBlob> binary;
    CheckHR(result->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(&binary), nullptr), "DXC object: " + path);
    Require(binary != nullptr, "DXC returned no shader binary: " + path);
    return binary;
}