#pragma once
#include <Windows.h>
#include <d3d12.h>
#pragma comment(lib,"d3d12.lib")
#include <dxgi1_6.h>
#pragma comment(lib,"dxgi.lib")
#include <cassert>
#include <wrl.h>
#include <array>
#include <cstdint>
#include "DescriptorHeap.h"

class DirectXCommon {
private:
	Microsoft::WRL::ComPtr<IDXGIFactory7> dxgiFactory_ = nullptr;
	Microsoft::WRL::ComPtr<ID3D12Device> device_ = nullptr;
	Microsoft::WRL::ComPtr<ID3D12CommandQueue> commandQueue_ = nullptr;
	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> commandAllocator_ = nullptr;
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList_ = nullptr;
	Microsoft::WRL::ComPtr<IDXGISwapChain4> swapChain_ = nullptr;

	DescriptorHeap rtvDescriptorHeap_;
	static const uint32_t kSwapChainBufferCount = 2;
	std::array<Microsoft::WRL::ComPtr<ID3D12Resource>, kSwapChainBufferCount> swapChainResources_;
	std::array<D3D12_CPU_DESCRIPTOR_HANDLE, kSwapChainBufferCount> rtvHandles_;

	Microsoft::WRL::ComPtr<ID3D12Fence> fence_ = nullptr;
	void CreateRenderTargetView();
	HANDLE fenceEvent_;

	DXGI_FORMAT rtvDescFormat_;

	D3D12_VIEWPORT viewport_{};

	D3D12_RECT scissorRect_{};

	DescriptorHeap dsvDescriptorHeap_;
	Microsoft::WRL::ComPtr<ID3D12Resource> depthStencilResource_ = nullptr;
	D3D12_CPU_DESCRIPTOR_HANDLE dsvHandle_;
	Microsoft::WRL::ComPtr<ID3D12Resource>CreateDepthStencilTextureResource(int32_t width, int32_t height);

	UINT backBufferIndex_ = 0;
	uint64_t fenceValue_ = 0;
public:
	void Initialize(HWND hwnd, int32_t width, int32_t height);
	void PreDraw();
	void PostDraw();
	DXGI_FORMAT GetDXGIFormat()const { return rtvDescFormat_; }
	ID3D12GraphicsCommandList* GetCommandList() const { return commandList_.Get(); }
	ID3D12Device* GetDevice() const { return device_.Get(); }
};

