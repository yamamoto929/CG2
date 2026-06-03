#pragma once
#include "Matrix4x4.h"
#include "Transform.h"
#include <d3d12.h>
#pragma comment(lib,"d3d12.lib")
#include <cstddef>
#include <wrl.h>
#include "TransformationMatrix.h"

class Model;
class TextureManager;

class Object3D {
private:
	Transform transform_{
		{ 1.0f, 1.0f, 1.0f },
		{ 0.0f, 0.0f, 0.0f },
		{ 0.0f, 0.0f, 0.0f },
	};
	Microsoft::WRL::ComPtr<ID3D12Resource> transformationMatrixResource_;
	TransformationMatrix* transformationMatrixData_ = nullptr;
	Model* model_ = nullptr;
	Matrix4x4 worldMatrix_{};
	Matrix4x4 wvpMatrix_{};

	Microsoft::WRL::ComPtr<ID3D12Resource> CreateBufferResource(ID3D12Device* device, size_t sizeInBytes);
public:
	void Initialize(ID3D12Device* device, Model* model);

	void SetModel(Model* model) { model_ = model; }
	Model* GetModel() const { return model_; }

	Transform& GetTransform() { return transform_; }
	const Transform& GetTransform() const { return transform_; }

	const Matrix4x4& GetWorldMatrix() const { return worldMatrix_; }
	const Matrix4x4& GetWVPMatrix() const { return wvpMatrix_; }

	void Update(const Matrix4x4& viewMatrix, const Matrix4x4& projectionMatrix);
	void Draw(ID3D12GraphicsCommandList* commandList, TextureManager* textureManager);


};
