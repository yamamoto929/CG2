#include "Object3D.h"
#include "AffineMatrix.h"
#include "Model.h"
#include <cassert>

namespace RunaEngine {
	void Object3D::Initialize(ID3D12Device* device, RunaEngine::Model* model) {

		model_ = model;
		worldMatrix_ = MakeIdentityMatrix();
		wvpMatrix_ = MakeIdentityMatrix();

		transformationMatrix_.Initialize(device);
		transformationMatrixData_.World = worldMatrix_;
		transformationMatrixData_.WVP = wvpMatrix_;
		transformationMatrixData_.WorldInverseTranspose = MakeIdentityMatrix();
	}

	void Object3D::Update(const Matrix4x4& viewMatrix, const Matrix4x4& projectionMatrix) {

		worldMatrix_ = MakeAffineMatrix(transform_.scale, transform_.rotate, transform_.translate);
		wvpMatrix_ = Multiply(worldMatrix_, Multiply(viewMatrix, projectionMatrix));

		transformationMatrixData_.World = worldMatrix_;
		transformationMatrixData_.WVP = wvpMatrix_;
		transformationMatrixData_.WorldInverseTranspose = MakeNormalMatrix(worldMatrix_);
	}

	void Object3D::Draw(
		ID3D12GraphicsCommandList* commandList,
		TextureManager* textureManager,
		const ModelDrawParameters& parameters
	) {

		commandList->SetGraphicsRootConstantBufferView(
			1,
			transformationMatrix_.Write(transformationMatrixData_)
		);

		auto drawParameters = parameters;
		if (!drawParameters.textureOverride && textureOverride_) {
			drawParameters.textureOverride = textureOverride_;
		}
		model_->Draw(commandList, textureManager, drawParameters);
	}
}
