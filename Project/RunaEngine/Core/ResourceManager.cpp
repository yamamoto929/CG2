#include "ResourceManager.h"
#include "ConvertString.h"
#include "TextureManager.h"
#include <filesystem>
#include <algorithm>

RunaEngine::Model* ResourceManager::LoadModel(
	ID3D12Device* device,
	TextureManager* textureManager,
	const std::string& directoryPath,
	const std::string& fileName
) {

	const auto path = std::filesystem::weakly_canonical(
		std::filesystem::path(ConvertString(directoryPath)) / ConvertString(fileName));
	const auto key = ConvertString(path.generic_wstring());
	if (const auto found = modelCache_.find(key); found != modelCache_.end()) { return found->second; }
	if (path.extension() != ".obj" && path.extension() != ".OBJ") { return nullptr; }
	RunaEngine::ModelData modelData = modelLoader_.LoadObjFile(
		ConvertString(path.parent_path().generic_wstring()), ConvertString(path.filename().generic_wstring()));
	if (modelData.vertices.empty()) { return nullptr; }
	std::unique_ptr<RunaEngine::Model> model = std::make_unique<RunaEngine::Model>();
	model->Initialize(device, textureManager, &modelData);

	RunaEngine::Model* result = model.get();
	models_.push_back(std::move(model));
	modelCache_[key] = result;
	return result;
}

void ResourceManager::BeginFrame() { for (auto& model : models_) { model->BeginFrame(); } }
bool ResourceManager::UsesTexture(uint32_t handle) const {
    return std::any_of(models_.begin(), models_.end(), [handle](const auto& item) { return item->UsesTexture(handle); });
}
void ResourceManager::DestroyModel(RunaEngine::Model* model) {

    std::erase_if(modelCache_, [model](const auto& item) { return item.second == model; });
    std::erase_if(models_, [model](const auto& item) { return item.get() == model; });
}
void ResourceManager::Clear() { modelCache_.clear(); models_.clear(); }
