#include "ModelLoader.h"
#include <cassert>
#include <fstream>
#include <sstream>
#include <cstdint>
#include "Vector2.h"
#include "Vector3.h"
#include "Vector4.h"
// =========================================================
// LoadObjFile 
// =========================================================
RunaEngine::ModelData ModelLoader::LoadObjFile(const std::string& directoryPath, const std::string& filename) {
	// 変数を宣言
	RunaEngine::ModelData modelData;
	std::vector<RunaEngine::Vector4> positions; // 位置
	std::vector<RunaEngine::Vector3> normals; // 法線
	std::vector<RunaEngine::Vector2> texCoords; // テクスチャ座標
	std::string line; // ファイルから読んだ1行を格納するもの

	// ファイルを開ける
	std::ifstream file(directoryPath + "/" + filename);
	assert(file.is_open());

	while (std::getline(file, line)) {
		std::string identifier;
		std::istringstream s(line);
		s >> identifier; // 先頭の識別子を読む
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
		} else if (identifier == "f") {
			RunaEngine::VertexData triangle[3];
			// 面は三角形限定。そのほかは未対応
			for (int32_t faceVertex = 0;faceVertex < 3;++faceVertex) {
				std::string vertexDefinition;
				s >> vertexDefinition;
				// 頂点の要素へのIndexを分解して取得
				std::istringstream v(vertexDefinition);
				uint32_t elementIndices[3];
				for (int32_t element = 0;element < 3;++element) {
					std::string index;
					std::getline(v, index, '/');
					elementIndices[element] = std::stoi(index);
				}

				// 要素へのIndexから、実際の要素の値を取得して、頂点を構築する
				RunaEngine::Vector4 position = positions[elementIndices[0] - 1];
				RunaEngine::Vector2 texcoord = texCoords[elementIndices[1] - 1];
				RunaEngine::Vector3 normal = normals[elementIndices[2] - 1];
				//VertexData vertex = { position, texcoord, normal };
				//modelData.vertices.push_back(vertex);
				triangle[faceVertex] = { position,texcoord,normal };
			}

			modelData.vertices.push_back(triangle[2]);
			modelData.vertices.push_back(triangle[1]);
			modelData.vertices.push_back(triangle[0]);

		} else if (identifier == "mtllib") {
			std::string materialFilename;
			s >> materialFilename;
			modelData.material = LoadMaterialTemplateFile(directoryPath, materialFilename);
		}
	}
	return modelData;
}

// =========================================================
// LoadMaterialTemplateFile 
// =========================================================
RunaEngine::MaterialData ModelLoader::LoadMaterialTemplateFile(const std::string& directoryPath, const std::string& filename) {
	RunaEngine::MaterialData materialData;
	std::string line;
	std::ifstream file(directoryPath + "/" + filename);
	assert(file.is_open());
	while (std::getline(file, line)) {
		std::string identifier;
		std::istringstream s(line);
		s >> identifier;

		if (identifier == "map_Kd") {
			std::string textureFilename;
			s >> textureFilename;
			materialData.textureFilePath = directoryPath + "/" + textureFilename;
		}
	}

	return materialData;
}
