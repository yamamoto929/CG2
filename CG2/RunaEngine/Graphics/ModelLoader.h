#pragma once
#include <string>
#include "ModelData.h"
#include "VertexData.h"
#include "MaterialData.h"
class ModelLoader {
private:
	RunaEngine::MaterialData LoadMaterialTemplateFile(const std::string& directoryPath, const std::string& filename);
public:	
	RunaEngine::ModelData LoadObjFile(const std::string& directoryPath, const std::string& filename);
};

