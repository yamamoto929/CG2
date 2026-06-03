#include "ResourceManager.h"
#include "TextureManager.h"
#include <cassert>

Model* ResourceManager::LoadModel(
	ID3D12Device* device,
	TextureManager* textureManager,
	const std::string& directoryPath,
	const std::string& fileName
) {
	assert(device);
	assert(textureManager);

	ModelData modelData = modelLoader_.LoadObjFile(directoryPath, fileName);
	std::unique_ptr<Model> model = std::make_unique<Model>();
	model->Initialize(device, textureManager, &modelData);

	Model* result = model.get();
	models_.push_back(std::move(model));
	return result;
}
