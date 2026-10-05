#pragma once
#include <d3d12.h>
#pragma comment(lib,"d3d12.lib")
#include <wrl.h>
#include "ShaderCompiler.h"
class SpriteGraphicsPipeline
{
public:
    void Initialize(
        ID3D12Device* device,
        ShaderCompiler* shaderCompiler,
        DXGI_FORMAT rtvFormat,
        DXGI_FORMAT dsvFormat
    );

    void Set(ID3D12GraphicsCommandList* commandList);
    void Shutdown() { pipelineState_.Reset(); rootSignature_.Reset(); }

    ID3D12RootSignature* GetRootSignature() const;

private:
    void CreateRootSignature(ID3D12Device* device);
    void CreatePipelineState(
        ID3D12Device* device,
        ShaderCompiler* shaderCompiler,
        DXGI_FORMAT rtvFormat,
        DXGI_FORMAT dsvFormat
    );

private:
    Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature_;
    Microsoft::WRL::ComPtr<ID3D12PipelineState> pipelineState_;
};

