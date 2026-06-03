#include "Model.h"
#include <cassert>
#include <cstring>

void Model::Initialize(ID3D12Device* device, TextureManager* textureManager, ModelData* modelData) {
	vertexResource_ = CreateBufferResource(device, sizeof(VertexData) * modelData->vertices.size());
	// 頂点バッファビューを作成
	vertexBufferView_.BufferLocation = vertexResource_->GetGPUVirtualAddress();
	vertexBufferView_.SizeInBytes = UINT(sizeof(VertexData) * modelData->vertices.size());
	vertexBufferView_.StrideInBytes = sizeof(VertexData);

	// 頂点リソースにデータを書き込む
	VertexData* vertexData = nullptr;
	vertexResource_->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));
	std::memcpy(vertexData, modelData->vertices.data(), sizeof(VertexData) * modelData->vertices.size());

	// マテリアル用のリソースを作る
	materialResource_ = CreateBufferResource(device, sizeof(Material));
	materialResource_->Map(0, nullptr, reinterpret_cast<void**>(&materialData_));
	materialData_->color = { 1.0f, 1.0f, 1.0f, 1.0f };
	materialData_->enableLighting = true;
	materialData_->uvTransform = MakeIdentityMatrix();

	textureHandle_ = textureManager->Load(modelData->material.textureFilePath);

	vertexCount_ = static_cast<uint32_t>(modelData->vertices.size());
}

void Model::Draw(ID3D12GraphicsCommandList* commandList, TextureManager* textureManager) {
	commandList->IASetVertexBuffers(0, 1, &vertexBufferView_);
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	commandList->SetGraphicsRootConstantBufferView(0, materialResource_->GetGPUVirtualAddress());
	commandList->SetGraphicsRootDescriptorTable(2, textureManager->GetSrvHandleGPU(textureHandle_));

	commandList->DrawInstanced(vertexCount_, 1, 0, 0);
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