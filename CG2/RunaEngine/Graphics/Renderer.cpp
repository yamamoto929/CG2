#include "Renderer.h"
#include "DirectXCommon.h"
#include "DirectionalLight.h"
#include "GraphicsPipeline.h"
#include "Object3D.h"
#include "Primitive3D.h"
#include "PrimitiveGraphicsPipeline.h"
#include "Sprite.h"
#include "SpriteGraphicsPipeline.h"
#include "TextureManager.h"
#include <cassert>

void Renderer::Initialize(
	DirectXCommon* directXCommon,
	TextureManager* textureManager,
	GraphicsPipeline* graphicsPipeline,
	PrimitiveGraphicsPipeline* primitiveGraphicsPipeline,
	DirectionalLight* directionalLight,
	SpriteGraphicsPipeline* spriteGraphicsPipeline
) {
	assert(directXCommon);
	assert(textureManager);
	assert(graphicsPipeline);
	assert(primitiveGraphicsPipeline);
	assert(directionalLight);
	assert(spriteGraphicsPipeline);

	directXCommon_ = directXCommon;
	textureManager_ = textureManager;
	graphicsPipeline_ = graphicsPipeline;
	primitiveGraphicsPipeline_ = primitiveGraphicsPipeline;
	directionalLight_ = directionalLight;
	spriteGraphicsPipeline_ = spriteGraphicsPipeline;
}

void Renderer::Begin() {
	assert(directXCommon_);
	assert(textureManager_);
	assert(graphicsPipeline_);
	assert(primitiveGraphicsPipeline_);
	assert(directionalLight_);
	assert(spriteGraphicsPipeline_);

	directXCommon_->PreDraw();

	ID3D12DescriptorHeap* descriptorHeaps[] = {
		textureManager_->GetSrvDescriptorHeap()
	};
	GetCommandList()->SetDescriptorHeaps(1, descriptorHeaps);

	graphicsPipeline_->Set(GetCommandList());
	directionalLight_->SetCommand(GetCommandList());
}

void Renderer::Draw(RunaEngine::Object3D& object, const RunaEngine::ModelDrawParameters& parameters) {
	graphicsPipeline_->Set(GetCommandList());
	directionalLight_->SetCommand(GetCommandList(), 3);
	object.Draw(GetCommandList(), textureManager_, parameters);
}

void Renderer::Draw(RunaEngine::Primitive3D& primitive) {
	primitiveGraphicsPipeline_->Set(GetCommandList());
	directionalLight_->SetCommand(GetCommandList(), 2);
	primitive.Draw(GetCommandList());
}

void Renderer::Draw(RunaEngine::Sprite& sprite) {
	spriteGraphicsPipeline_->Set(GetCommandList());
	sprite.Draw(GetCommandList(), textureManager_);
}

void Renderer::End() {
	assert(directXCommon_);
	directXCommon_->PostDraw();
}

ID3D12GraphicsCommandList* Renderer::GetCommandList() const {
	assert(directXCommon_);
	return directXCommon_->GetCommandList();
}
