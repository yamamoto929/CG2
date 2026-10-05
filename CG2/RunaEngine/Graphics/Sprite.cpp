#include "Sprite.h"
#include "SpriteDrawCommand.h"
#include "AffineMatrix.h"
#include "WVPMatrix.h"
#include "EngineError.h"

namespace RunaEngine {
    void Sprite::Initialize(ID3D12Device* device, TextureManager* textureManager,
        const std::string& texturePath) {
        Require(device && textureManager, "Sprite::Initialize: missing device or texture manager");
        textureManager_ = textureManager;
        vertices_.Initialize(device);
        material_.Initialize(device);
        transformationMatrix_.Initialize(device);
        SetTextureHandle(textureManager->Load(texturePath));
        size_ = textureSize_;
        transform_ = {{1, 1, 1}, {0, 0, 0}, {0, 0, 0}};
        uvTransformMatrix_ = MakeIdentityMatrix();
        indexResource_ = CreateBufferResource(device, sizeof(uint32_t) * 6);
        indexBufferView_ = {indexResource_->GetGPUVirtualAddress(), sizeof(uint32_t) * 6, DXGI_FORMAT_R32_UINT};
        uint32_t* indices = nullptr;
        CheckHR(indexResource_->Map(0, nullptr, reinterpret_cast<void**>(&indices)), "Sprite index buffer Map");
        const uint32_t values[] = {0, 1, 2, 1, 3, 2};
        for (size_t i = 0; i < 6; ++i) { indices[i] = values[i]; }
        indexResource_->Unmap(0, nullptr);
    }
    void Sprite::BeginFrame() {
        vertices_.BeginFrame();
        material_.BeginFrame();
        transformationMatrix_.BeginFrame();
    }
    void Sprite::Update(int32_t width, int32_t height) { screenWidth_ = width; screenHeight_ = height; }
    SpriteDrawCommand Sprite::CaptureDrawCommand(const Transform& transform, uint64_t index) const {
        SpriteDrawCommand command{};
        command.sprite = const_cast<Sprite*>(this);
        command.transform = transform;
        command.color = color_;
        command.size = size_;
        command.pivot = pivot_;
        command.uvLeftTop = uvLeftTop_;
        command.uvRightBottom = uvRightBottom_;
        command.uvTransform = uvTransformMatrix_;
        command.textureHandle = textureHandle_;
        command.drawOrder = drawOrder_;
        command.submissionIndex = index;
        return command;
    }
    void Sprite::Draw(ID3D12GraphicsCommandList* list, TextureManager* textures) {
        Draw(list, textures, CaptureDrawCommand(transform_, 0), screenWidth_, screenHeight_);
    }
    void Sprite::Draw(ID3D12GraphicsCommandList* list, TextureManager* textures,
        const SpriteDrawCommand& command, int32_t width, int32_t height) {
        Require(list && textures && width > 0 && height > 0,
            "Sprite::Draw: call Update with a positive screen size first");
        const auto vertexAddress = vertices_.Write(BuildVertices(command));
        const D3D12_VERTEX_BUFFER_VIEW vertexView{vertexAddress, sizeof(VertexData) * 4, sizeof(VertexData)};
        TransformationMatrix matrices{};
        matrices.World = MakeAffineMatrix(command.transform.scale, command.transform.rotate, command.transform.translate);
        matrices.WVP = Multiply(matrices.World, MakeOrthographicMatrix(0, 0, float(width), float(height), 0, 100));
        matrices.WorldInverseTranspose = MakeIdentityMatrix(); // Spriteは照明計算を行わない
        const SpriteMaterial material{command.color, command.uvTransform};
        list->IASetVertexBuffers(0, 1, &vertexView);
        list->IASetIndexBuffer(&indexBufferView_);
        list->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
        list->SetGraphicsRootConstantBufferView(0, material_.Write(material));
        list->SetGraphicsRootConstantBufferView(1, transformationMatrix_.Write(matrices));
        list->SetGraphicsRootDescriptorTable(2, textures->GetSrvHandleGPU(command.textureHandle));
        list->DrawIndexedInstanced(6, 1, 0, 0, 0);
    }
    const Transform& Sprite::GetTransform() const { return transform_; }
    void Sprite::SetSize(const float& x, const float& y) { SetSize(Vector2{x, y}); }
    void Sprite::SetSize(const Vector2& size) { size_ = size; }
    void Sprite::SetPivot(const float& x, const float& y) { SetPivot(Vector2{x, y}); }
    void Sprite::SetPivot(const Vector2& pivot) { pivot_ = pivot; }
    void Sprite::SetColor(const Vector4& color) { color_ = color; }
    void Sprite::SetUVTransform(const Matrix4x4& uvTransform) { uvTransformMatrix_ = uvTransform; }
    void Sprite::SetDrawOrder(int32_t order) { drawOrder_ = order; }
    int32_t Sprite::GetDrawOrder() const { return drawOrder_; }
    void Sprite::SetTextureHandle(uint32_t handle) {
        Require(textureManager_ != nullptr, "Sprite is not initialized");
        const auto size = textureManager_->GetTextureSize(handle);
        textureHandle_ = handle;
        textureSize_ = {float(size.width), float(size.height)};
    }
    void Sprite::SetTextureRect(float x, float y, float width, float height) {
        Require(textureSize_.x > 0 && textureSize_.y > 0, "Sprite texture size is invalid");
        SetUVRect(x / textureSize_.x, y / textureSize_.y,
            (x + width) / textureSize_.x, (y + height) / textureSize_.y);
    }
    void Sprite::SetUVRect(float left, float top, float right, float bottom) {
        uvLeftTop_ = {left, top}; uvRightBottom_ = {right, bottom};
    }
    std::array<VertexData, 4> Sprite::BuildVertices(const SpriteDrawCommand& command) {
        const float left = -command.pivot.x * command.size.x;
        const float right = left + command.size.x;
        const float top = -command.pivot.y * command.size.y;
        const float bottom = top + command.size.y;
        const auto uv0 = command.uvLeftTop;
        const auto uv1 = command.uvRightBottom;
        return {{
            {{left, bottom, 0, 1}, {uv0.x, uv1.y}, {0, 0, -1}},
            {{left, top, 0, 1}, {uv0.x, uv0.y}, {0, 0, -1}},
            {{right, bottom, 0, 1}, {uv1.x, uv1.y}, {0, 0, -1}},
            {{right, top, 0, 1}, {uv1.x, uv0.y}, {0, 0, -1}}
        }};
    }
    Microsoft::WRL::ComPtr<ID3D12Resource> Sprite::CreateBufferResource(ID3D12Device* device, size_t size) {
        D3D12_HEAP_PROPERTIES heap{}; heap.Type = D3D12_HEAP_TYPE_UPLOAD;
        D3D12_RESOURCE_DESC desc{};
        desc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER; desc.Width = size;
        desc.Height = 1; desc.DepthOrArraySize = 1; desc.MipLevels = 1;
        desc.SampleDesc.Count = 1; desc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
        Microsoft::WRL::ComPtr<ID3D12Resource> resource;
        CheckHR(device->CreateCommittedResource(&heap, D3D12_HEAP_FLAG_NONE, &desc,
            D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&resource)), "Sprite index buffer creation");
        return resource;
    }
}