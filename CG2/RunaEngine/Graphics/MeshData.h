#pragma once
#include <cstdint>
#include <string>
#include <vector>

namespace RunaEngine {
	struct SubMeshData {
		std::string materialName;
		uint32_t firstVertex = 0;
		uint32_t vertexCount = 0;
	};

	struct MeshData {
		std::string name;
		std::vector<SubMeshData> subMeshes;
	};
}
