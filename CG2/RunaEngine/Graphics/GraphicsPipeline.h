#pragma once
#include <d3d12.h>
#pragma comment(lib,"d3d12.lib")
#include <wrl.h>
#include "ShaderCompiler.h"
#include "BlendMode.h"

class GraphicsPipeline {
public:
    void Initialize(
        ID3D12Device* device,
        ShaderCompiler* shaderCompiler,
        DXGI_FORMAT rtvFormat,
        DXGI_FORMAT dsvFormat
    );

    void Set(ID3D12GraphicsCommandList* commandList);
    void Set(ID3D12GraphicsCommandList* commandList, const RunaEngine::BlendMode& blendmode);
    void Shutdown() {
        pipelineState_.Reset();
        normalPipelineState_.Reset();
        addPipelineState_.Reset();
        subtractPipelineState_.Reset();
        multiplyPipelineState_.Reset();
        screenPipelineState_.Reset();
        rootSignature_.Reset();
    }

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
    Microsoft::WRL::ComPtr<ID3D12PipelineState> normalPipelineState_;
    Microsoft::WRL::ComPtr<ID3D12PipelineState> addPipelineState_;
    Microsoft::WRL::ComPtr<ID3D12PipelineState> subtractPipelineState_;
    Microsoft::WRL::ComPtr<ID3D12PipelineState> multiplyPipelineState_;
    Microsoft::WRL::ComPtr<ID3D12PipelineState> screenPipelineState_;
};

