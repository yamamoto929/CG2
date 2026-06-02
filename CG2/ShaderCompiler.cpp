#include "ShaderCompiler.h"
#include <cassert>
#include "ConvertString.h"
#include "Log.h"
#include <format>
void ShaderCompiler::Initialize() {
	HRESULT hr = DxcCreateInstance(CLSID_DxcUtils, IID_PPV_ARGS(&dxcUtils_));
	assert(SUCCEEDED(hr));
	hr = DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&dxcCompiler_));
	assert(SUCCEEDED(hr));

	// 現時点でincludeはしないが、includeに対応するための設定を行っておく
	hr = dxcUtils_->CreateDefaultIncludeHandler(&includeHandler_);
	assert(SUCCEEDED(hr));
}

Microsoft::WRL::ComPtr<IDxcBlob> ShaderCompiler::Compile(const std::wstring& filePath, const wchar_t* profile) {
	// これからシェーダーにコンパイルする旨をログに出す
	Log(ConvertString(std::format(L"Begin CompilerShader, path:{}, profile:{}\n", filePath, profile)));
	// hlslファイルを読む
	Microsoft::WRL::ComPtr<IDxcBlobEncoding> shaderSource;
	HRESULT hr = dxcUtils_->LoadFile(filePath.c_str(), nullptr, &shaderSource);
	// 読めなかったら止める
	assert(SUCCEEDED(hr));
	// 読み込んだファイルの内容を設定する
	DxcBuffer shaderSourcesBuffer;
	shaderSourcesBuffer.Ptr = shaderSource->GetBufferPointer();
	shaderSourcesBuffer.Size = shaderSource->GetBufferSize();
	shaderSourcesBuffer.Encoding = DXC_CP_UTF8;

	LPCWSTR arguments[] = {
		filePath.c_str(), // コンパイル対象のhlslファイル名
		L"-E",L"main", // エントリーポイントの指定。基本的にmain以外ではしない
		L"-T",profile, // ShaderProfileの設定
		L"-Zi",L"-Qembed_debug", // デバッグ用の情報を読み込む
		L"-Od",		// 最適化を外しておく
		L"-Zpr",	// メモリレイアウトは行優先
	};
	// 実際にShaderをコンパイルする
	Microsoft::WRL::ComPtr<IDxcResult> shaderResult;
	hr = dxcCompiler_->Compile(
		&shaderSourcesBuffer,	// 読み込んだファイル
		arguments,				// コンパイルオプション
		_countof(arguments),	// コンパイルオプションの数
		includeHandler_.Get(),	// includeが含まれた諸々 (ComPtrから生ポインタを取得)
		IID_PPV_ARGS(&shaderResult)// コンパイル結果
	);
	// コンパイルエラーではなくdxcが起動できななど致命的な状況
	assert(SUCCEEDED(hr));

	// 警告やエラーが出てたらログに出して止める
	Microsoft::WRL::ComPtr<IDxcBlobUtf8> shaderError;
	shaderResult->GetOutput(DXC_OUT_ERRORS, IID_PPV_ARGS(&shaderError), nullptr);
	if (shaderError != nullptr && shaderError->GetStringLength() != 0) {
		Log(shaderError->GetStringPointer());
		// 警告エラーダメゼッタイ
		assert(false);
	}

	// コンパイル結果から実行用のバイナリ部分を取得
	Microsoft::WRL::ComPtr<IDxcBlob> shaderBlob;
	hr = shaderResult->GetOutput(DXC_OUT_OBJECT, IID_PPV_ARGS(&shaderBlob), nullptr);
	assert(SUCCEEDED(hr));
	// 成功したログを出す
	Log(ConvertString(std::format(L"Compile Succeeded, path:{}, profile:{}\n", filePath, profile)));

	// 実行用のバイナリを返却
	return shaderBlob;
}