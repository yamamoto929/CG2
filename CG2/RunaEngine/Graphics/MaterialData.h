#pragma once
#include <string>
#include "Vector4.h"
#include "Vector3.h"
namespace RunaEngine {
	struct MaterialData {
		std::string textureFilePath;
		Vector4 color{1, 1, 1, 1};
		Vector3 specular{0, 0, 0};
		float shininess = 1.0f;
	};
}
