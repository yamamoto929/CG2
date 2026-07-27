#include "Object3D.h"
#include "AffineMatrix.h"
#include "Model.h"
#include <cassert>

namespace RunaEngine {
	void Object3D::Initialize(ID3D12Device* device, RunaEngine::Model* model) {
		assert(device);
		assert(model);

		model_ = model;
		worldMatrix_ = MakeIdentityMatrix();
		wvpMatrix_ = MakeIdentityMatrix();

		transformationMatrix_.Initialize(device);
		transformationMatrixData_ = transformationMatrix_.GetData();

		transformationMatrixData_->World = worldMatrix_;
		transformationMatrixData_->WVP = wvpMatrix_;

		material_.Initialize(device);
		materialData_ = material_.GetData();
	}

	void Object3D::Update(const Matrix4x4& viewMatrix, const Matrix4x4& projectionMatrix) {
		assert(model_);
		assert(transformationMatrixData_);

		worldMatrix_ = MakeAffineMatrix(transform_.scale, transform_.rotate, transform_.translate);
		wvpMatrix_ = Multiply(worldMatrix_, Multiply(viewMatrix, projectionMatrix));

		transformationMatrixData_->World = worldMatrix_;
		transformationMatrixData_->WVP = wvpMatrix_;
	}

	void Object3D::Draw(
		ID3D12GraphicsCommandList* commandList,
		TextureManager* textureManager,
		const ModelDrawParameters& parameters
	) {
		assert(model_);
		assert(transformationMatrixData_);
		assert(materialData_);

		materialData_->color = parameters.color;
		materialData_->lightingMode = parameters.lightingMode;
		materialData_->uvTransform = parameters.uvTransform;

		commandList->SetGraphicsRootConstantBufferView(
			0,
			material_.GetGPUVirtualAddress()
		);
		commandList->SetGraphicsRootConstantBufferView(
			1,
			transformationMatrix_.GetGPUVirtualAddress()
		);

		model_->Draw(commandList, textureManager);
	}
}
