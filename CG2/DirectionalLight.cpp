#include "DirectionalLight.h"
#include <cassert>

void DirectionalLight::Initialize(ID3D12Device* device) {
	buffer_.Initialize(device);
	data_ = buffer_.GetData();

	data_->color = { 1.0f, 1.0f, 1.0f, 1.0f };
	data_->direction = { 0.0f, -1.0f, 0.0f };
	data_->intensity = 1.0f;
}

void DirectionalLight::SetCommand(ID3D12GraphicsCommandList* commandList, UINT rootParameterIndex) {
	assert(commandList);
	commandList->SetGraphicsRootConstantBufferView(rootParameterIndex, buffer_.GetGPUVirtualAddress());
}

void DirectionalLight::SetColor(const Vector4& color) {
	assert(data_);
	data_->color = color;
}

void DirectionalLight::SetDirection(const Vector3& direction) {
	assert(data_);
	data_->direction = direction;
	data_->direction.Normalize();
}

void DirectionalLight::SetIntensity(float intensity) {
	assert(data_);
	data_->intensity = intensity;
}
