#pragma once
#include "Vector4.h"
#include "Matrix4x4.h"
#include "LightingMode.h"
#include <cstdint>
namespace RunaEngine{
	struct Material {
		RunaEngine::Vector4 color;
		LightingMode lightingMode;
		float padding[3];
		RunaEngine::Matrix4x4 uvTransform;
	};
}
