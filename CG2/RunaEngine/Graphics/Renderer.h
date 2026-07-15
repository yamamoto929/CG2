#pragma once
#include <d3d12.h>
#pragma comment(lib,"d3d12.lib")

class DirectXCommon;
class DirectionalLight;
class GraphicsPipeline;
namespace RunaEngine{
	class Object3D;
	class Primitive3D;
	class Sprite;
}

class PrimitiveGraphicsPipeline;
class TextureManager;
class SpriteGraphicsPipeline;

class Renderer {
public:
	void Initialize(
		DirectXCommon* directXCommon,
		TextureManager* textureManager,
		GraphicsPipeline* graphicsPipeline,
		PrimitiveGraphicsPipeline* primitiveGraphicsPipeline,
		DirectionalLight* directionalLight,
		SpriteGraphicsPipeline* spriteGraphicsPipeline
	);

	void Begin();
	void Draw(RunaEngine::Object3D& object);
	void Draw(RunaEngine::Primitive3D& primitive);
	void Draw(RunaEngine::Sprite& sprite);
	void End();

	ID3D12GraphicsCommandList* GetCommandList() const;

private:
	DirectXCommon* directXCommon_ = nullptr;
	TextureManager* textureManager_ = nullptr;
	GraphicsPipeline* graphicsPipeline_ = nullptr;
	PrimitiveGraphicsPipeline* primitiveGraphicsPipeline_ = nullptr;
	DirectionalLight* directionalLight_ = nullptr;
	SpriteGraphicsPipeline* spriteGraphicsPipeline_ = nullptr;
};
