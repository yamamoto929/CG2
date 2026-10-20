#include "GraphicsPipeline.h"
#include <cassert>
void GraphicsPipeline::Initialize(
    ID3D12Device* device,
    ShaderCompiler* shaderCompiler,
    DXGI_FORMAT rtvFormat,
    DXGI_FORMAT dsvFormat
) {
    CreateRootSignature(device);
    CreatePipelineState(device, shaderCompiler, rtvFormat, dsvFormat);
}

void GraphicsPipeline::Set(ID3D12GraphicsCommandList* commandList) {
    Set(commandList, RunaEngine::BlendMode::NONE);
}

void GraphicsPipeline::Set(ID3D12GraphicsCommandList* commandList, const RunaEngine::BlendMode& blendmode) {
    ID3D12PipelineState* pso = pipelineState_.Get();

    switch (blendmode) {
    case RunaEngine::BlendMode::NONE:
        break;
    case RunaEngine::BlendMode::NORMAL:
        pso = normalPipelineState_.Get();
        break;
    case RunaEngine::BlendMode::ADD:
        pso = addPipelineState_.Get();
        break;
    case RunaEngine::BlendMode::SUBTRACT:
        pso = subtractPipelineState_.Get();
        break;
    case RunaEngine::BlendMode::MULTIPLY:
        pso = multiplyPipelineState_.Get();
        break;
    case RunaEngine::BlendMode::SCREEN:
        pso = screenPipelineState_.Get();
        break;
    default:
        break;
    }
    assert(pso != nullptr);
    commandList->SetGraphicsRootSignature(rootSignature_.Get());
    commandList->SetPipelineState(pso);
}


ID3D12RootSignature* GraphicsPipeline::GetRootSignature() const {
    return rootSignature_.Get();
}

