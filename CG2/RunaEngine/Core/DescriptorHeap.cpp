#include "DescriptorHeap.h"
#include <cassert>
#include "EngineError.h"
void DescriptorHeap::Initialize(ID3D12Device* device, D3D12_DESCRIPTOR_HEAP_TYPE heapType, uint32_t numDescriptors, bool shaderVisible) {
	Require(device && numDescriptors > 0, "DescriptorHeap: device or capacity is invalid");
	D3D12_DESCRIPTOR_HEAP_DESC descriptorHeapDesc{};
	descriptorHeapDesc.Type = heapType;
	descriptorHeapDesc.NumDescriptors = numDescriptors;
	descriptorHeapDesc.Flags = shaderVisible ? D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE : D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

	descriptorSize_ = device->GetDescriptorHandleIncrementSize(heapType);
	HRESULT hr = device->CreateDescriptorHeap(&descriptorHeapDesc, IID_PPV_ARGS(&descriptorHeap_));
	// ディスクリプタヒープが作れなかったので起動できない
	CheckHR(hr, "CreateDescriptorHeap");
	numDescriptors_ = numDescriptors;
	shaderVisible_ = shaderVisible;
}

// =========================================================
// GetCPUDescriptorHandle
// =========================================================
D3D12_CPU_DESCRIPTOR_HANDLE DescriptorHeap::GetCPUDescriptorHandle(uint32_t index) {
	Require(descriptorHeap_ && index < numDescriptors_, "CPU descriptor index out of range: " + std::to_string(index));
	D3D12_CPU_DESCRIPTOR_HANDLE handleCPU = descriptorHeap_->GetCPUDescriptorHandleForHeapStart();
	handleCPU.ptr += (UINT64(descriptorSize_) * UINT64(index));
	return handleCPU;
}

// =========================================================
// GetGPUDescriptorHandle
// =========================================================
D3D12_GPU_DESCRIPTOR_HANDLE DescriptorHeap::GetGPUDescriptorHandle(uint32_t index) {
	Require(descriptorHeap_ && shaderVisible_ && index < numDescriptors_, "GPU descriptor index out of range: " + std::to_string(index));
	D3D12_GPU_DESCRIPTOR_HANDLE handleGPU = descriptorHeap_->GetGPUDescriptorHandleForHeapStart();
	handleGPU.ptr += (UINT64(descriptorSize_) * UINT64(index));
	return handleGPU;
}
