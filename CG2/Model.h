#pragma once
#include <d3d12.h>
#pragma comment(lib,"d3d12.lib")
#include <string>
#include "TextureManager.h"
#include "ModelData.h"
#include "Material.h"
#include "TransformationMatrix.h"
#include "Matrix4x4.h"
class Model {
private:
	ModelData* modelData_;
	Microsoft::WRL::ComPtr<ID3D12Resource> vertexResource_ = nullptr;
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView_{};
	Material* materialData_ = nullptr;
	Microsoft::WRL::ComPtr<ID3D12Resource> materialResource_ = nullptr;
	Microsoft::WRL::ComPtr<ID3D12Resource> transformationMatrixResource_ = nullptr;
	TransformationMatrix* transformationMatrixData_ = nullptr;
	uint32_t textureHandle_;
	uint32_t vertexCount_;
	Microsoft::WRL::ComPtr<ID3D12Resource> CreateBufferResource(ID3D12Device* device, size_t sizeInBytes);
public:
	void Initialize(ID3D12Device* device, TextureManager* textureManager, ModelData* modelData);
	void Draw(ID3D12GraphicsCommandList* commandList, TextureManager* textureManager);
	const Material& GetMaterial() const { return *materialData_; }
	void SetColor(const Vector4& color) { materialData_->color = color; }
	void SetEnableLighting(bool enableLighting) { materialData_->enableLighting = enableLighting; }
	void SetUVTransform(const Matrix4x4& uvTransform) { materialData_->uvTransform = uvTransform; }
	void SetTransformationMatrix(const Matrix4x4& world, const Matrix4x4& wvp);
};

