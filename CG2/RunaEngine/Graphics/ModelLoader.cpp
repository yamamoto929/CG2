#include "ModelLoader.h"
#include "Vector2.h"
#include "Vector3.h"
#include "Vector4.h"
#include <cassert>
#include <cstdint>
#include <fstream>
#include <sstream>
#include <utility>
#include <vector>

namespace {
	uint32_t ParseObjIndex(const std::string& index) {
		return index.empty() ? 0u : static_cast<uint32_t>(std::stoi(index));
	}
}

RunaEngine::ModelData ModelLoader::LoadObjFile(
	const std::string& directoryPath,
	const std::string& filename
) {
	RunaEngine::ModelData modelData;
	std::vector<RunaEngine::Vector4> positions;
	std::vector<RunaEngine::Vector3> normals;
	std::vector<RunaEngine::Vector2> texCoords;

	RunaEngine::MeshData currentMesh{};
	RunaEngine::SubMeshData currentSubMesh{};
	std::string currentMaterialName;
	bool hasCurrentMesh = false;
	bool hasCurrentSubMesh = false;

	auto FinishCurrentSubMesh = [&]() {
		if (!hasCurrentSubMesh) {
			return;
		}

		const uint32_t currentVertexCount =
			static_cast<uint32_t>(modelData.vertices.size());
		currentSubMesh.vertexCount =
			currentVertexCount - currentSubMesh.firstVertex;

		if (currentSubMesh.vertexCount > 0) {
			currentMesh.subMeshes.push_back(std::move(currentSubMesh));
		}

		currentSubMesh = {};
		hasCurrentSubMesh = false;
	};

	auto FinishCurrentMesh = [&]() {
		FinishCurrentSubMesh();
		if (hasCurrentMesh && !currentMesh.subMeshes.empty()) {
			modelData.meshes.push_back(std::move(currentMesh));
		}

		currentMesh = {};
		hasCurrentMesh = false;
	};

	auto StartMesh = [&](const std::string& meshName) {
		FinishCurrentMesh();
		currentMesh = {};
		currentMesh.name = meshName.empty() ? "default" : meshName;
		hasCurrentMesh = true;
	};

	auto EnsureCurrentMesh = [&]() {
		if (!hasCurrentMesh) {
			currentMesh = {};
			currentMesh.name = "default";
			hasCurrentMesh = true;
		}
	};

	auto StartSubMesh = [&](const std::string& materialName) {
		EnsureCurrentMesh();
		FinishCurrentSubMesh();
		currentSubMesh = {};
		currentSubMesh.materialName = materialName;
		currentSubMesh.firstVertex =
			static_cast<uint32_t>(modelData.vertices.size());
		hasCurrentSubMesh = true;
	};

	auto EnsureCurrentSubMesh = [&]() {
		EnsureCurrentMesh();
		if (!hasCurrentSubMesh) {
			StartSubMesh(currentMaterialName);
		}
	};

	std::ifstream file(directoryPath + "/" + filename);
	assert(file.is_open());

	std::string line;
	while (std::getline(file, line)) {
		std::string identifier;
		std::istringstream s(line);
		s >> identifier;

		if (identifier == "v") {
			RunaEngine::Vector4 position;
			s >> position.x >> position.y >> position.z;
			position.w = 1.0f;
			position.x *= -1.0f;
			positions.push_back(position);
		} else if (identifier == "vt") {
			RunaEngine::Vector2 texCoord;
			s >> texCoord.x >> texCoord.y;
			texCoord.y = 1.0f - texCoord.y;
			texCoords.push_back(texCoord);
		} else if (identifier == "vn") {
			RunaEngine::Vector3 normal;
			s >> normal.x >> normal.y >> normal.z;
			normal.x *= -1.0f;
			normals.push_back(normal);
		} else if (identifier == "o" || identifier == "g") {
			std::string meshName;
			s >> meshName;
			StartMesh(meshName);
		} else if (identifier == "usemtl") {
			s >> currentMaterialName;
			StartSubMesh(currentMaterialName);
		} else if (identifier == "f") {
			EnsureCurrentSubMesh();

			std::vector<RunaEngine::VertexData> faceVertices;
			std::string vertexDefinition;
			while (s >> vertexDefinition) {
				std::istringstream v(vertexDefinition);
				uint32_t elementIndices[3] = {};
				for (int32_t element = 0; element < 3; ++element) {
					std::string index;
					if (std::getline(v, index, '/')) {
						elementIndices[element] = ParseObjIndex(index);
					}
				}

				assert(elementIndices[0] > 0);
				assert(elementIndices[0] <= positions.size());
				RunaEngine::Vector4 position = positions[elementIndices[0] - 1];

				RunaEngine::Vector2 texCoord{ 0.0f, 0.0f };
				if (elementIndices[1] != 0) {
					assert(elementIndices[1] <= texCoords.size());
					texCoord = texCoords[elementIndices[1] - 1];
				}

				RunaEngine::Vector3 normal{ 0.0f, 0.0f, 0.0f };
				if (elementIndices[2] != 0) {
					assert(elementIndices[2] <= normals.size());
					normal = normals[elementIndices[2] - 1];
				}

				faceVertices.push_back({ position, texCoord, normal });
			}

			assert(faceVertices.size() >= 3);
			for (size_t vertexIndex = 1; vertexIndex + 1 < faceVertices.size(); ++vertexIndex) {
				modelData.vertices.push_back(faceVertices[vertexIndex + 1]);
				modelData.vertices.push_back(faceVertices[vertexIndex]);
				modelData.vertices.push_back(faceVertices[0]);
			}
		} else if (identifier == "mtllib") {
			std::string materialFilename;
			s >> materialFilename;
			auto loadedMaterials =
				LoadMaterialTemplateFile(directoryPath, materialFilename);
			for (auto& [materialName, material] : loadedMaterials) {
				modelData.materials.insert_or_assign(
					materialName,
					std::move(material)
				);
			}
		}
	}

	FinishCurrentMesh();
	return modelData;
}

std::unordered_map<std::string, RunaEngine::MaterialData>
ModelLoader::LoadMaterialTemplateFile(
	const std::string& directoryPath,
	const std::string& filename
) {
	std::unordered_map<std::string, RunaEngine::MaterialData> materials;
	RunaEngine::MaterialData* currentMaterial = nullptr;

	std::ifstream file(directoryPath + "/" + filename);
	assert(file.is_open());

	std::string line;
	while (std::getline(file, line)) {
		std::string identifier;
		std::istringstream s(line);
		s >> identifier;

		if (identifier == "newmtl") {
			std::string materialName;
			s >> materialName;
			currentMaterial = &materials[materialName];
		} else if (identifier == "map_Kd" && currentMaterial) {
			std::string textureFilename;
			s >> textureFilename;
			currentMaterial->textureFilePath =
				directoryPath + "/" + textureFilename;
		}
	}

	return materials;
}
