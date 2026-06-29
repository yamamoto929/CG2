#pragma once
namespace RunaEngine{
	struct Matrix4x4 {
		float m[4][4];
	};
}
RunaEngine::Matrix4x4 Add(const RunaEngine::Matrix4x4& m1, const RunaEngine::Matrix4x4& m2);
RunaEngine::Matrix4x4 Subtract(const RunaEngine::Matrix4x4& m1, const RunaEngine::Matrix4x4& m2);
RunaEngine::Matrix4x4 Multiply(const RunaEngine::Matrix4x4& m1, const RunaEngine::Matrix4x4& m2);
RunaEngine::Matrix4x4 Inverse(const RunaEngine::Matrix4x4& m);
RunaEngine::Matrix4x4 Transpose(const RunaEngine::Matrix4x4& m);
RunaEngine::Matrix4x4 MakeIdentityMatrix();


