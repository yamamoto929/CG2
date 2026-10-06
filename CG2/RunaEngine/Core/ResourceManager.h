#pragma once
#include "Model.h"
#include "ModelLoader.h"
#include <memory>
#include <string>
#include <vector>
#include <unordered_map>

class TextureManager;
struct ID3D12Device;

class ResourceManager {
public:
	RunaEngine::Model* LoadModel(
		ID3D12Device* device,
		TextureManager* textureManager,
		const std::string& directoryPath,
		const std::string& fileName
	);
	void BeginFrame();
	bool UsesTexture(uint32_t handle) const;
	void DestroyModel(RunaEngine::Model* model);
	void Clear();

private:
	ModelLoader modelLoader_;
	std::vector<std::unique_ptr<RunaEngine::Model>> models_;
	std::unordered_map<std::string, RunaEngine::Model*> modelCache_;
};
