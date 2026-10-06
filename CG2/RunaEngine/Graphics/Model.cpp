#include "Model.h"
#include <cassert>
#include <cstring>
#include <utility>

namespace RunaEngine{
	void Model::Initialize(ID3D12Device* device, TextureManager* textureManager, ModelData* modelData) {

		materials_ = modelData->materials;
		materialBuffers_.Initialize(device);

		vertexResource_ = CreateBufferResource(device, sizeof(VertexData) * modelData->vertices.size());
		// 頂点バッファビューを作成
		vertexBufferView_.BufferLocation = vertexResource_->GetGPUVirtualAddress();
		vertexBufferView_.SizeInBytes = UINT(sizeof(VertexData) * modelData->vertices.size());
		vertexBufferView_.StrideInBytes = sizeof(VertexData);

		// 頂点リソースにデータを書き込む
		VertexData* vertexData = nullptr;
		HRESULT hr = vertexResource_->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));
		assert(SUCCEEDED(hr));
		std::memcpy(vertexData, modelData->vertices.data(), sizeof(VertexData) * modelData->vertices.size());
		vertexResource_->Unmap(0, nullptr);

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

	void Model::Draw(ID3D12GraphicsCommandList* commandList, TextureManager* textureManager,
		const ModelDrawParameters& parameters) {

		commandList->IASetVertexBuffers(0, 1, &vertexBufferView_);
		commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		for (const MeshData& mesh : meshes_) {
			for (const SubMeshData& subMesh : mesh.subMeshes) {
				Material material{};
				material.color = parameters.color;
				if (const auto found = materials_.find(subMesh.materialName); found != materials_.end()) {
					const auto& color = found->second.color;
					material.color = {material.color.x * color.x, material.color.y * color.y,
						material.color.z * color.z, material.color.w * color.w};
				}
				material.lightingMode = parameters.lightingMode;
				material.uvTransform = parameters.uvTransform;
				commandList->SetGraphicsRootConstantBufferView(0, materialBuffers_.Write(material));
				const uint32_t textureHandle =
					parameters.textureOverride.value_or(ResolveTextureHandle(subMesh.materialName));
				if (textureHandle == 0) { continue; }
				const auto texture = textureManager->GetSrvHandleGPU(textureHandle);
				if (texture.ptr == 0) { continue; }
				commandList->SetGraphicsRootDescriptorTable(
					2,
					texture
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

	bool Model::UsesTexture(uint32_t handle) const {
		if (handle == fallbackTextureHandle_ || textureOverride_ == handle) { return true; }
		for (const auto& entry : materialTextureHandles_) {
			if (entry.second == handle) { return true; }
		}
		return false;
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
