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

class DirectXCommon {
private:
	Microsoft::WRL::ComPtr<IDXGIFactory7> dxgiFactory_ = nullptr;
	Microsoft::WRL::ComPtr<ID3D12Device> device_ = nullptr;
	Microsoft::WRL::ComPtr<ID3D12CommandQueue> commandQueue_ = nullptr;
	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> commandAllocator_ = nullptr;
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList_ = nullptr;
	Microsoft::WRL::ComPtr<IDXGISwapChain4> swapChain_ = nullptr;

	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> rtvDescriptorHeap_ = nullptr;
	static const uint32_t kSwapChainBufferCount = 2;
	std::array<Microsoft::WRL::ComPtr<ID3D12Resource>, kSwapChainBufferCount> swapChainResources_;
	std::array<D3D12_CPU_DESCRIPTOR_HANDLE, kSwapChainBufferCount> rtvHandles_;

	Microsoft::WRL::ComPtr<ID3D12Fence> fence_ = nullptr;
	void CreateRenderTargetView();
	HANDLE fenceEvent_;

	DXGI_FORMAT rtvDescFormat_;
public:
	void Initialize(HWND hwnd, int32_t width, int32_t height);
	DXGI_FORMAT GetDXGIFormat()const { return rtvDescFormat_; }
};

