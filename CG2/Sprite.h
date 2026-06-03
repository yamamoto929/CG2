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
#include "Material.h"
#include "TransformationMatrix.h"

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
    void SetSize(const Vector2& size);
    void SetColor(const Vector4& color);
    void SetUVTransform(const Matrix4x4& uvTransform);
   
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
    Microsoft::WRL::ComPtr<ID3D12Resource> materialResource_;
    Material* materialData_;

    Vector4 color_ = { 1.0f, 1.0f, 1.0f, 1.0f };

    Transform uvTransform_{
        { 1.0f, 1.0f, 1.0f },
        { 0.0f, 0.0f, 0.0f },
        { 0.0f, 0.0f, 0.0f },
    };

    // WVP
    Microsoft::WRL::ComPtr<ID3D12Resource> transformationMatrixResource_;
    TransformationMatrix* transformationMatrixData_;

    Microsoft::WRL::ComPtr<ID3D12Resource> CreateBufferResource(const Microsoft::WRL::ComPtr<ID3D12Device>& device, size_t sizeInBytes);
};
