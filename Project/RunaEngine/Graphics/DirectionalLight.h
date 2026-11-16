#pragma once
#include "ConstantBuffer.h"
#include "Vector3.h"
#include "Vector4.h"
#include <d3d12.h>
#pragma comment(lib,"d3d12.lib")

struct DirectionalLightData {
	RunaEngine::Vector4 color;
	RunaEngine::Vector3 direction;
	float intensity;
};

class DirectionalLight {
public:
	void Initialize(ID3D12Device* device);
	void BeginFrame() { buffer_.BeginFrame(); }
	void Shutdown() { buffer_.Clear(); }
	void SetCommand(ID3D12GraphicsCommandList* commandList, UINT rootParameterIndex = 3);

	void SetColor(const RunaEngine::Vector4& color);
	void SetDirection(const RunaEngine::Vector3& direction);
	void SetIntensity(float intensity);

	const RunaEngine::Vector4& GetColor() const { return data_.color; }
	const RunaEngine::Vector3& GetDirection() const { return data_.direction; }
	float GetIntensity() const { return data_.intensity; }

	DirectionalLightData* GetData() { return &data_; }
	const DirectionalLightData* GetData() const { return &data_; }

private:
	FrameBuffer<DirectionalLightData> buffer_;
	DirectionalLightData data_{};
};
