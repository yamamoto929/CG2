#pragma once
#include <d3d12.h>
#pragma comment(lib,"d3d12.lib")
#include <cstdint>
#include <string>
#include <wrl.h>
#include "TextureManager.h"
#include "Vector2.h"
#include "Vector4.h"
#include "Matrix4x4.h"
#include "Transform.h"
#include "ConstantBuffer.h"
#include "Material.h"
#include "TransformationMatrix.h"

namespace RunaEngine{
    class Sprite {
    public:
        void Initialize(
            ID3D12Device* device,
            TextureManager* textureManager,
            const std::string& texturePath
        );

        void Update(int32_t screenWidth, int32_t screenHeight);
        void Draw(ID3D12GraphicsCommandList* commandList, TextureManager* textureManager);

        Transform& GetTransform();
        const Transform& GetTransform() const;
        void SetSize(const Vector2& size);
        void SetColor(const Vector4& color);
        void SetUVTransform(const Matrix4x4& uvTransform);
        uint32_t GetTextureHandle() const { return textureHandle_; }
        void SetTextureHandle(uint32_t textureHandle) { textureHandle_ = textureHandle; }

    private:
        Transform transform_;
        Vector2 size_;

        uint32_t textureHandle_;

        // 頂点・インデックス
        Microsoft::WRL::ComPtr<ID3D12Resource> vertexResource_;
        D3D12_VERTEX_BUFFER_VIEW vertexBufferView_;

        Microsoft::WRL::ComPtr<ID3D12Resource> indexResource_;
        D3D12_INDEX_BUFFER_VIEW indexBufferView_;

        // マテリアル
        ConstantBuffer<Material> material_;
        Material* materialData_;

        Vector4 color_ = { 1.0f, 1.0f, 1.0f, 1.0f };

        Transform uvTransform_{
            { 1.0f, 1.0f, 1.0f },
            { 0.0f, 0.0f, 0.0f },
            { 0.0f, 0.0f, 0.0f },
        };

        // WVP
        ConstantBuffer<TransformationMatrix> transformationMatrix_;
        TransformationMatrix* transformationMatrixData_;

        Microsoft::WRL::ComPtr<ID3D12Resource> CreateBufferResource(ID3D12Device* device, size_t sizeInBytes);
    };
}
