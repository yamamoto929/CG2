#pragma once
#include "Model.h"
#include "ModelLoader.h"
#include <memory>
#include <string>
#include <vector>

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

private:
	ModelLoader modelLoader_;
	std::vector<std::unique_ptr<RunaEngine::Model>> models_;
};
