#pragma once
#include "ShaderCompiler.h"
#include <d3d12.h>
#pragma comment(lib, "d3d12.lib")
#include <wrl.h>

class PrimitiveGraphicsPipeline {
public:
	void Initialize(
		ID3D12Device* device,
		ShaderCompiler* shaderCompiler,
		DXGI_FORMAT rtvFormat,
		DXGI_FORMAT dsvFormat
	);

	void Set(ID3D12GraphicsCommandList* commandList);

private:
	void CreateRootSignature(ID3D12Device* device);
	void CreatePipelineState(
		ID3D12Device* device,
		ShaderCompiler* shaderCompiler,
		DXGI_FORMAT rtvFormat,
		DXGI_FORMAT dsvFormat
	);

	Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature_;
	Microsoft::WRL::ComPtr<ID3D12PipelineState> pipelineState_;
};
