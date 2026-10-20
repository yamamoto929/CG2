#pragma once
#include "BlendMode.h"
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
		// 未指定ならObject3Dの設定を使う。指定はこの描画だけに適用する。
		std::optional<BlendMode> blendMode;
	};
}
