#pragma once
#include "LightingMode.h"
#include "Matrix4x4.h"
#include "Vector4.h"
#include <optional>
#include <cstdint>

namespace RunaEngine {
	struct ModelDrawParameters {
		Vector4 color{ 1.0f, 1.0f, 1.0f, 1.0f };
		LightingMode lightingMode = LightingMode::HALF_LAMBERT;
		Matrix4x4 uvTransform = MakeIdentityMatrix();
		std::optional<uint32_t> textureOverride;
	};
}
