#pragma once
#include <cassert>
#include <cstddef>
#include <d3d12.h>
#include <wrl.h>
#pragma comment(lib, "d3d12.lib")

template <class T>
class ConstantBuffer {
public:
	void Initialize(ID3D12Device* device) {
		assert(device);

		resource_ = CreateBufferResource(device, AlignConstantBufferSize(sizeof(T)));
		resource_->Map(0, nullptr, reinterpret_cast<void**>(&data_));
		*data_ = {};
	}

	T* GetData() const { return data_; }

	D3D12_GPU_VIRTUAL_ADDRESS GetGPUVirtualAddress() const {
		assert(resource_);
		return resource_->GetGPUVirtualAddress();
	}

private:
	static size_t AlignConstantBufferSize(size_t size) {
		return (size + 0xff) & ~size_t(0xff);
	}

	static Microsoft::WRL::ComPtr<ID3D12Resource> CreateBufferResource(ID3D12Device* device, size_t sizeInBytes) {
		D3D12_HEAP_PROPERTIES uploadHeapProperties{};
		uploadHeapProperties.Type = D3D12_HEAP_TYPE_UPLOAD;

		D3D12_RESOURCE_DESC resourceDesc{};
		resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		resourceDesc.Width = sizeInBytes;
		resourceDesc.Height = 1;
		resourceDesc.DepthOrArraySize = 1;
		resourceDesc.MipLevels = 1;
		resourceDesc.SampleDesc.Count = 1;
		resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;

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

	Microsoft::WRL::ComPtr<ID3D12Resource> resource_;
	T* data_ = nullptr;
};
