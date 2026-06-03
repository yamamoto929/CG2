#include "Object3D.h"
#include "AffineMatrix.h"
#include "Model.h"
#include <cassert>
#include <d3d12.h>
#pragma comment(lib,"d3d12.lib")


void Object3D::Initialize(ID3D12Device* device, Model* model) {
	assert(device);
	assert(model);

	model_ = model;
	worldMatrix_ = MakeIdentityMatrix();
	wvpMatrix_ = MakeIdentityMatrix();

	const size_t resourceSize = (sizeof(TransformationMatrix) + 0xff) & ~static_cast<size_t>(0xff);
	transformationMatrixResource_ = CreateBufferResource(device, resourceSize);
	HRESULT hr = transformationMatrixResource_->Map(0, nullptr, reinterpret_cast<void**>(&transformationMatrixData_));
	assert(SUCCEEDED(hr));

	transformationMatrixData_->World = worldMatrix_;
	transformationMatrixData_->WVP = wvpMatrix_;
}

void Object3D::Update(const Matrix4x4& viewMatrix, const Matrix4x4& projectionMatrix) {
	assert(model_);
	assert(transformationMatrixData_);

	worldMatrix_ = MakeAffineMatrix(transform_.scale, transform_.rotate, transform_.translate);
	wvpMatrix_ = Multiply(worldMatrix_, Multiply(viewMatrix, projectionMatrix));

	transformationMatrixData_->World = worldMatrix_;
	transformationMatrixData_->WVP = wvpMatrix_;
}

void Object3D::Draw(ID3D12GraphicsCommandList* commandList, TextureManager* textureManager) {
	assert(model_);
	assert(transformationMatrixResource_);
	commandList->SetGraphicsRootConstantBufferView(
		1,
		transformationMatrixResource_->GetGPUVirtualAddress()
	);

	model_->Draw(commandList, textureManager);
}

Microsoft::WRL::ComPtr<ID3D12Resource> Object3D::CreateBufferResource(ID3D12Device* device, size_t sizeInBytes) {
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
