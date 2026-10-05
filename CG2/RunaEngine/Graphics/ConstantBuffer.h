#pragma once
#include <cassert>
#include <cstddef>
#include <d3d12.h>
#include <wrl.h>
#include "EngineError.h"
#include <memory>
#include <vector>
#pragma comment(lib, "d3d12.lib")

template <class T>
class ConstantBuffer {
public:
	void Initialize(ID3D12Device* device) {
		Require(device != nullptr, "ConstantBuffer: device is null");

		resource_ = CreateBufferResource(device, AlignConstantBufferSize(sizeof(T)));
		const D3D12_RANGE readRange{0, 0};
		CheckHR(resource_->Map(0, &readRange, reinterpret_cast<void**>(&data_)), "ConstantBuffer::Map");
		*data_ = {};
	}

	T* GetData() const { return data_; }

	D3D12_GPU_VIRTUAL_ADDRESS GetGPUVirtualAddress() const {
		Require(resource_ != nullptr, "ConstantBuffer is not initialized");
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
		CheckHR(hr, "ConstantBuffer::CreateCommittedResource");
		return resource;
	}

	Microsoft::WRL::ComPtr<ID3D12Resource> resource_;
	T* data_ = nullptr;
};

// 1回の描画ごとに別の保存場所へコピーする。GPU完了後だけBeginFrameで再利用する。
template <class T>
class FrameBuffer {
public:
    void Initialize(ID3D12Device* device) {
        Require(device != nullptr, "FrameBuffer: device is null");
        buffers_.clear(); next_ = 0; device_ = device;
    }
    void BeginFrame() { next_ = 0; }
    void Clear() { buffers_.clear(); next_ = 0; device_ = nullptr; }
    D3D12_GPU_VIRTUAL_ADDRESS Write(const T& value) {
        Require(device_ != nullptr, "FrameBuffer is not initialized");
        if (next_ == buffers_.size()) {
            auto buffer = std::make_unique<ConstantBuffer<T>>();
            buffer->Initialize(device_);
            buffers_.push_back(std::move(buffer));
        }
        auto& buffer = *buffers_[next_++];
        *buffer.GetData() = value;
        return buffer.GetGPUVirtualAddress();
    }
private:
    ID3D12Device* device_ = nullptr;
    size_t next_ = 0;
    std::vector<std::unique_ptr<ConstantBuffer<T>>> buffers_;
};
