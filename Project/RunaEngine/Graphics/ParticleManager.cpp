#include "ParticleManager.h"
#include "RunaEngine/RunaEngine.h"
#include "RunaEngine/Core/DirectXCommon.h"
#include "RunaEngine/Core/TextureManager.h"
#include "RunaEngine/Graphics/Camera.h"
#include "RunaEngine/Graphics/GraphicsPipeline.h"
#include "RunaEngine/Graphics/VertexData.h"
#include "RunaEngine/Math/AABB.h"
#include "RunaEngine/Math/AffineMatrix.h"
#include "RunaEngine/Math/RandomNumberGenerator.h"
#include "RunaEngine/externals/DirectXTex/d3dx12.h"

#include <cassert>
#include <cstring>

using namespace RunaEngine;

namespace {
    constexpr UINT kVertexCount = 6;

    Microsoft::WRL::ComPtr<ID3D12Resource> CreateUploadBuffer(
        ID3D12Device* device, UINT64 byteSize) {
        const CD3DX12_HEAP_PROPERTIES heap(D3D12_HEAP_TYPE_UPLOAD);
        const auto desc = CD3DX12_RESOURCE_DESC::Buffer(byteSize);

        Microsoft::WRL::ComPtr<ID3D12Resource> resource;
        const HRESULT hr = device->CreateCommittedResource(
            &heap, D3D12_HEAP_FLAG_NONE, &desc,
            D3D12_RESOURCE_STATE_GENERIC_READ, nullptr,
            IID_PPV_ARGS(&resource));
        assert(SUCCEEDED(hr));
        return resource;
    }
}

void ParticleManager::Initialize(DirectXCommon* directXCommon,
    TextureManager* textureManager,
    GraphicsPipeline* graphicsPipeline,
    ShaderCompiler* shaderCompiler) {
    directXCommon_ = directXCommon;
    textureManager_ = textureManager;
    graphicsPipeline_ = graphicsPipeline;

    // 初回取得で乱数エンジンが初期化される。
    (void)RNG::GetInstance();

    auto* device = directXCommon_->GetDevice();
    graphicsPipeline_->InitializeParticle(
        device, shaderCompiler, directXCommon_->GetDXGIFormat(),
        DXGI_FORMAT_D24_UNORM_S8_UINT);

    constexpr Vector3 normal{ 0.0f, 0.0f, 1.0f };
    constexpr VertexData bottomLeft{ {-1, -1, 0, 1}, {0, 1}, normal };
    constexpr VertexData topLeft{ {-1,  1, 0, 1}, {0, 0}, normal };
    constexpr VertexData bottomRight{ { 1, -1, 0, 1}, {1, 1}, normal };
    constexpr VertexData topRight{ { 1,  1, 0, 1}, {1, 0}, normal };
    constexpr VertexData vertices[kVertexCount]{
        bottomLeft, topLeft, bottomRight,
        topLeft, topRight, bottomRight
    };

    vertexResource_ = CreateUploadBuffer(device, sizeof(vertices));
    if (!vertexResource_) {
        return;
    }

    vertexBufferView_.BufferLocation = vertexResource_->GetGPUVirtualAddress();
    vertexBufferView_.SizeInBytes = static_cast<UINT>(sizeof(vertices));
    vertexBufferView_.StrideInBytes = static_cast<UINT>(sizeof(VertexData));

    const D3D12_RANGE readRange{ 0, 0 };
    void* mappedVertices = nullptr;
    const HRESULT hr = vertexResource_->Map(0, &readRange, &mappedVertices);
    assert(SUCCEEDED(hr));
    if (FAILED(hr)) {
        vertexResource_.Reset();
        return;
    }

    std::memcpy(mappedVertices, vertices, sizeof(vertices));
    vertexResource_->Unmap(0, nullptr);

    materialBuffer_.Initialize(device);
    auto* material = materialBuffer_.GetData();
    material->color = { 0.8f, 0.8f, 0.8f, 1.0f };
    material->lightingMode = LightingMode::NONE;
    material->uvTransform = MakeIdentityMatrix();
}

void ParticleManager::CreateParticleGroup(const std::string& name,
    const std::string& texturePath) {
    assert(vertexResource_.Get() != nullptr);
    if (!vertexResource_) {
        return;
    }

    const auto [it, inserted] = groups_.try_emplace(name);
    assert(inserted);
    if (!inserted) {
        return;
    }

    auto& group = it->second;
    group.settings.size = 0.14f;
    group.settings.lifetime = 1.0f;
    group.settings.color = { 1.0f, 1.0f, 1.0f };
    group.settings.useBillboard = true;

    group.material.textureFilePath = texturePath;
    group.textureHandle = textureManager_->Load(group.material.textureFilePath);
    group.material.textureSrvIndex = textureManager_->GetTextureSrvIndex(group.textureHandle);
    assert(group.material.textureSrvIndex != 0);
    if (group.material.textureSrvIndex == 0) {
        groups_.erase(it);
        return;
    }

    static_assert(sizeof(DataForGPU) == 144);
    auto* device = directXCommon_->GetDevice();
    group.instancingResource = CreateUploadBuffer(
        device, sizeof(DataForGPU) * kMaxParticleInstances);
    if (!group.instancingResource) {
        groups_.erase(it);
        return;
    }

    const D3D12_RANGE readRange{ 0, 0 };
    void* mappedInstances = nullptr;
    const HRESULT hr = group.instancingResource->Map(0, &readRange, &mappedInstances);
    assert(SUCCEEDED(hr));
    if (FAILED(hr)) {
        groups_.erase(it);
        return;
    }
    group.instancingData = static_cast<DataForGPU*>(mappedInstances);

    group.instancingSrvIndex = textureManager_->AllocateInstancingSrvIndex();
    assert(group.instancingSrvIndex != 0);
    if (group.instancingSrvIndex == 0) {
        groups_.erase(it);
        return;
    }

    D3D12_SHADER_RESOURCE_VIEW_DESC srv{};
    srv.Format = DXGI_FORMAT_UNKNOWN;
    srv.ViewDimension = D3D12_SRV_DIMENSION_BUFFER;
    srv.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srv.Buffer.NumElements = kMaxParticleInstances;
    srv.Buffer.StructureByteStride = static_cast<UINT>(sizeof(DataForGPU));

    device->CreateShaderResourceView(
        group.instancingResource.Get(), &srv,
        textureManager_->GetSrvHandleCPU(group.instancingSrvIndex));
}

