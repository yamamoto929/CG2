#include "DirectionalLight.h"
#include <cassert>
#include <cmath>

void DirectionalLight::Initialize(ID3D12Device* device) {
	buffer_.Initialize(device);
	data_.color = { 1.0f, 1.0f, 1.0f, 1.0f };
	data_.direction = { 0.0f, -1.0f, 0.0f };
	data_.intensity = 1.0f;
}

void DirectionalLight::SetCommand(ID3D12GraphicsCommandList* commandList, UINT rootParameterIndex) {
	Require(commandList != nullptr, "DirectionalLight requires a command list");
	SetDirection(data_.direction);
	SetIntensity(data_.intensity);
	commandList->SetGraphicsRootConstantBufferView(rootParameterIndex, buffer_.Write(data_));
}

void DirectionalLight::SetColor(const RunaEngine::Vector4& color) {
	data_.color = color;
}

void DirectionalLight::SetDirection(const RunaEngine::Vector3& direction) {
	const double length = std::sqrt(double(direction.x) * direction.x + double(direction.y) * direction.y + double(direction.z) * direction.z);
	Require(std::isfinite(length) && length > 0.0, "Light direction must be finite and nonzero");
	data_.direction = {float(direction.x / length), float(direction.y / length), float(direction.z / length)};
}

void DirectionalLight::SetIntensity(float intensity) {
	Require(std::isfinite(intensity) && intensity >= 0.0f, "Light intensity must be finite and nonnegative");
	data_.intensity = intensity;
}
