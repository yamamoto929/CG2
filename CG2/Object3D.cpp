#include "Object3D.h"
#include "AffineMatrix.h"
#include "Model.h"
#include <cassert>


void Object3D::Initialize(ID3D12Device* device, Model* model) {
	assert(device);
	assert(model);

	model_ = model;
	worldMatrix_ = MakeIdentityMatrix();
	wvpMatrix_ = MakeIdentityMatrix();

	transformationMatrix_.Initialize(device);
	transformationMatrixData_ = transformationMatrix_.GetData();

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
	assert(transformationMatrixData_);
	commandList->SetGraphicsRootConstantBufferView(
		1,
		transformationMatrix_.GetGPUVirtualAddress()
	);

	model_->Draw(commandList, textureManager);
}
