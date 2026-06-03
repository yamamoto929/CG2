#pragma once
#include "ConstantBuffer.h"
#include "Vector3.h"
#include "Vector4.h"
#include <d3d12.h>
#pragma comment(lib,"d3d12.lib")

struct DirectionalLightData {
	Vector4 color;
	Vector3 direction;
	float intensity;
};

class DirectionalLight {
public:
	void Initialize(ID3D12Device* device);
	void SetCommand(ID3D12GraphicsCommandList* commandList, UINT rootParameterIndex = 3);

	void SetColor(const Vector4& color);
	void SetDirection(const Vector3& direction);
	void SetIntensity(float intensity);

	const Vector4& GetColor() const { return data_->color; }
	const Vector3& GetDirection() const { return data_->direction; }
	float GetIntensity() const { return data_->intensity; }

	DirectionalLightData* GetData() { return data_; }
	const DirectionalLightData* GetData() const { return data_; }

private:
	ConstantBuffer<DirectionalLightData> buffer_;
	DirectionalLightData* data_ = nullptr;
};
