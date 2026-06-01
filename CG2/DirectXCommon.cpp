#include "DirectXCommon.h"
#include "ConvertString.h"
#include "Log.h"
#include <format>
void DirectXCommon::Initialize(HWND hwnd, int32_t width, int32_t height) {
	HRESULT hr = CreateDXGIFactory(IID_PPV_ARGS(&dxgiFactory_));
	// 初期化の根本的な部分でエラーが出た場合はプログラムが間違っているか、どうにもできない場合が多いのでassertにしておく
	assert(SUCCEEDED(hr));

	Microsoft::WRL::ComPtr<IDXGIAdapter4> useAdapter = nullptr;
	for (UINT i = 0;dxgiFactory_->EnumAdapterByGpuPreference(i, DXGI_GPU_PREFERENCE_HIGH_PERFORMANCE,
		IID_PPV_ARGS(&useAdapter)) != DXGI_ERROR_NOT_FOUND;++i) {

		// アダプタの情報を取得する
		DXGI_ADAPTER_DESC3 adapterDesc{};
		hr = useAdapter->GetDesc3(&adapterDesc);
		assert(SUCCEEDED(hr)); // 取得できないのは一大事
		// ソフトウェアアダプタでなければ採用!
		if (!(adapterDesc.Flags & DXGI_ADAPTER_FLAG3_SOFTWARE)) {
			// 採用したアダプタの情報をログに出力。wstringの方なので注意
			Log(std::format("Use Adapter:{}\n", ConvertString(adapterDesc.Description)));
			break;
		}
		useAdapter = nullptr; // ソフトェアアダプタの場合は見なかったことにする
	}

	// 適切なアダプタが見つからなかったので起動できない
	assert(useAdapter != nullptr);

	// 機能レベルとログ出力用の文字列
	D3D_FEATURE_LEVEL featureLevels[] = {
		D3D_FEATURE_LEVEL_12_2, D3D_FEATURE_LEVEL_12_1, D3D_FEATURE_LEVEL_12_0,
	};
	const char* featureLevelStrings[] = { "12.2","12.1","12.0" };
	// 高い順に生成できるか試していく
	for (size_t i = 0;i < _countof(featureLevels);++i) {
		// 採用したアダプターでデバイスを生成
		hr = D3D12CreateDevice(useAdapter.Get(), featureLevels[i], IID_PPV_ARGS(&device_));
		// 指定した機能レベルでデバイスが生成できたか確認
		if (SUCCEEDED(hr)) {
			// 生成できたのでログ出力を行ってループを抜ける
			Log(std::format("FeatureLevel:{}\n", featureLevelStrings[i]));
			break;
		}
	}
	// デバイスの生成がうまくいかなかったので起動できない
	assert(device_ != nullptr);
	Log("Complete create D3D12Device!!!\n");

	// コマンドキューを生成する
	D3D12_COMMAND_QUEUE_DESC commandQueueDesc{};
	hr = device_->CreateCommandQueue(&commandQueueDesc,
		IID_PPV_ARGS(&commandQueue_));
	// コマンドキューの生成がうまくいかなかったので生成できない
	assert(SUCCEEDED(hr));

	// コマンドアロケーターを生成する
	hr = device_->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT,
		IID_PPV_ARGS(&commandAllocator_));
	// コマンドアロケーターの生成がうまくいかなかったので生成できない
	assert(SUCCEEDED(hr));

	// コマンドリストを生成する
	hr = device_->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT,
		commandAllocator_.Get(), nullptr, IID_PPV_ARGS(&commandList_));
	// コマンドリストの生成がうまくいかなかったので生成できない
	assert(SUCCEEDED(hr));

	// スワップチェーンを生成する
	DXGI_SWAP_CHAIN_DESC1 swapChainDesc{};
	swapChainDesc.Width = width; // 画面の幅
	swapChainDesc.Height = height; // 画面の高さ
	swapChainDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM; // 色の形式
	swapChainDesc.SampleDesc.Count = 1; // マルチサンプルしない
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT; // 画面のターゲットとして再利用する
	swapChainDesc.BufferCount = kSwapChainBufferCount;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD; // モニタに移したら、中身を破棄
	// コマンドキュー、ウィンドウハンドル、設定を渡して生成する
	hr = dxgiFactory_->CreateSwapChainForHwnd(commandQueue_.Get(), hwnd, &swapChainDesc,
		nullptr, nullptr, reinterpret_cast<IDXGISwapChain1**>(swapChain_.GetAddressOf()));

	assert(SUCCEEDED(hr));

	// RTV用ディスクリプタヒープの生成
	rtvDescriptorHeap_.Initialize(device_.Get(), D3D12_DESCRIPTOR_HEAP_TYPE_RTV, kSwapChainBufferCount, false);

	// SwapChainからResourceを引っ張ってくる
	for (uint32_t i = 0; i < kSwapChainBufferCount; ++i) {
		hr = swapChain_->GetBuffer(i, IID_PPV_ARGS(&swapChainResources_[i]));
		assert(SUCCEEDED(hr));
	}

	CreateRenderTargetView();

	// 初期値0でFenceを作る
	uint64_t fenceValue = 0;
	hr = device_->CreateFence(fenceValue, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence_));
	assert(SUCCEEDED(hr));

	// FenceもSignalを待つためのイベントを作成する
	fenceEvent_ = CreateEvent(NULL, FALSE, FALSE, NULL);
	assert(fenceEvent_ != nullptr);
}

void DirectXCommon::CreateRenderTargetView() {
	D3D12_RENDER_TARGET_VIEW_DESC rtvDesc{};
	rtvDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM_SRGB;
	rtvDescFormat_ = rtvDesc.Format;
	rtvDesc.ViewDimension = D3D12_RTV_DIMENSION_TEXTURE2D;

	for (uint32_t i = 0; i < kSwapChainBufferCount; ++i) {
		rtvHandles_[i] =rtvDescriptorHeap_.GetCPUDescriptorHandle(
			device_->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV)
		);

		device_->CreateRenderTargetView(
			swapChainResources_[i].Get(),
			&rtvDesc,
			rtvHandles_[i]
		);
	}
}