void GraphicsPipeline::CreateRootSignature(ID3D12Device* device) {

    HRESULT hr;

    D3D12_ROOT_SIGNATURE_DESC descriptionRootSignature{};
    descriptionRootSignature.Flags =
        D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

    D3D12_DESCRIPTOR_RANGE descriptorRange[1] = {};
    descriptorRange[0].BaseShaderRegister = 0;
    descriptorRange[0].NumDescriptors = 1;
    descriptorRange[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_SRV;
    descriptorRange[0].OffsetInDescriptorsFromTableStart =
        D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

    D3D12_ROOT_PARAMETER rootParameters[4] = {};

    rootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
    rootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
    rootParameters[0].Descriptor.ShaderRegister = 0;

    rootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
    rootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;
    rootParameters[1].Descriptor.ShaderRegister = 0;

    rootParameters[2].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    rootParameters[2].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
    rootParameters[2].DescriptorTable.pDescriptorRanges = descriptorRange;
    rootParameters[2].DescriptorTable.NumDescriptorRanges = _countof(descriptorRange);

    rootParameters[3].ParameterType = D3D12_ROOT_PARAMETER_TYPE_CBV;
    rootParameters[3].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;
    rootParameters[3].Descriptor.ShaderRegister = 1;

    descriptionRootSignature.pParameters = rootParameters;
    descriptionRootSignature.NumParameters = _countof(rootParameters);

    D3D12_STATIC_SAMPLER_DESC staticSamplers[1] = {};
    staticSamplers[0].Filter = D3D12_FILTER_MIN_MAG_MIP_LINEAR;
    staticSamplers[0].AddressU = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    staticSamplers[0].AddressV = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    staticSamplers[0].AddressW = D3D12_TEXTURE_ADDRESS_MODE_WRAP;
    staticSamplers[0].ComparisonFunc = D3D12_COMPARISON_FUNC_NEVER;
    staticSamplers[0].MaxLOD = D3D12_FLOAT32_MAX;
    staticSamplers[0].ShaderRegister = 0;
    staticSamplers[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_PIXEL;

    descriptionRootSignature.pStaticSamplers = staticSamplers;
    descriptionRootSignature.NumStaticSamplers = _countof(staticSamplers);

    Microsoft::WRL::ComPtr<ID3DBlob> signatureBlob = nullptr;
    Microsoft::WRL::ComPtr<ID3DBlob> errorBlob = nullptr;

    hr = D3D12SerializeRootSignature(
        &descriptionRootSignature,
        D3D_ROOT_SIGNATURE_VERSION_1,
        &signatureBlob,
        &errorBlob
    );

    assert(SUCCEEDED(hr));

    hr = device->CreateRootSignature(
        0,
        signatureBlob->GetBufferPointer(),
        signatureBlob->GetBufferSize(),
        IID_PPV_ARGS(&rootSignature_)
    );
    assert(SUCCEEDED(hr));
}

void GraphicsPipeline::CreatePipelineState(
    ID3D12Device* device,
    ShaderCompiler* shaderCompiler,
    DXGI_FORMAT rtvFormat,
    DXGI_FORMAT dsvFormat
) {

    HRESULT hr;

    D3D12_INPUT_ELEMENT_DESC inputElementDescs[3] = {};

    inputElementDescs[0].SemanticName = "POSITION";
    inputElementDescs[0].SemanticIndex = 0;
    inputElementDescs[0].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
    inputElementDescs[0].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;

    inputElementDescs[1].SemanticName = "TEXCOORD";
    inputElementDescs[1].SemanticIndex = 0;
    inputElementDescs[1].Format = DXGI_FORMAT_R32G32_FLOAT;
    inputElementDescs[1].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;

    inputElementDescs[2].SemanticName = "NORMAL";
    inputElementDescs[2].SemanticIndex = 0;
    inputElementDescs[2].Format = DXGI_FORMAT_R32G32B32_FLOAT;
    inputElementDescs[2].AlignedByteOffset = D3D12_APPEND_ALIGNED_ELEMENT;

    D3D12_INPUT_LAYOUT_DESC inputLayoutDesc{};
    inputLayoutDesc.pInputElementDescs = inputElementDescs;
    inputLayoutDesc.NumElements = _countof(inputElementDescs);

    Microsoft::WRL::ComPtr<IDxcBlob> vertexShaderBlob =
        shaderCompiler->Compile(L"RunaEngine/Graphics/Object3d.VS.hlsl", L"vs_6_0");

    Microsoft::WRL::ComPtr<IDxcBlob> pixelShaderBlob =
        shaderCompiler->Compile(L"RunaEngine/Graphics/Object3d.PS.hlsl", L"ps_6_0");

    D3D12_BLEND_DESC noneBlendDesc{};
    noneBlendDesc.RenderTarget[0].RenderTargetWriteMask =
        D3D12_COLOR_WRITE_ENABLE_ALL;
    noneBlendDesc.RenderTarget[0].BlendEnable = FALSE;
    noneBlendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_ONE;
    noneBlendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
    noneBlendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_ZERO;
    noneBlendDesc.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
    noneBlendDesc.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
    noneBlendDesc.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;

    D3D12_BLEND_DESC normalBlendDesc{};
    normalBlendDesc.RenderTarget[0].RenderTargetWriteMask =
        D3D12_COLOR_WRITE_ENABLE_ALL;
    normalBlendDesc.RenderTarget[0].BlendEnable = TRUE;
    normalBlendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
    normalBlendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
    normalBlendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_INV_SRC_ALPHA;
    normalBlendDesc.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
    normalBlendDesc.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
    normalBlendDesc.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;

    D3D12_BLEND_DESC addBlendDesc{};
    addBlendDesc.RenderTarget[0].RenderTargetWriteMask =
        D3D12_COLOR_WRITE_ENABLE_ALL;
    addBlendDesc.RenderTarget[0].BlendEnable = TRUE;
    addBlendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
    addBlendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
    addBlendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_ONE;
    addBlendDesc.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
    addBlendDesc.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
    addBlendDesc.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;

    D3D12_BLEND_DESC subtractBlendDesc{};
    subtractBlendDesc.RenderTarget[0].RenderTargetWriteMask =
        D3D12_COLOR_WRITE_ENABLE_ALL;
    subtractBlendDesc.RenderTarget[0].BlendEnable = TRUE;
    subtractBlendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_SRC_ALPHA;
    subtractBlendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_REV_SUBTRACT;
    subtractBlendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_ONE;
    subtractBlendDesc.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
    subtractBlendDesc.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
    subtractBlendDesc.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;

    D3D12_BLEND_DESC multiplyBlendDesc{};
    multiplyBlendDesc.RenderTarget[0].RenderTargetWriteMask =
        D3D12_COLOR_WRITE_ENABLE_ALL;
    multiplyBlendDesc.RenderTarget[0].BlendEnable = TRUE;
    multiplyBlendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_ZERO;
    multiplyBlendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
    multiplyBlendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_SRC_COLOR;
    multiplyBlendDesc.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
    multiplyBlendDesc.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
    multiplyBlendDesc.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;

    D3D12_BLEND_DESC screenBlendDesc{};
    screenBlendDesc.RenderTarget[0].RenderTargetWriteMask =
        D3D12_COLOR_WRITE_ENABLE_ALL;
    screenBlendDesc.RenderTarget[0].BlendEnable = TRUE;
    screenBlendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_INV_DEST_COLOR;
    screenBlendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
    screenBlendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_ONE;
    screenBlendDesc.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
    screenBlendDesc.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
    screenBlendDesc.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;

    D3D12_RASTERIZER_DESC rasterizerDesc{};
    rasterizerDesc.CullMode = D3D12_CULL_MODE_NONE;
    rasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;
    rasterizerDesc.FrontCounterClockwise = FALSE;

    D3D12_DEPTH_STENCIL_DESC depthStencilDesc{};
    depthStencilDesc.DepthEnable = true;
    depthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
    depthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;

    D3D12_GRAPHICS_PIPELINE_STATE_DESC graphicsPipelineStateDesc{};
    graphicsPipelineStateDesc.pRootSignature = rootSignature_.Get();
    graphicsPipelineStateDesc.InputLayout = inputLayoutDesc;
    graphicsPipelineStateDesc.VS = {
        vertexShaderBlob->GetBufferPointer(),
        vertexShaderBlob->GetBufferSize()
    };
    graphicsPipelineStateDesc.PS = {
        pixelShaderBlob->GetBufferPointer(),
        pixelShaderBlob->GetBufferSize()
    };

    graphicsPipelineStateDesc.RasterizerState = rasterizerDesc;
    graphicsPipelineStateDesc.NumRenderTargets = 1;
    graphicsPipelineStateDesc.RTVFormats[0] = rtvFormat;
    graphicsPipelineStateDesc.PrimitiveTopologyType =
        D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    graphicsPipelineStateDesc.SampleDesc.Count = 1;
    graphicsPipelineStateDesc.SampleMask = D3D12_DEFAULT_SAMPLE_MASK;
    graphicsPipelineStateDesc.DepthStencilState = depthStencilDesc;
    graphicsPipelineStateDesc.DSVFormat = dsvFormat;

    graphicsPipelineStateDesc.BlendState = noneBlendDesc;
    hr = device->CreateGraphicsPipelineState(
        &graphicsPipelineStateDesc,
        IID_PPV_ARGS(&pipelineState_)
    );
    assert(SUCCEEDED(hr));

    graphicsPipelineStateDesc.BlendState = normalBlendDesc;
    hr = device->CreateGraphicsPipelineState(
        &graphicsPipelineStateDesc,
        IID_PPV_ARGS(&normalPipelineState_)
    );
    assert(SUCCEEDED(hr));

    graphicsPipelineStateDesc.BlendState = addBlendDesc;
    hr = device->CreateGraphicsPipelineState(
        &graphicsPipelineStateDesc,
        IID_PPV_ARGS(&addPipelineState_)
    );
    assert(SUCCEEDED(hr));

    graphicsPipelineStateDesc.BlendState = subtractBlendDesc;
    hr = device->CreateGraphicsPipelineState(
        &graphicsPipelineStateDesc,
        IID_PPV_ARGS(&subtractPipelineState_)
    );
    assert(SUCCEEDED(hr));

    graphicsPipelineStateDesc.BlendState = multiplyBlendDesc;
    hr = device->CreateGraphicsPipelineState(
        &graphicsPipelineStateDesc,
        IID_PPV_ARGS(&multiplyPipelineState_)
    );
    assert(SUCCEEDED(hr));

    graphicsPipelineStateDesc.BlendState = screenBlendDesc;
    hr = device->CreateGraphicsPipelineState(
        &graphicsPipelineStateDesc,
        IID_PPV_ARGS(&screenPipelineState_)
    );
    assert(SUCCEEDED(hr));
}
