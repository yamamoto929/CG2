#include "ShaderCompiler.h"
#include <cassert>

void ShaderCompiler::Initialize() {
    HRESULT hr = DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&dxcUtils_));
    assert(SUCCEEDED(hr));
    hr = DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&dxcCompiler_));
    assert(SUCCEEDED(hr));
    hr = dxcUtils_->CreateDefaultIncludeHandler(&includeHandler_);
    assert(SUCCEEDED(hr));
}

Microsoft::WRL::ComPtr<IDxcBlob> ShaderCompiler::Compile(const std::wstring& filePath, const wchar_t* profile) {
    Microsoft::WRL::ComPtr<IDxcBlobEncoding> source;
    HRESULT hr = dxcUtils_->LoadFile(filePath.c_str(), nullptr, &source);
    assert(SUCCEEDED(hr));
    if (FAILED(hr)) { return nullptr; }

    DxcBuffer input{source->GetBufferPointer(), source->GetBufferSize(), DXC_CP_UTF8};
    LPCWSTR arguments[] = {filePath.c_str(), L"-E", L"main", L"-T", profile, L"-Zpr"};
    Microsoft::WRL::ComPtr<IDxcResult> result;
    hr = dxcCompiler_->Compile(&input, arguments, _countof(arguments), includeHandler_.Get(), IID_PPV_ARGS(&result));
    assert(SUCCEEDED(hr));
    if (FAILED(hr)) { return nullptr; }

    // Compileの成功と、HLSLの内容が正しいかは別の結果なので両方確認する。
    HRESULT status = E_FAIL;
    hr = result->GetStatus(&status);
    assert(SUCCEEDED(hr));
    assert(SUCCEEDED(status));
    if (FAILED(hr) || FAILED(status)) { return nullptr; }

    Microsoft::WRL::ComPtr<IDxcBlob> binary;
    hr = result->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(&binary), nullptr);
    assert(SUCCEEDED(hr));
    return binary;
}
