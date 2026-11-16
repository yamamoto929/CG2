#pragma once
#include <string>
#include <unordered_map>
#include <vector>
#include "VertexData.h"
#include "MaterialData.h"
#include "MeshData.h"
namespace RunaEngine {
	struct ModelData {
		std::vector<VertexData> vertices;
		std::vector<MeshData> meshes;
		std::unordered_map<std::string, MaterialData> materials;
	};
}
