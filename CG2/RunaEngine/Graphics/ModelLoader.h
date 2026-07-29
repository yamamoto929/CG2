#pragma once
#include <string>
#include <unordered_map>
#include "ModelData.h"
#include "VertexData.h"
#include "MaterialData.h"
class ModelLoader {
private:
	std::unordered_map<std::string, RunaEngine::MaterialData> LoadMaterialTemplateFile(
		const std::string& directoryPath,
		const std::string& filename
	);
public:	
	RunaEngine::ModelData LoadObjFile(const std::string& directoryPath, const std::string& filename);
};

