#include "Object3D.h"
#include "AffineMatrix.h"
#include "Model.h"
#include <cassert>

void Object3D::Initialize(Model* model) {
	model_ = model;
	worldMatrix_ = MakeIdentityMatrix();
	wvpMatrix_ = MakeIdentityMatrix();

	if (model_) {
		model_->SetTransformationWorld(worldMatrix_);
		model_->SetTransformationWVP(wvpMatrix_);
	}
}

void Object3D::Update(const Matrix4x4& viewMatrix, const Matrix4x4& projectionMatrix) {
	assert(model_);

	worldMatrix_ = MakeAffineMatrix(transform_.scale, transform_.rotate, transform_.translate);
	wvpMatrix_ = Multiply(worldMatrix_, Multiply(viewMatrix, projectionMatrix));

	model_->SetTransformationWorld(worldMatrix_);
	model_->SetTransformationWVP(wvpMatrix_);
}

void Object3D::Draw(ID3D12GraphicsCommandList* commandList, TextureManager* textureManager) {
	assert(model_);
	model_->Draw(commandList, textureManager);
}
