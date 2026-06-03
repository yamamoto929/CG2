#include <Windows.h>
#include <cstdint>
#include <string>
#include <vector>
#include <wrl.h>

#include <d3d12.h>
#pragma comment(lib,"d3d12.lib")

#include <xaudio2.h>
#pragma comment(lib,"xaudio2.lib")

#include <mfapi.h>
#include <mfidl.h>
#include <mfreadwrite.h>
#pragma comment(lib,"Mf.lib")
#pragma comment(lib,"mfplat.lib")
#pragma comment(lib,"Mfreadwrite.lib")
#pragma comment(lib,"mfuuid.lib")

#define DIRECTINPUT_VERSION 0X0800
#include <dinput.h>
#pragma comment(lib,"dinput8.lib")
#pragma comment(lib,"dxguid.lib")

#include "AffineMatrix.h"
#include "CrashHandler.h"
#include "D3DResourceLeakChecker.h"
#include "DebugCamera.h"
#include "DirectXCommon.h"
#include "DirectXDebug.h"
#include "DirectionalLight.h"
#include "GraphicsPipeline.h"
#include "Input.h"
#include "Log.h"
#include "Object3D.h"
#include "Renderer.h"
#include "ResourceManager.h"
#include "ShaderCompiler.h"
#include "Sprite.h"
#include "TextureManager.h"
#include "Transform.h"
#include "WinApp.h"
#include "WVPMatrix.h"

#ifdef USE_IMGUI
#include "externals/imgui/imgui.h"
#include "externals/imgui/imgui_impl_dx12.h"
#include "externals/imgui/imgui_impl_win32.h"
extern IMGUI_IMPL_API LRESULT ImGui_ImplWin32_WndProcHandler(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);
#endif

const Transform kDefaultCameraTransform{
	{ 1.0f, 1.0f, 1.0f },
	{ 0.0f, 0.0f, 0.0f },
	{ 0.0f, 0.0f, -10.0f }
};

int WINAPI WinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPSTR, _In_ int) {
	D3DResourceLeakChecker leakChecker;
	CoInitializeEx(nullptr, COINIT_MULTITHREADED);
	MFStartup(MF_VERSION, MFSTARTUP_NOSOCKET);
	InitLog();
	CrashHandler::Initialize();

	const int32_t kClientWidth = 1280;
	const int32_t kClientHeight = 720;

	WinApp winApp;
	winApp.CreateNewWindow(kClientWidth, kClientHeight, "CG2");

	DirectXDebug::EnableDebugLayer();

	DirectXCommon directXCommon;
	directXCommon.Initialize(winApp.GetHwnd(), kClientWidth, kClientHeight);
	DirectXDebug::SetupInfoQueue();

	TextureManager textureManager;
	textureManager.Initialize(directXCommon.GetDevice(), directXCommon.GetCommandList(), 128);

	Input input;
	input.Initialize(winApp.GetHInstance(), winApp.GetHwnd());

	Microsoft::WRL::ComPtr<IXAudio2> xAudio2;
	IXAudio2MasteringVoice* masterVoice = nullptr;
	HRESULT hr = XAudio2Create(xAudio2.GetAddressOf(), 0, XAUDIO2_DEFAULT_PROCESSOR);
	hr = xAudio2->CreateMasteringVoice(&masterVoice);

	std::wstring path = L"Resources/Alarm01.wav";
	IMFSourceReader* mfSourceReader = nullptr;
	MFCreateSourceReaderFromURL(path.c_str(), nullptr, &mfSourceReader);

	IMFMediaType* mfMediaType = nullptr;
	MFCreateMediaType(&mfMediaType);
	mfMediaType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Audio);
	mfMediaType->SetGUID(MF_MT_SUBTYPE, MFAudioFormat_PCM);
	mfSourceReader->SetCurrentMediaType(MF_SOURCE_READER_FIRST_AUDIO_STREAM, nullptr, mfMediaType);

	mfMediaType->Release();
	mfMediaType = nullptr;
	mfSourceReader->GetCurrentMediaType(MF_SOURCE_READER_FIRST_AUDIO_STREAM, &mfMediaType);

	WAVEFORMATEX* waveFormat = nullptr;
	MFCreateWaveFormatExFromMFMediaType(mfMediaType, &waveFormat, nullptr);

	std::vector<BYTE> mediaData;
	while (true) {
		IMFSample* mfSample = nullptr;
		DWORD streamFlags = 0;
		mfSourceReader->ReadSample(MF_SOURCE_READER_FIRST_AUDIO_STREAM, 0, nullptr, &streamFlags, nullptr, &mfSample);

		if (streamFlags & MF_SOURCE_READERF_ENDOFSTREAM) {
			break;
		}

		if (mfSample) {
			IMFMediaBuffer* mfMediaBuffer = nullptr;
			mfSample->ConvertToContiguousBuffer(&mfMediaBuffer);

			BYTE* buffer = nullptr;
			DWORD currentLength = 0;
			mfMediaBuffer->Lock(&buffer, nullptr, &currentLength);

			mediaData.resize(mediaData.size() + currentLength);
			memcpy(mediaData.data() + mediaData.size() - currentLength, buffer, currentLength);

			mfMediaBuffer->Unlock();
			mfMediaBuffer->Release();
			mfSample->Release();
		}
	}

	IXAudio2SourceVoice* sourceVoice = nullptr;
	xAudio2->CreateSourceVoice(&sourceVoice, waveFormat);

	XAUDIO2_BUFFER audioBuffer{};
	audioBuffer.pAudioData = mediaData.data();
	audioBuffer.Flags = XAUDIO2_END_OF_STREAM;
	audioBuffer.AudioBytes = sizeof(BYTE) * static_cast<UINT32>(mediaData.size());
	sourceVoice->SubmitSourceBuffer(&audioBuffer);
	sourceVoice->Start(0);

	mfMediaType->Release();
	mfSourceReader->Release();
	CoTaskMemFree(waveFormat);

	ShaderCompiler shaderCompiler;
	shaderCompiler.Initialize();

	GraphicsPipeline graphicsPipeline;
	graphicsPipeline.Initialize(
		directXCommon.GetDevice(),
		&shaderCompiler,
		directXCommon.GetDXGIFormat(),
		DXGI_FORMAT_D24_UNORM_S8_UINT
	);

	ResourceManager resourceManager;
	Model* model = resourceManager.LoadModel(
		directXCommon.GetDevice(),
		&textureManager,
		"resources",
		"axis.obj"
	);

	Object3D object3D;
	object3D.Initialize(directXCommon.GetDevice(), model);

	Sprite sprite;
	sprite.Initialize(
		directXCommon.GetDevice(),
		&textureManager,
		"./resources/uvChecker.png"
	);

	DirectionalLight directionalLight;
	directionalLight.Initialize(directXCommon.GetDevice());

	Renderer renderer;
	renderer.Initialize(&directXCommon, &textureManager, &graphicsPipeline, &directionalLight);

	Transform cameraTransform = kDefaultCameraTransform;

	DebugCamera debugCamera;
	debugCamera.Initialize();
	bool useDebugCamera = false;

