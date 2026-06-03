#include <Windows.h>
#include <cstdint>
#include <string>
#include <format>
#include <fstream>
#include <d3d12.h>
#pragma comment(lib,"d3d12.lib")
#include <dxgi1_6.h>
#pragma comment(lib,"dxgi.lib")
#include <cassert>
#include <dbghelp.h>
#pragma comment(lib,"Dbghelp.lib")
#include <strsafe.h>
#include <dxgidebug.h>
#pragma comment(lib,"dxguid.lib")
#include <dxcapi.h>
#pragma comment(lib,"dxcompiler.lib")
#include <vector>
#include <numbers>
#include <cmath>
#include <sstream>
#include <wrl.h>
#include <xaudio2.h>
#pragma comment (lib,"xaudio2.lib")
#include <mfapi.h>
#include <mfidl.h>
#include <mfreadwrite.h>
#pragma comment(lib, "Mf.lib")
#pragma comment(lib, "mfplat.lib")
#pragma comment(lib, "Mfreadwrite.lib")
#pragma comment(lib, "mfuuid.lib")
#define DIRECTINPUT_VERSION		0X0800
#include <dinput.h>
#pragma comment(lib,"dinput8.lib")
#pragma comment(lib,"dxguid.lib")
#include "externals\DirectXTex\DirectXTex.h"
#include "externals\DirectXTex\d3dx12.h"
#include "ConvertString.h"
#include "Vector4.h"
#include "Matrix4x4.h"
#include "Transform.h"
#include "AffineMatrix.h"
#include "WVPMatrix.h"
#include "VertexData.h"
#include "Material.h"
#include "TransformationMatrix.h"
#include "directionalLight.h"
#include "ModelData.h"
#include "MaterialData.h"
#include "D3DResourceLeakChecker.h"
#include "DebugCamera.h"
#ifdef USE_IMGUI
#include "externals/imgui/imgui.h"
#include "externals/imgui/imgui_impl_dx12.h"
#include "externals/imgui/imgui_impl_win32.h"
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
#endif // USE_IMGUI
#include "WinApp.h"
#include "Log.h"
#include "DirectXCommon.h"
#include "Input.h"
#include "ShaderCompiler.h"
#include "TextureManager.h"
#include "ModelLoader.h"
#include "Model.h"
#include "Object3D.h"
#include "Sprite.h"
static LONG WINAPI ExportDump(EXCEPTION_POINTERS* exception);
Microsoft::WRL::ComPtr<ID3D12Resource> CreateBufferResource(const Microsoft::WRL::ComPtr<ID3D12Device>& device,
	size_t sizeInBytes);
const Transform kDefaultCameraTransform{ {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f},{0.0f,0.0f,-10.0f} };

// Windowsアプリのエントリポイント(main関数)
int WINAPI WinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPSTR, _In_ int) {
	D3DResourceLeakChecker leakChecker;
	CoInitializeEx(0, COINIT_MULTITHREADED);
	MFStartup(MF_VERSION, MFSTARTUP_NOSOCKET);
	InitLog();
	SetUnhandledExceptionFilter(ExportDump);

	WinApp winApp;
	// クライアント領域のサイズ
	const int32_t kClientWidth = 1280;
	const int32_t kClientHeight = 720;
	winApp.CreateNewWindow(kClientWidth, kClientHeight, "CG2");

#ifdef _DEBUG
	Microsoft::WRL::ComPtr<ID3D12Debug1> debugController = nullptr;
	if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)))) {
		// デバッグレイヤーを有効化する
		debugController->EnableDebugLayer();
		// さらにGPU側でもチェックを行うようにする
		debugController->SetEnableGPUBasedValidation(TRUE);
	}
#endif
	DirectXCommon directXCommon;
	directXCommon.Initialize(winApp.GetHwnd(), kClientWidth, kClientHeight);
