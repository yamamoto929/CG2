#include "Primitive3D.h"
#include "AffineMatrix.h"
#include <cassert>
#include <cstring>

namespace RunaEngine{
	void Primitive3D::InitializeTriangle(ID3D12Device* device) {

		VertexData vertices[3] = {
			{
				{ -1.0f, -1.0f, 0.0f, 1.0f },
				{ 0.0f, 0.0f },
				{ 0.0f, 0.0f, -1.0f },
			},
			{
				{ 1.0f, -1.0f, 0.0f, 1.0f },
				{ 0.0f, 0.0f },
				{ 0.0f, 0.0f, -1.0f },
			},
			{
				{ 0.0f, 1.0f, 0.0f, 1.0f },
				{ 0.0f, 0.0f },
				{ 0.0f, 0.0f, -1.0f },
			},
		};

		vertexResource_ = CreateBufferResource(device, sizeof(vertices));
		vertexBufferView_.BufferLocation = vertexResource_->GetGPUVirtualAddress();
		vertexBufferView_.SizeInBytes = sizeof(vertices);
		vertexBufferView_.StrideInBytes = sizeof(VertexData);

		VertexData* vertexData = nullptr;
		HRESULT hr = vertexResource_->Map(0, nullptr, reinterpret_cast<void**>(&vertexData));
		assert(SUCCEEDED(hr));
		std::memcpy(vertexData, vertices, sizeof(vertices));
		vertexResource_->Unmap(0, nullptr);
		vertexCount_ = 3;

		material_.Initialize(device);
		materialData_.color = { 1.0f, 0.2f, 0.1f, 1.0f };
		materialData_.lightingMode = LightingMode::NONE;
		materialData_.uvTransform = MakeIdentityMatrix();

		worldMatrix_ = MakeIdentityMatrix();
		wvpMatrix_ = MakeIdentityMatrix();
		transformationMatrix_.Initialize(device);
		transformationMatrixData_.World = worldMatrix_;
		transformationMatrixData_.WVP = wvpMatrix_;
		transformationMatrixData_.WorldInverseTranspose = MakeNormalMatrix(worldMatrix_);
	}

	void Primitive3D::Update(const Matrix4x4& viewMatrix, const Matrix4x4& projectionMatrix) {

		worldMatrix_ = MakeAffineMatrix(transform_.scale, transform_.rotate, transform_.translate);
		wvpMatrix_ = Multiply(worldMatrix_, Multiply(viewMatrix, projectionMatrix));

		transformationMatrixData_.World = worldMatrix_;
		transformationMatrixData_.WVP = wvpMatrix_;
		transformationMatrixData_.WorldInverseTranspose = MakeNormalMatrix(worldMatrix_);
	}

	void Primitive3D::Draw(ID3D12GraphicsCommandList* commandList) {

		commandList->IASetVertexBuffers(0, 1, &vertexBufferView_);
		commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

		commandList->SetGraphicsRootConstantBufferView(0, material_.Write(materialData_));
		commandList->SetGraphicsRootConstantBufferView(1, transformationMatrix_.Write(transformationMatrixData_));

		commandList->DrawInstanced(vertexCount_, 1, 0, 0);
	}

	void Primitive3D::SetColor(const Vector4& color) {
		materialData_.color = color;
	}

	const Vector4& Primitive3D::GetColor() const {
		return materialData_.color;
	}

	void Primitive3D::SetLightingMode(LightingMode lightingMode) {
		materialData_.lightingMode = lightingMode;
	}

	Microsoft::WRL::ComPtr<ID3D12Resource> Primitive3D::CreateBufferResource(ID3D12Device* device, size_t sizeInBytes) {
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
}
