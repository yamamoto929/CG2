#pragma once
#include <d3d12.h>
#pragma comment(lib,"d3d12.lib")

class DirectXCommon;
class DirectionalLight;
class GraphicsPipeline;
class Object3D;
class Primitive3D;
class PrimitiveGraphicsPipeline;
class Sprite;
class TextureManager;

class Renderer {
public:
	void Initialize(
		DirectXCommon* directXCommon,
		TextureManager* textureManager,
		GraphicsPipeline* graphicsPipeline,
		PrimitiveGraphicsPipeline* primitiveGraphicsPipeline,
		DirectionalLight* directionalLight
	);

	void Begin();
	void Draw(Object3D& object);
	void Draw(Primitive3D& primitive);
	void Draw(Sprite& sprite);
	void End();

	ID3D12GraphicsCommandList* GetCommandList() const;

private:
	DirectXCommon* directXCommon_ = nullptr;
	TextureManager* textureManager_ = nullptr;
	GraphicsPipeline* graphicsPipeline_ = nullptr;
	PrimitiveGraphicsPipeline* primitiveGraphicsPipeline_ = nullptr;
	DirectionalLight* directionalLight_ = nullptr;
};
