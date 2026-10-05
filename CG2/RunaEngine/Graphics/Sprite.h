#pragma once
#include <d3d12.h>
#pragma comment(lib,"d3d12.lib")
#include <cstdint>
#include <string>
#include <wrl.h>
#include "VertexData.h"
#include "TextureManager.h"
#include "Vector2.h"
#include "Vector4.h"
#include "Matrix4x4.h"
#include "Transform.h"
#include "ConstantBuffer.h"
#include "SpriteMaterial.h"
#include "TransformationMatrix.h"
#include <array>

namespace RunaEngine {
	struct SpriteDrawCommand;
	class Sprite {
	public:
		void Initialize(
			ID3D12Device* device,
			TextureManager* textureManager,
			const std::string& texturePath
		);

		void Update(int32_t screenWidth, int32_t screenHeight);
		void Draw(ID3D12GraphicsCommandList* commandList, TextureManager* textureManager);
		void Draw(ID3D12GraphicsCommandList* commandList, TextureManager* textureManager,
			const SpriteDrawCommand& command, int32_t screenWidth, int32_t screenHeight);
		SpriteDrawCommand CaptureDrawCommand(const Transform& transform, uint64_t submissionIndex) const;
		void BeginFrame();

		Transform& GetTransform() { return transform_; }
		void SetTransform(const Transform& transform) { transform_ = transform; }
		const Transform& GetTransform() const;
		void SetSize(const float& sizeX, const float& sizeY);
		void SetSize(const RunaEngine::Vector2& size);
		void SetPivot(const float& pivotX, const float& pivotY);
		void SetPivot(const RunaEngine::Vector2& pivot);
		void SetColor(const RunaEngine::Vector4& color);
		void SetUVTransform(const RunaEngine::Matrix4x4& uvTransform);
		uint32_t GetTextureHandle() const { return textureHandle_; }
		void SetTextureHandle(uint32_t textureHandle);
		void SetDrawOrder(int32_t drawOrder);
		int32_t GetDrawOrder() const;

		void SetTextureRect(
			float x,
			float y,
			float width,
			float height
		);

		void SetUVRect(
			float left,
			float top,
			float right,
			float bottom
		);

		Vector4 GetColor() const { return color_; }
		Vector2 GetSize() const { return size_; }
		Vector2 GetUVLeftTop() const { return uvLeftTop_; }
		Vector2 GetUVRightBottom() const { return uvRightBottom_; }

	private:
		Vector2 textureSize_{};
		Vector2 uvLeftTop_{ 0.0f, 0.0f };
		Vector2 uvRightBottom_{ 1.0f, 1.0f };
		TextureManager* textureManager_ = nullptr;
		int32_t screenWidth_ = 0;
		int32_t screenHeight_ = 0;
		Transform transform_{};
		Vector2 size_{};
		Vector2 pivot_{};

		uint32_t textureHandle_ = 0u;

		// 描画順
		int32_t drawOrder_ = 0;

		// 頂点・インデックス
		FrameBuffer<std::array<VertexData, 4>> vertices_;

		Microsoft::WRL::ComPtr<ID3D12Resource> indexResource_;
		D3D12_INDEX_BUFFER_VIEW indexBufferView_;

		// マテリアル
		FrameBuffer<SpriteMaterial> material_;

		Vector4 color_ = { 1.0f, 1.0f, 1.0f, 1.0f };

		Matrix4x4 uvTransformMatrix_{};

		// WVP
		FrameBuffer<TransformationMatrix> transformationMatrix_;

		Microsoft::WRL::ComPtr<ID3D12Resource> CreateBufferResource(ID3D12Device* device, size_t sizeInBytes);

		static std::array<VertexData, 4> BuildVertices(const SpriteDrawCommand& command);
	};
}