#ifdef USE_IMGUI
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui::StyleColorsDark();
	ImGui_ImplWin32_Init(winApp.GetHwnd());
	ImGui_ImplDX12_Init(
		directXCommon.GetDevice(),
		directXCommon.GetSwapChainDesc().BufferCount,
		directXCommon.GetDXGIFormat(),
		textureManager.GetSrvDescriptorHeap(),
		textureManager.GetSrvHandleCPU(0),
		textureManager.GetSrvHandleGPU(0)
	);
	ImGuiIO& io = ImGui::GetIO();
	io.Fonts->Build();
#endif

	MSG msg{};
	while (msg.message != WM_QUIT) {
		if (PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
			TranslateMessage(&msg);
			DispatchMessage(&msg);
			continue;
		}

#ifdef USE_IMGUI
		ImGui_ImplDX12_NewFrame();
		ImGui_ImplWin32_NewFrame();
		ImGui::NewFrame();
#endif

		input.Update();

		Matrix4x4 cameraMatrix = MakeAffineMatrix(
			cameraTransform.scale,
			cameraTransform.rotate,
			cameraTransform.translate
		);
		Matrix4x4 viewMatrix = Inverse(cameraMatrix);

#ifdef _DEBUG
		if (useDebugCamera) {
			debugCamera.Update(input.GetKey(), input.GetMouseState());
			viewMatrix = debugCamera.GetViewMatrix();
		}
#endif

		Matrix4x4 projectionMatrix = MakePerspectiveFovMatrix(
			0.45f,
			float(kClientWidth) / float(kClientHeight),
			0.1f,
			100.0f
		);

		object3D.Update(viewMatrix, projectionMatrix);
		sprite.Update(kClientWidth, kClientHeight);

#ifdef USE_IMGUI
		ImGui::Begin("Debug");
		ImGui::Checkbox("useDebugCamera", &useDebugCamera);
		ImGui::End();
		ImGui::Render();
#endif

		renderer.Begin();
		renderer.Draw(object3D);
		renderer.Draw(sprite);

#ifdef USE_IMGUI
		ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), renderer.GetCommandList());
#endif

		renderer.End();
	}

#ifdef USE_IMGUI
	ImGui_ImplDX12_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
#endif

	if (sourceVoice) {
		sourceVoice->DestroyVoice();
	}
	if (masterVoice) {
		masterVoice->DestroyVoice();
	}

	xAudio2.Reset();
	MFShutdown();
	CoUninitialize();
	CloseWindow(winApp.GetHwnd());

	return 0;
}
