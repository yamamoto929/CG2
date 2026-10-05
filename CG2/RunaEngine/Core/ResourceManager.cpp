#include "ResourceManager.h"
#include "TextureManager.h"
#include <cassert>
#include <filesystem>
#include <algorithm>
#include "EngineError.h"

RunaEngine::Model* ResourceManager::LoadModel(
	ID3D12Device* device,
	TextureManager* textureManager,
	const std::string& directoryPath,
	const std::string& fileName
) {
	Require(device != nullptr, "device is not initialized");
	Require(textureManager != nullptr, "textureManager is not initialized");

	const auto path = std::filesystem::weakly_canonical(std::filesystem::path(directoryPath) / fileName);
	const auto key = path.generic_string();
	if (const auto found = modelCache_.find(key); found != modelCache_.end()) { return found->second; }
	Require(path.extension() == ".obj" || path.extension() == ".OBJ", "Only OBJ models are supported: " + key);
	RunaEngine::ModelData modelData = modelLoader_.LoadObjFile(path.parent_path().generic_string(), path.filename().generic_string());
	std::unique_ptr<RunaEngine::Model> model = std::make_unique<RunaEngine::Model>();
	model->Initialize(device, textureManager, &modelData);

	RunaEngine::Model* result = model.get();
	models_.push_back(std::move(model));
	modelCache_[key] = result;
	return result;
}

void ResourceManager::BeginFrame() { for (auto& model : models_) { model->BeginFrame(); } }
bool ResourceManager::Contains(const RunaEngine::Model* model) const {
    return std::any_of(models_.begin(), models_.end(), [model](const auto& item) { return item.get() == model; });
}
bool ResourceManager::UsesTexture(uint32_t handle) const {
    return std::any_of(models_.begin(), models_.end(), [handle](const auto& item) { return item->UsesTexture(handle); });
}
void ResourceManager::DestroyModel(RunaEngine::Model* model) {
    Require(Contains(model), "Model is not owned by this resource manager");
    std::erase_if(modelCache_, [model](const auto& item) { return item.second == model; });
    std::erase_if(models_, [model](const auto& item) { return item.get() == model; });
}
void ResourceManager::Clear() { modelCache_.clear(); models_.clear(); }
