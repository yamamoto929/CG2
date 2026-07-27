#pragma once
#include <d3d12.h>
#pragma comment(lib,"d3d12.lib")
#include <string>
#include "TextureManager.h"
#include "ModelData.h"
namespace RunaEngine{
	class Model {
	private:
		Microsoft::WRL::ComPtr<ID3D12Resource> vertexResource_ = nullptr;
		D3D12_VERTEX_BUFFER_VIEW vertexBufferView_{};
		uint32_t textureHandle_ = 0;
		uint32_t vertexCount_ = 0;
		Microsoft::WRL::ComPtr<ID3D12Resource> CreateBufferResource(ID3D12Device* device, size_t sizeInBytes);
	public:
		void Initialize(ID3D12Device* device, TextureManager* textureManager, ModelData* modelData);
		void Draw(ID3D12GraphicsCommandList* commandList, TextureManager* textureManager);
		uint32_t GetTextureHandle() const { return textureHandle_; }
		void SetTextureHandle(uint32_t textureHandle) { textureHandle_ = textureHandle; }
	};
}

