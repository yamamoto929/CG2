#pragma once
#include <string>
#include "ModelData.h"
#include "VertexData.h"
#include "MaterialData.h"
class ModelLoader {
private:
	MaterialData LoadMaterialTemplateFile(const std::string& directoryPath, const std::string& filename);
public:	
	ModelData LoadObjFile(const std::string& directoryPath, const std::string& filename);
};

