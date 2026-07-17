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
private:

	DescriptorHeap srvDescriptorHeap_;
	ID3D12Device* device_ = nullptr;
	ID3D12GraphicsCommandList* commandList_ = nullptr;

	uint32_t nextIndex_ = 0;
	std::unordered_map<std::string, uint32_t> textureHandles_;
	std::vector<Microsoft::WRL::ComPtr<ID3D12Resource>> textureResources_;
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

