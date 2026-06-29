#include "ResourceManager.h"
#include "TextureManager.h"
#include <cassert>

RunaEngine::Model* ResourceManager::LoadModel(
	ID3D12Device* device,
	TextureManager* textureManager,
	const std::string& directoryPath,
	const std::string& fileName
) {
	assert(device);
	assert(textureManager);

	RunaEngine::ModelData modelData = modelLoader_.LoadObjFile(directoryPath, fileName);
	std::unique_ptr<RunaEngine::Model> model = std::make_unique<RunaEngine::Model>();
	model->Initialize(device, textureManager, &modelData);

	RunaEngine::Model* result = model.get();
	models_.push_back(std::move(model));
	return result;
}
