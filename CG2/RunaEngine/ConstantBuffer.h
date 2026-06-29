#pragma once
#include <cassert>
#include <cstddef>
#include <d3d12.h>
#pragma comment(lib,"d3d12.lib")
#include <wrl.h>

template <class T>
class ConstantBuffer {
public:
	void Initialize(ID3D12Device* device) {
		assert(device);

		const size_t sizeInBytes = (sizeof(T) + 0xff) & ~static_cast<size_t>(0xff);
		resource_ = CreateBufferResource(device, sizeInBytes);

		HRESULT hr = resource_->Map(0, nullptr, reinterpret_cast<void**>(&data_));
		assert(SUCCEEDED(hr));
	}

	T* GetData() { return data_; }
	const T* GetData() const { return data_; }

	ID3D12Resource* GetResource() const { return resource_.Get(); }

	D3D12_GPU_VIRTUAL_ADDRESS GetGPUVirtualAddress() const {
		assert(resource_);
		return resource_->GetGPUVirtualAddress();
	}

private:
	Microsoft::WRL::ComPtr<ID3D12Resource> CreateBufferResource(ID3D12Device* device, size_t sizeInBytes) {
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
