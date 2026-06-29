#pragma once
#include <vector>
#include "VertexData.h"
#include "MaterialData.h"
namespace RunaEngine {
	struct ModelData {
		std::vector<VertexData> vertices;
		MaterialData material;
	};
}