ParticleManager::Group* ParticleManager::FindGroup(const std::string& name) {
    const auto it = groups_.find(name);
    assert(it != groups_.end());
    if (it == groups_.end()) {
        return nullptr;
    }
    return &it->second;
}
void ParticleManager::SetGroupSettings(const std::string& name,
    const Settings& settings) {
    if (auto* group = FindGroup(name)) {
        group->settings = settings;
    }
}

void ParticleManager::ClearParticles(const std::string& name) {
    if (auto* group = FindGroup(name)) {
        group->particles.clear();
        group->instanceCount = 0;
    }
}

void ParticleManager::Emit(const std::string& name,
    const Vector3& position, uint32_t count) {
    auto* group = FindGroup(name);
    if (!group) {
        return;
    }

    const auto& settings = group->settings;
    auto& rng = RNG::GetInstance();

    for (uint32_t i = 0; i < count; ++i) {
        Particle particle{};
        particle.transform.scale = { settings.size, settings.size, settings.size };
        particle.transform.translate = position + Vector3{
            rng.Generate(-0.25f, 0.25f),
            rng.Generate(-0.10f, 0.10f),
            rng.Generate(-0.25f, 0.25f)
        };
        particle.velocity = {
            rng.Generate(-0.65f, 0.65f),
            rng.Generate(-0.35f, 0.35f) + settings.upwardSpeed,
            rng.Generate(-0.65f, 0.65f)
        };
        particle.color = { settings.color.x, settings.color.y, settings.color.z, 1.0f };
        particle.lifeTime = settings.lifetime;

        group->particles.push_back(particle);
    }
}

void ParticleManager::Update(float dt, const Camera& camera) {
    const auto billboard = MakeAffineMatrix({ 1, 1, 1 }, camera.GetRotate(), {});
    const auto viewProjection = Multiply(camera.GetViewMatrix(), camera.GetProjectionMatrix());
    const AABB windArea{ {-1, -1, -1}, {1, 1, 1} };

    for (auto& entry : groups_) {
        auto& group = entry.second;
        group.instanceCount = 0;

        for (auto it = group.particles.begin(); it != group.particles.end();) {
            auto& particle = *it;
            if (particle.currentTime >= particle.lifeTime) {
                it = group.particles.erase(it);
                continue;
            }

            auto& transform = particle.transform;
            if (group.settings.useWind && windArea.Contains(transform.translate)) {
                particle.velocity.x += group.settings.windStrength * dt;
            }
            transform.translate += particle.velocity * dt;
            particle.currentTime += dt;

            // このフレームの時間加算で寿命に達した粒も描画前に外す。
            if (particle.currentTime >= particle.lifeTime) {
                it = group.particles.erase(it);
                continue;
            }
            ++it;

            if (group.instanceCount >= kMaxParticleInstances) {
                continue;
            }

            auto& gpu = group.instancingData[group.instanceCount];
            if (group.settings.useBillboard) {
                const auto scaleAndRotation =
                    Multiply(MakeScaleMatrix(transform.scale), billboard);
                gpu.World = Multiply(scaleAndRotation, MakeTranslateMatrix(transform.translate));
            } else {
                gpu.World = MakeAffineMatrix(transform.scale, transform.rotate,
                    transform.translate);
            }
            gpu.WVP = Multiply(gpu.World, viewProjection);
            gpu.color = particle.color;
            gpu.color.w *= 1.0f - particle.currentTime / particle.lifeTime;
            ++group.instanceCount;
        }
    }
}

void ParticleManager::Draw() {
    if (groups_.empty()) {
        return;
    }

    auto* commandList = directXCommon_->GetCommandList();
    graphicsPipeline_->SetParticle(commandList);
    commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    commandList->IASetVertexBuffers(0, 1, &vertexBufferView_);
    commandList->SetGraphicsRootConstantBufferView(0, materialBuffer_.GetGPUVirtualAddress());

    for (const auto& entry : groups_) {
        const auto& group = entry.second;
        if (group.instanceCount == 0) {
            continue;
        }

        commandList->SetGraphicsRootDescriptorTable(
            1, textureManager_->GetSrvHandleGPUByIndex(group.instancingSrvIndex));
        commandList->SetGraphicsRootDescriptorTable(
            2, textureManager_->GetSrvHandleGPUByIndex(group.material.textureSrvIndex));
        commandList->DrawInstanced(kVertexCount, group.instanceCount, 0, 0);
    }
}

bool ParticleManager::UsesTexture(uint32_t textureHandle) const {
    for (const auto& entry : groups_) {
        if (entry.second.textureHandle == textureHandle) {
            return true;
        }
    }
    return false;
}

void ParticleManager::ClearGroups() {
    for (const auto& entry : groups_) {
        textureManager_->ReleaseInstancingSrvIndex(entry.second.instancingSrvIndex);
    }
    groups_.clear();
}

void ParticleManager::Shutdown() {
    ClearGroups();
    materialBuffer_ = {};
    vertexBufferView_ = {};
    vertexResource_.Reset();
}