#pragma once
#include "Matrix4x4.h"
namespace RunaEngine {
	struct TransformationMatrix {
		Matrix4x4 WVP;
		Matrix4x4 World;
		Matrix4x4 WorldInverseTranspose;
	};
static_assert(sizeof(TransformationMatrix) == 192, "Keep the C++ and HLSL matrix layouts identical");
}

// 形の拡大と、面の向きの変換は別々に計算する。
RunaEngine::Matrix4x4 MakeNormalMatrix(const RunaEngine::Matrix4x4& world);
