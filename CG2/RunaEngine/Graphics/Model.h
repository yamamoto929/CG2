#pragma once
#include <d3d12.h>
#pragma comment(lib,"d3d12.lib")
#include <cstddef>
#include <optional>
#include <string>
#include <unordered_map>
#include <vector>
#include "TextureManager.h"
#include "ModelData.h"
#include "ConstantBuffer.h"
#include "Material.h"
#include "ModelDrawParameters.h"
namespace RunaEngine{
	class Model {
	private:
		Microsoft::WRL::ComPtr<ID3D12Resource> vertexResource_ = nullptr;
		D3D12_VERTEX_BUFFER_VIEW vertexBufferView_{};
		std::vector<MeshData> meshes_;
		std::unordered_map<std::string, uint32_t> materialTextureHandles_;
		std::unordered_map<std::string, MaterialData> materials_;
		FrameBuffer<Material> materialBuffers_;
		uint32_t fallbackTextureHandle_ = 0;
		std::optional<uint32_t> textureOverride_;
		uint32_t vertexCount_ = 0;
		uint32_t ResolveTextureHandle(const std::string& materialName) const;
		Microsoft::WRL::ComPtr<ID3D12Resource> CreateBufferResource(ID3D12Device* device, size_t sizeInBytes);
	public:
		void Initialize(ID3D12Device* device, TextureManager* textureManager, ModelData* modelData);
		void Draw(ID3D12GraphicsCommandList* commandList, TextureManager* textureManager,
			const ModelDrawParameters& parameters = {});
		void BeginFrame() { materialBuffers_.BeginFrame(); }
		bool UsesTexture(uint32_t handle) const;
		uint32_t GetTextureHandle() const { return textureOverride_.value_or(fallbackTextureHandle_); }
		void SetTextureHandle(uint32_t textureHandle) { textureOverride_ = textureHandle; }
		void ClearTextureOverride() { textureOverride_.reset(); }
		size_t GetMeshCount() const { return meshes_.size(); }
		size_t GetSubMeshCount() const;
		size_t GetMaterialCount() const { return materialTextureHandles_.size(); }
	};
}

