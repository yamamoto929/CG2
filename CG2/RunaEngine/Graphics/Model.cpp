#include "Model.h"
#include <cassert>
#include <cstring>
#include <utility>

namespace RunaEngine{
	void Model::Initialize(ID3D12Device* device, TextureManager* textureManager, ModelData* modelData) {
		assert(device);
		assert(textureManager);
		assert(modelData);
		assert(!modelData->vertices.empty());

		vertexResource_ = CreateBufferResource(device, sizeof(VertexData) * modelData->vertices.size());
		// 頂点バッファビューを作成
		vertexBufferView_.BufferLocation = vertexResource_->GetGPUVirtualAddress();
		vertexBufferView_.SizeInBytes = UINT(sizeof(VertexData) * modelData->vertices.size());
		vertexBufferView_.StrideInBytes = sizeof(VertexData);

		// 頂点リソースにデータを書き込む
		VertexData* vertexData = nullptr;
		vertexResource_->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));
		std::memcpy(vertexData, modelData->vertices.data(), sizeof(VertexData) * modelData->vertices.size());

		fallbackTextureHandle_ =
			textureManager->Load("RunaEngine/Graphics/Resources/white.png");
		materialTextureHandles_.clear();
		for (const auto& [materialName, material] : modelData->materials) {
			uint32_t textureHandle = fallbackTextureHandle_;
			if (!material.textureFilePath.empty()) {
				textureHandle = textureManager->Load(material.textureFilePath);
			}
			materialTextureHandles_[materialName] = textureHandle;
		}

		vertexCount_ = static_cast<uint32_t>(modelData->vertices.size());
		meshes_ = modelData->meshes;
		if (meshes_.empty()) {
			MeshData defaultMesh{};
			defaultMesh.name = "default";
			defaultMesh.subMeshes.push_back({ "", 0, vertexCount_ });
			meshes_.push_back(std::move(defaultMesh));
		}
	}

	void Model::Draw(ID3D12GraphicsCommandList* commandList, TextureManager* textureManager) {
		assert(commandList);
		assert(textureManager);

		commandList->IASetVertexBuffers(0, 1, &vertexBufferView_);
		commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		for (const MeshData& mesh : meshes_) {
			for (const SubMeshData& subMesh : mesh.subMeshes) {
				const uint32_t textureHandle =
					ResolveTextureHandle(subMesh.materialName);
				commandList->SetGraphicsRootDescriptorTable(
					2,
					textureManager->GetSrvHandleGPU(textureHandle)
				);
				commandList->DrawInstanced(
					subMesh.vertexCount,
					1,
					subMesh.firstVertex,
					0
				);
			}
		}
	}

	uint32_t Model::ResolveTextureHandle(const std::string& materialName) const {
		if (textureOverride_) {
			return *textureOverride_;
		}

		const auto material = materialTextureHandles_.find(materialName);
		if (material != materialTextureHandles_.end()) {
			return material->second;
		}

		if (materialName.empty() && materialTextureHandles_.size() == 1) {
			return materialTextureHandles_.begin()->second;
		}

		return fallbackTextureHandle_;
	}

	size_t Model::GetSubMeshCount() const {
		size_t subMeshCount = 0;
		for (const MeshData& mesh : meshes_) {
			subMeshCount += mesh.subMeshes.size();
		}
		return subMeshCount;
	}

	// =========================================================
	// CreateBufferResource
	// =========================================================
	Microsoft::WRL::ComPtr<ID3D12Resource> Model::CreateBufferResource(ID3D12Device* device, size_t sizeInBytes) {
		// 頂点リソース用のヒープの設定
		D3D12_HEAP_PROPERTIES uploadHeapProperties{};
		uploadHeapProperties.Type = D3D12_HEAP_TYPE_UPLOAD; // UploadHeapを使う
		// 頂点リソースの設定
		D3D12_RESOURCE_DESC resourceDesc{};
		// バッファリソース。テクスチャの場合はまた別の設定をする
		resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
		resourceDesc.Width = sizeInBytes; // リソースのサイズ
		// バッファの場合はこれらを1にする決まり
		resourceDesc.Height = 1;
		resourceDesc.DepthOrArraySize = 1;
		resourceDesc.MipLevels = 1;
		resourceDesc.SampleDesc.Count = 1;
		// バッファの場合はこれにする決まり
		resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
		// 実際にリソースを作る
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
}
