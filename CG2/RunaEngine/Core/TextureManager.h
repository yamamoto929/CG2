#pragma once
#include <d3d12.h>
#pragma comment(lib,"d3d12.lib")
#include <wrl.h>
#include<cstdint>
#include <unordered_map>
#include <string>
#include "externals\DirectXTex\DirectXTex.h"
#include "externals\DirectXTex\d3dx12.h"
#include "DescriptorHeap.h"
#include "TextureSize.h"
class TextureManager{
public:
	void Initialize(ID3D12Device* device, ID3D12GraphicsCommandList* commandList, uint32_t maxTextureCount);

	uint32_t Load(const std::string& filePath);

	D3D12_GPU_DESCRIPTOR_HANDLE GetSrvHandleGPU(uint32_t textureHandle);

	ID3D12DescriptorHeap* GetSrvDescriptorHeap() const;
	D3D12_CPU_DESCRIPTOR_HANDLE GetSrvHandleCPU(uint32_t index);
	TextureSize GetTextureSize(uint32_t textureHandle)const;
	void ReleaseUploadResources(); // GPU完了後だけ呼ぶ
	bool HasPendingUploads() const { return !intermediateResources_.empty(); }
	void Unload(uint32_t handle); // GPU完了後、参照がなくなった画像だけ
	void Clear(); // 0番のImGui用ディスクリプタは残す
	void Shutdown() { Clear(); srvDescriptorHeap_.Reset(); device_ = nullptr; commandList_ = nullptr; }
	size_t GetLoadedTextureCount() const { return textureResources_.size(); }
	size_t GetPendingUploadCount() const { return intermediateResources_.size(); }
private:

	DescriptorHeap srvDescriptorHeap_;
	ID3D12Device* device_ = nullptr;
	ID3D12GraphicsCommandList* commandList_ = nullptr;

	uint32_t nextIndex_ = 1;
	uint32_t maxTextureCount_ = 0;
	uint32_t nextDescriptorIndex_ = 1;
	std::vector<uint32_t> freeDescriptorIndices_;
	std::unordered_map<uint32_t, uint32_t> descriptorIndices_;
	std::unordered_map<std::string, uint32_t> textureHandles_;
	std::unordered_map<uint32_t, Microsoft::WRL::ComPtr<ID3D12Resource>> textureResources_;
	std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>> intermediateResources_;

	DirectX::ScratchImage LoadTexture(const std::string& filePath);
	
	Microsoft::WRL::ComPtr<ID3D12Resource> UploadTextureData(
		const Microsoft::WRL::ComPtr<ID3D12Resource>& texture,
		const DirectX::ScratchImage& mipImages);
	Microsoft::WRL::ComPtr<ID3D12Resource> CreateTextureResource( const DirectX::TexMetadata& metadata);
	Microsoft::WRL::ComPtr<ID3D12Resource> CreateBufferResource( size_t sizeInBytes);

	// テクスチャサイズのデータ
	std::unordered_map<uint32_t, TextureSize> textureSizes_;

};

