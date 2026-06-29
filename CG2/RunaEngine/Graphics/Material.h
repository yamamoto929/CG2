#pragma once
#include "Vector4.h"
#include "Matrix4x4.h"
#include <cstdint>
namespace RunaEngine{
	struct Material {
		RunaEngine::Vector4 color;
		int32_t enableLighting;
		float padding[3];
		RunaEngine::Matrix4x4 uvTransform;
	};
}
