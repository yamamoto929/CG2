#pragma once
#include "ConstantBuffer.h"
#include "Material.h"
#include "Matrix4x4.h"
#include "Transform.h"
#include "TransformationMatrix.h"
#include "Vector4.h"
#include "VertexData.h"
#include <cstdint>
#include <d3d12.h>
#pragma comment(lib, "d3d12.lib")
#include <wrl.h>

class Primitive3D {
public:
	void InitializeTriangle(ID3D12Device* device);
	void Update(const Matrix4x4& viewMatrix, const Matrix4x4& projectionMatrix);
	void Draw(ID3D12GraphicsCommandList* commandList);

	Transform& GetTransform() { return transform_; }
	const Transform& GetTransform() const { return transform_; }

	void SetColor(const Vector4& color);
	const Vector4& GetColor() const;
	void SetEnableLighting(bool enableLighting);

private:
	Microsoft::WRL::ComPtr<ID3D12Resource> CreateBufferResource(ID3D12Device* device, size_t sizeInBytes);

	Transform transform_{
		{ 1.0f, 1.0f, 1.0f },
		{ 0.0f, 0.0f, 0.0f },
		{ 0.0f, 0.0f, 0.0f },
	};

	Microsoft::WRL::ComPtr<ID3D12Resource> vertexResource_;
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView_{};
	uint32_t vertexCount_ = 0;

	ConstantBuffer<Material> material_;
	Material* materialData_ = nullptr;

	ConstantBuffer<TransformationMatrix> transformationMatrix_;
	TransformationMatrix* transformationMatrixData_ = nullptr;

	Matrix4x4 worldMatrix_{};
	Matrix4x4 wvpMatrix_{};
};