#ifdef _DEBUG
	ID3D12InfoQueue* infoQueue = nullptr;
	if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&infoQueue)))) {
		// やばいエラー時に止まる
		infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, true);
		// エラー時に止まる
		infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, true);
		// 警告時に止まる
		infoQueue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_WARNING, true);
		// 抑制するメッセージのID
		D3D12_MESSAGE_ID denyIds[] = {
			// Windows11でのDXGIデバッグレイヤーとDX12デバッグレイヤーの相互作用バグによるエラーメッセージ
			D3D12_MESSAGE_ID_RESOURCE_BARRIER_MISMATCHING_COMMAND_LIST_TYPE
		};
		// 抑制するレベル
		D3D12_MESSAGE_SEVERITY severities[] = { D3D12_MESSAGE_SEVERITY_INFO };
		D3D12_INFO_QUEUE_FILTER filter{};
		filter.DenyList.NumIDs = _countof(denyIds);
		filter.DenyList.pIDList = denyIds;
		filter.DenyList.NumSeverities = _countof(severities);
		filter.DenyList.pSeverityList = severities;
		// 指定したメッセージの表示を抑制する
		infoQueue->PushStorageFilter(&filter);
		// 解放
		infoQueue->Release();
	}
#endif 
	TextureManager textureManager;
	textureManager.Initialize(directXCommon.GetDevice(), directXCommon.GetCommandList(), 128);

	Input input;
	input.Initialize(winApp.GetHInstance(), winApp.GetHwnd());

	Microsoft::WRL::ComPtr<IXAudio2> xAudio2;
	IXAudio2MasteringVoice* masterVoice = nullptr;
	HRESULT hr = XAudio2Create(xAudio2.GetAddressOf(), 0, XAUDIO2_DEFAULT_PROCESSOR);
	hr = xAudio2->CreateMasteringVoice(&masterVoice);

	std::wstring path = (L"Resources/Alarm01.wav");
	IMFSourceReader* MFSourceReader{ nullptr };
	MFCreateSourceReaderFromURL(path.c_str(), NULL, &MFSourceReader);

	IMFMediaType* MFMediaType{ nullptr };
	MFCreateMediaType(&MFMediaType);
	MFMediaType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Audio);
	MFMediaType->SetGUID(MF_MT_SUBTYPE, MFAudioFormat_PCM);
	MFSourceReader->SetCurrentMediaType(MF_SOURCE_READER_FIRST_AUDIO_STREAM, nullptr, MFMediaType);

	MFMediaType->Release();
	MFMediaType = nullptr;
	MFSourceReader->GetCurrentMediaType(MF_SOURCE_READER_FIRST_AUDIO_STREAM, &MFMediaType);

	WAVEFORMATEX* waveFormat{ nullptr };
	MFCreateWaveFormatExFromMFMediaType(MFMediaType, &waveFormat, nullptr);

	std::vector<BYTE> mediaData;
	while (true) {
		IMFSample* MFSample{ nullptr };
		DWORD dwStreamFlags{ 0 };
		MFSourceReader->ReadSample(MF_SOURCE_READER_FIRST_AUDIO_STREAM, 0, nullptr, &dwStreamFlags, nullptr, &MFSample);

		if (dwStreamFlags & MF_SOURCE_READERF_ENDOFSTREAM) {
			break;
		}

		if (MFSample) {
			IMFMediaBuffer* pMFMediaBuffer{ nullptr };
			MFSample->ConvertToContiguousBuffer(&pMFMediaBuffer);

			BYTE* pBuffer{ nullptr };
			DWORD cbCurrentLength{ 0 };
			pMFMediaBuffer->Lock(&pBuffer, nullptr, &cbCurrentLength);

			mediaData.resize(mediaData.size() + cbCurrentLength);
			memcpy(mediaData.data() + mediaData.size() - cbCurrentLength, pBuffer, cbCurrentLength);

			pMFMediaBuffer->Unlock();

			pMFMediaBuffer->Release();
			MFSample->Release();
		}
	}

	IXAudio2SourceVoice* sourceVoice{ nullptr };
	xAudio2->CreateSourceVoice(&sourceVoice, waveFormat);

	XAUDIO2_BUFFER buffer{ 0 };
	buffer.pAudioData = mediaData.data();
	buffer.Flags = XAUDIO2_END_OF_STREAM;
	buffer.AudioBytes = sizeof(BYTE) * static_cast<UINT32>(mediaData.size());
	sourceVoice->SubmitSourceBuffer(&buffer);
	sourceVoice->Start(0);

	MFMediaType->Release();
	MFSourceReader->Release();
	CoTaskMemFree(waveFormat);

	ShaderCompiler shaderCompiler;
	shaderCompiler.Initialize();

	// RootSignature作成
	D3D12_ROOT_SIGNATURE_DESC descriptionRootSignature{};
	descriptionRootSignature.Flags =
		D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

	D3D12_DESCRIPTOR_RANGE descriptorRange[1] = {};
	descriptorRange[0].BaseShaderRegister = 0; // 0から始まる
	descriptorRange[0].NumDescriptors = 1; // 数は1つ
	descriptorRange[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV; // SRVを使う
	descriptorRange[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND; // Offsetを自動計算
	// RootParameter作成。複数設定できるので配列。今回は結果1つだけだから長さ1の配列
	D3D12_ROOT_PARAMETER rootParameters[4] = {};
	rootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV; // CBVを使う
	rootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL; // PixelShaderで使う
	rootParameters[0].Descriptor.ShaderRegister = 0; // レジスタ番号0とバインド
	rootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
	rootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
	rootParameters[1].Descriptor.ShaderRegister = 0;
	rootParameters[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE; // DescriptorTableを使う
	rootParameters[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL; // PixelShaderで使う
	rootParameters[2].DescriptorTable.pDescriptorRanges = descriptorRange; // Tableの中身の配列を指定
	rootParameters[2].DescriptorTable.NumDescriptorRanges = _countof(descriptorRange); // Tableで利用する数
	rootParameters[3].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV; // CBVを使う
	rootParameters[3].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;			//PixelShaderで使う
	rootParameters[3].Descriptor.ShaderRegister = 1;	// レジスタ番号1を使う
	descriptionRootSignature.pParameters = rootParameters; // ルートパラメータ配列へのポインタ
	descriptionRootSignature.NumParameters = _countof(rootParameters); // 配列の長さ

	D3D12_STATIC_SAMPLER_DESC staticSamplers[1] = {};
	staticSamplers[0].Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR; // バイリニアフィルタ
	staticSamplers[0].AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;// 0~1の範囲外をリピート
	staticSamplers[0].AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	staticSamplers[0].AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
	staticSamplers[0].ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;// 比較しない
	staticSamplers[0].MaxLOD = D3D12_FLOAT32_MAX;// ありったけのMipmapを使う
	staticSamplers[0].ShaderRegister = 0;// レジスタ番号0を使う
	staticSamplers[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL; // PixelShaderで使う
	descriptionRootSignature.pStaticSamplers = staticSamplers;
	descriptionRootSignature.NumStaticSamplers = _countof(staticSamplers);

	// シリアライズしてバイナリにする
	Microsoft::WRL::ComPtr<ID3DBlob> signatureBlob = nullptr;
	Microsoft::WRL::ComPtr<ID3DBlob> errorBlob = nullptr;
	hr = D3D12SerializeRootSignature(&descriptionRootSignature,
		D3D_ROOT_SIGNATURE_VERSION_1, &signatureBlob, &errorBlob);
	if (FAILED(hr)) {
		Log(reinterpret_cast<char*>(errorBlob->GetBufferPointer()));
		assert(false);
	}
	// バイナリを元に生成
	Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature = nullptr;
	hr = directXCommon.GetDevice()->CreateRootSignature(0,
		signatureBlob->GetBufferPointer(), signatureBlob->GetBufferSize(),
		IID_PPV_ARGS(&rootSignature));
	assert(SUCCEEDED(hr));
	D3D12_INPUT_ELEMENT_DESC inputElementDescs[3] = {};
	inputElementDescs[0].SemanticName = "POSITION";
	inputElementDescs[0].SemanticIndex = 0;
	inputElementDescs[0].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	inputElementDescs[0].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
	inputElementDescs[1].SemanticName = "TEXCOORD";
	inputElementDescs[1].SemanticIndex = 0;
	inputElementDescs[1].Format = DXGI_FORMAT_R32G32_FLOAT;
	inputElementDescs[1].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
	inputElementDescs[2].SemanticName = "NORMAL";
	inputElementDescs[2].SemanticIndex = 0;
	inputElementDescs[2].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	inputElementDescs[2].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;
	D3D12_INPUT_LAYOUT_DESC inputLayoutDesc{};
	inputLayoutDesc.pInputElementDescs = inputElementDescs;
	inputLayoutDesc.NumElements = _countof(inputElementDescs);
	// BlendStateの設定
	D3D12_BLEND_DESC blendDesc{};
	// 全ての色要素を書き込む
	blendDesc.RenderTarget[0].RenderTargetWriteMask =
		D3D12_COLOR_WRITE_ENABLE_ALL;
	// RasterizerStateの設定
	D3D12_RASTERIZER_DESC rasterizerDesc{};
	// 裏面(時計回り)を表示しない
	rasterizerDesc.CullMode = D3D12_CULL_MODE_BACK;
	// 三角形の中を塗りつぶす
	rasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;
	// Shaderをコンパイルする
	Microsoft::WRL::ComPtr<IDxcBlob> vertexShaderBlob =
		shaderCompiler.Compile(L"Object3D.VS.hlsl", L"vs_6_0");

	Microsoft::WRL::ComPtr<IDxcBlob> pixelShaderBlob =
		shaderCompiler.Compile(L"Object3d.PS.hlsl", L"ps_6_0");

	// DepthStencilStateの設定
	D3D12_DEPTH_STENCIL_DESC depthStencilDesc{};
	// Depthの機能を有効化する
	depthStencilDesc.DepthEnable = true;
	// 書き込みします
	depthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
	// 比較関数はLess Equal。つまり、近ければ描画される
	depthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;

	D3D12_GRAPHICS_PIPELINE_STATE_DESC graphicPipelineStateDesc{};
	graphicPipelineStateDesc.pRootSignature = rootSignature.Get(); // RootSignature
	graphicPipelineStateDesc.InputLayout = inputLayoutDesc; // InputLayout
	graphicPipelineStateDesc.VS = { vertexShaderBlob->GetBufferPointer(),
	vertexShaderBlob->GetBufferSize() }; // VertexShader
	graphicPipelineStateDesc.PS = { pixelShaderBlob->GetBufferPointer(),
	pixelShaderBlob->GetBufferSize() }; // PixelShader
	graphicPipelineStateDesc.BlendState = blendDesc;
	graphicPipelineStateDesc.RasterizerState = rasterizerDesc; // RasterizerState
	// 書き込むRTVの情報
	graphicPipelineStateDesc.NumRenderTargets = 1;
	graphicPipelineStateDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	// 利用するトポロジ(形状)のタイプ。三角形
	graphicPipelineStateDesc.PrimitiveTopologyType =
		D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	// どのように画面に色を打ち込むかの設定(気にしなくてよい)
	graphicPipelineStateDesc.SampleDesc.Count = 1;
	graphicPipelineStateDesc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;
	// DepthStencilの設定
	graphicPipelineStateDesc.DepthStencilState = depthStencilDesc;
	graphicPipelineStateDesc.DSVFormat = DXGI_FORMAT_D24_UNORM_S8_UINT;
	// 実際に生成
	Microsoft::WRL::ComPtr<ID3D12PipelineState> graphicPipelineState = nullptr;
	hr = directXCommon.GetDevice()->CreateGraphicsPipelineState(&graphicPipelineStateDesc,
		IID_PPV_ARGS(&graphicPipelineState));
	assert(SUCCEEDED(hr));

	ModelLoader modelLoader;
	// モデル読み込み
	ModelData modelData = modelLoader.LoadObjFile("resources", "axis.obj");
	Model model;
	model.Initialize(directXCommon.GetDevice(), &textureManager, &modelData);

	Object3D object3D;
	object3D.Initialize(directXCommon.GetDevice(), &model);

	Sprite sprite;
	sprite.Initialize(
		directXCommon.GetDevice(),
		&textureManager,
		"./resources/uvChecker.png"
	);

	Microsoft::WRL::ComPtr<ID3D12Resource> directionalLightResource = CreateBufferResource(directXCommon.GetDevice(), sizeof(DirectionalLight));
	DirectionalLight* directionalLightData = nullptr;
	directionalLightResource->Map(0, nullptr, reinterpret_cast<void**>(&directionalLightData));
	directionalLightData->color = { 1.0f,1.0f,1.0f,1.0f };
	directionalLightData->direction = { 0.0f,-1.0f,0.0f };
	directionalLightData->intensity = 1.0f;

	// Transform変数
	Transform& transform = object3D.GetTransform();

	// カメラの変数
	Transform cameraTransform = kDefaultCameraTransform;

	// uvTransform変数
	Transform uvTransformSprite{
		{1.0f,1.0f,1.0f},
		{0.0f,0.0f,0.0f},
		{0.0f,0.0f,0.0f},
	};

	// 球テクスチャ切り替え
	bool useMonsterBall = true;

	// デバッグカメラ
	DebugCamera debugCamera;
	debugCamera.Initialize();
	bool useDebugCamera = false;

	// ImGui初期化
#ifdef USE_IMGUI
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui::StyleColorsDark();
	ImGui_ImplWin32_Init(winApp.GetHwnd());
	ImGui_ImplDX12_Init(directXCommon.GetDevice(),
		directXCommon.GetSwapChainDesc().BufferCount,
		directXCommon.GetDXGIFormat(),
		textureManager.GetSrvDescriptorHeap(),
		textureManager.GetSrvHandleCPU(0),
		textureManager.GetSrvHandleGPU(0));
	ImGuiIO& io = ImGui::GetIO();
	io.Fonts->Build();
#endif
	MSG msg{};
	// ウィンドウの・ボタンが押されるまでループ
	while (msg.message != WM_QUIT) {
		// Windowにメッセージが来てたら最優先で処理させる
		if (PeekMessage(&msg, NULL, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
		} else {
		#ifdef USE_IMGUI
			ImGui_ImplDX12_NewFrame();
			ImGui_ImplWin32_NewFrame();
			ImGui::NewFrame();
		#endif // USE_IMGUI
			input.Update();
			// ゲームの処理
			Matrix4x4 cameraMatrix = MakeAffineMatrix(cameraTransform.scale, cameraTransform.rotate, cameraTransform.translate);
			Matrix4x4 viewMatrix = Inverse(cameraMatrix);
		#ifdef _DEBUG
			if (useDebugCamera) {
				debugCamera.Update(input.GetKey(), input.GetMouseState());
				viewMatrix = debugCamera.GetViewMatrix();
			}
		#endif
			Matrix4x4 projectionMatrix = MakePerspectiveFovMatrix(0.45f, float(kClientWidth) / float(kClientHeight), 0.1f, 100.0f);
			object3D.Update(viewMatrix, projectionMatrix);

			sprite.Update(kClientWidth,kClientHeight);
		#ifdef USE_IMGUI
			ImGui::Begin("Debug");
			
			ImGui::End();
			// ImGuiの内部コマンドを生成
			ImGui::Render();
		#endif
			
			directXCommon.PreDraw();
			// 描画用のDescriptorHeapの設定
			ID3D12DescriptorHeap* descriptorHeaps[] = {
				textureManager.GetSrvDescriptorHeap()
			};
			directXCommon.GetCommandList()->SetDescriptorHeaps(1, descriptorHeaps);
			// RootSignatureを設定。PSOに設定しているけど別途設定が必要
			directXCommon.GetCommandList()->SetGraphicsRootSignature(rootSignature.Get());
			directXCommon.GetCommandList()->SetPipelineState(graphicPipelineState.Get()); // PSOを設定		
			directXCommon.GetCommandList()->SetGraphicsRootConstantBufferView(3, directionalLightResource->GetGPUVirtualAddress());
			// 3D描画!
			object3D.Draw(directXCommon.GetCommandList(), &textureManager);
			// Spriteの描画
			sprite.Draw(directXCommon.GetCommandList(),&textureManager);
		#ifdef USE_IMGUI
			// 実際のcommandListのImGuiの描画コマンドを積む
			ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), directXCommon.GetCommandList());
		#endif

			directXCommon.PostDraw();
		}
	}
	// 解放
#ifdef USE_IMGUI
	ImGui_ImplDX12_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
#endif
	CoUninitialize();
	CloseWindow(winApp.GetHwnd());
	xAudio2.Reset();
	MFShutdown();

	CoUninitialize();

	return 0;
}

static LONG WINAPI ExportDump(EXCEPTION_POINTERS* exception) {
	// 時刻を取得して、時刻を名前に入れたファイルを作成。Dumpsディレクトリ以下に出力
	SYSTEMTIME time;
	GetLocalTime(&time);
	wchar_t filePath[MAX_PATH] = { 0 };
	// 作成失敗時は早期リターン
	if (!CreateDirectoryW(L"./Dumps", nullptr)) {

		if (GetLastError() != ERROR_ALREADY_EXISTS) {
			OutputDebugStringW(L"Failed to create Dumps directory.\n");
			return EXCEPTION_EXECUTE_HANDLER;
		}
	}

	StringCchPrintfW(filePath, MAX_PATH, L"./Dumps/%04d-%02d%02d-%02d%02d.dmp", time.wYear, time.wMonth, time.wDay, time.wHour, time.wMinute);
	HANDLE dumpFileHandle = CreateFileW(filePath, GENERIC_READ | GENERIC_WRITE, FILE_SHARE_WRITE | FILE_SHARE_READ, 0, CREATE_ALWAYS, 0, 0);
	// ファイルが作れなかったら早期リターン
	if (dumpFileHandle == INVALID_HANDLE_VALUE) {
		OutputDebugStringW(L"Failed to create dump file.\n");
		return EXCEPTION_EXECUTE_HANDLER;
	}

	// processId(このexeのid)とクラッシュ(例外)の発生したthreadIdを取得
	DWORD processId = GetCurrentProcessId();
	DWORD threadId = GetCurrentThreadId();
	// 設定情報を入力
	MINIDUMP_EXCEPTION_INFORMATION minidumpInformation{ 0 };
	minidumpInformation.ThreadId = threadId;
	minidumpInformation.ExceptionPointers = exception;
	minidumpInformation.ClientPointers = TRUE;
	BOOL isDumpSaved = MiniDumpWriteDump(
		GetCurrentProcess(), processId, dumpFileHandle,
		MiniDumpNormal, &minidumpInformation, nullptr, nullptr
	);
	// 成功失敗時のログ出力
	if (!isDumpSaved) {
		OutputDebugStringW(L"Failed to write minidump.\n");
	} else {
		OutputDebugStringW(L"Successfully saved minidump.\n");
	}

	// ハンドルのクローズ
	CloseHandle(dumpFileHandle);

	// ほかに関連付けられているSEH例外ハンドラがあれば実行。通常はプロセスを終了する
	return EXCEPTION_EXECUTE_HANDLER;
}

// =========================================================
// CreateBufferResource
// =========================================================
Microsoft::WRL::ComPtr<ID3D12Resource> CreateBufferResource(const Microsoft::WRL::ComPtr<ID3D12Device>& device, size_t sizeInBytes) {
	// 頂点リソース用のヒープの設定
	D3D12_HEAP_PROPERTIES uploadHeapProperties{};
	uploadHeapProperties.Type = D3D12_HEAP_TYPE_UPLOAD; // UploadHeapを使う
	// 頂点リソースの設定
	D3D12_RESOURCE_DESC resourceDesc{};
	// バッファリソース。テクスチャの場合はまた別の設定をする
	resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	resourceDesc.Width = sizeInBytes; // リソースのサイズ
	// バッファの場合はこれらを1にする決まり
	resourceDesc.Height = 1;
	resourceDesc.DepthOrArraySize = 1;
	resourceDesc.MipLevels = 1;
	resourceDesc.SampleDesc.Count = 1;
	// バッファの場合はこれにする決まり
	resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	// 実際にリソースを作る
	Microsoft::WRL::ComPtr<ID3D12Resource> resource;
	HRESULT hr = device->CreateCommittedResource(
		&uploadHeapProperties,
		D3D12_HEAP_FLAG_NONE,
		&resourceDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&resource)
	);
	assert(SUCCEEDED(hr));

	return resource;
}