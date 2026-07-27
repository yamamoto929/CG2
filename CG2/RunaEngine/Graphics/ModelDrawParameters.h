#pragma once
#include "LightingMode.h"
#include "Matrix4x4.h"
#include "Vector4.h"

namespace RunaEngine {
	struct ModelDrawParameters {
		Vector4 color{ 1.0f, 1.0f, 1.0f, 1.0f };
		LightingMode lightingMode = LightingMode::HALF_LAMBERT;
		Matrix4x4 uvTransform = MakeIdentityMatrix();
	};
}
