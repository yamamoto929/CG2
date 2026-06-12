#include "TetrahedronEffect.h"
#include "AffineMatrix.h"
#include <cassert>
#include <cmath>

namespace {

constexpr float kSqrt2 = 1.4142135623f;
constexpr float kSqrt3 = 1.7320508075f;

RunaEngine::Vector3 Add(const RunaEngine::Vector3& a, const RunaEngine::Vector3& b) {
	return { a.x + b.x, a.y + b.y, a.z + b.z };
}

RunaEngine::Vector3 Subtract(const RunaEngine::Vector3& a, const RunaEngine::Vector3& b) {
	return { a.x - b.x, a.y - b.y, a.z - b.z };
}

RunaEngine::Vector3 Multiply(const RunaEngine::Vector3& v, float scalar) {
	return { v.x * scalar, v.y * scalar, v.z * scalar };
}

RunaEngine::Vector3 Divide(const RunaEngine::Vector3& v, float scalar) {
	return { v.x / scalar, v.y / scalar, v.z / scalar };
}

float Dot(const RunaEngine::Vector3& a, const RunaEngine::Vector3& b) {
	return a.x * b.x + a.y * b.y + a.z * b.z;
}

RunaEngine::Vector3 Cross(const RunaEngine::Vector3& a, const RunaEngine::Vector3& b) {
	return {
		a.y * b.z - a.z * b.y,
		a.z * b.x - a.x * b.z,
		a.x * b.y - a.y * b.x,
	};
}

RunaEngine::Vector3 Normalize(const RunaEngine::Vector3& v) {
	float length = std::sqrt(Dot(v, v));
	if (length <= 0.0001f) {
		return { 0.0f, 0.0f, 0.0f };
	}
	return Divide(v, length);
}

Matrix4x4 MakeRotationMatrix(const RunaEngine::Vector3& rotation) {
	return ::Multiply(
		::Multiply(MakeRotateXMatrix(rotation.x), MakeRotateYMatrix(rotation.y)),
		MakeRotateZMatrix(rotation.z)
	);
}

RunaEngine::Vector3 TransformVector(const RunaEngine::Vector3& v, const Matrix4x4& m) {
	return {
		v.x * m.m[0][0] + v.y * m.m[1][0] + v.z * m.m[2][0],
		v.x * m.m[0][1] + v.y * m.m[1][1] + v.z * m.m[2][1],
		v.x * m.m[0][2] + v.y * m.m[1][2] + v.z * m.m[2][2],
	};
}

Matrix4x4 MakeBasisMatrix(
	const RunaEngine::Vector3& xAxis,
	const RunaEngine::Vector3& yAxis,
	const RunaEngine::Vector3& zAxis
) {
	return { {
		{ xAxis.x, xAxis.y, xAxis.z, 0.0f },
		{ yAxis.x, yAxis.y, yAxis.z, 0.0f },
		{ zAxis.x, zAxis.y, zAxis.z, 0.0f },
		{ 0.0f, 0.0f, 0.0f, 1.0f },
	} };
}

RunaEngine::Vector3 ExtractEulerRotation(const Matrix4x4& m) {
	float rotateY = std::asin(-m.m[0][2]);
	float cosY = std::cos(rotateY);

	if (std::abs(cosY) <= 0.0001f) {
		return {
			0.0f,
			rotateY,
			std::atan2(-m.m[1][0], m.m[1][1]),
		};
	}

	return {
		std::atan2(m.m[1][2], m.m[2][2]),
		rotateY,
		std::atan2(m.m[0][1], m.m[0][0]),
	};
}

struct FaceTransform {
	RunaEngine::Vector3 center;
	Matrix4x4 rotation;
};

FaceTransform MakeFaceTransform(
	RunaEngine::Vector3 a,
	RunaEngine::Vector3 b,
	RunaEngine::Vector3 c
) {
	RunaEngine::Vector3 center = Divide(Add(Add(a, b), c), 3.0f);

	RunaEngine::Vector3 xAxis = Normalize(Subtract(b, a));
	RunaEngine::Vector3 yAxis = Normalize(Subtract(c, Multiply(Add(a, b), 0.5f)));
	RunaEngine::Vector3 zAxis = Normalize(Cross(xAxis, yAxis));

	if (Dot(zAxis, center) < 0.0f) {
		RunaEngine::Vector3 temp = b;
		b = c;
		c = temp;
		xAxis = Normalize(Subtract(b, a));
		yAxis = Normalize(Subtract(c, Multiply(Add(a, b), 0.5f)));
		zAxis = Normalize(Cross(xAxis, yAxis));
	}

	return { center, MakeBasisMatrix(xAxis, yAxis, zAxis) };
}

} // namespace

void TetrahedronEffect::Initialize() {
	triangleModel_ = RunaEngine::CreateModel("./Resources/triangle.obj");
	triangleModel_->SetColor({ 1.0f, 1.0f, 1.0f, 1.0f });
	triangleModel_->SetEnableLighting(true);
	RunaEngine::SetModelTexture(triangleModel_, "./resources/white.png");

	Reset();
}

void TetrahedronEffect::Reset() {
	state_ = State::Falling;

	const std::array<RunaEngine::Vector3, 8> positions = {
		RunaEngine::Vector3{ -4.2f, 5.2f, 11.0f },
		RunaEngine::Vector3{ -1.4f, 6.6f, 12.4f },
		RunaEngine::Vector3{ 1.4f, 5.8f, 11.0f },
		RunaEngine::Vector3{ 4.2f, 7.2f, 12.4f },
		RunaEngine::Vector3{ -4.2f, 9.0f, 14.0f },
		RunaEngine::Vector3{ -1.4f, 10.4f, 15.4f },
		RunaEngine::Vector3{ 1.4f, 9.6f, 14.0f },
		RunaEngine::Vector3{ 4.2f, 11.0f, 15.4f },
	};

	for (size_t i = 0; i < tetrahedrons_.size(); ++i) {
		TetrahedronPiece& tetrahedron = tetrahedrons_[i];
		tetrahedron.position = positions[i];
		tetrahedron.velocity = {
			0.0f,
			-(0.9f + float(i % 4) * 0.22f),
			0.0f,
		};
		tetrahedron.rotation = {
			float(i) * 0.27f,
			float(i) * 0.41f,
			float(i) * 0.19f,
		};
		tetrahedron.rotationVelocity = {
			(0.6f + float(i % 3) * 0.22f) * (i % 2 == 0 ? 1.0f : -1.0f),
			0.8f + float(i % 4) * 0.18f,
			(0.45f + float(i % 5) * 0.12f) * (i % 2 == 0 ? -1.0f : 1.0f),
		};
		tetrahedron.scale = 0.65f + float(i % 3) * 0.18f;
		tetrahedron.active = true;
	}
}

void TetrahedronEffect::Update(float deltaTime) {
	if (state_ == State::Falling) {
		bool hasActiveTetrahedron = false;
		const float offscreenY = -5.0f;

		for (TetrahedronPiece& tetrahedron : tetrahedrons_) {
			if (!tetrahedron.active) {
				continue;
			}

			tetrahedron.position = Add(tetrahedron.position, Multiply(tetrahedron.velocity, deltaTime));
			tetrahedron.rotation = Add(tetrahedron.rotation, Multiply(tetrahedron.rotationVelocity, deltaTime));

			if (tetrahedron.position.y < offscreenY) {
				tetrahedron.active = false;
			} else {
				hasActiveTetrahedron = true;
			}
		}

		if (!hasActiveTetrahedron) {
			state_ = State::Finished;
		}
	}
}

void TetrahedronEffect::Draw() {
	if (state_ == State::Finished) {
		return;
	}

	for (const TetrahedronPiece& tetrahedron : tetrahedrons_) {
		if (tetrahedron.active) {
			DrawTetrahedron(tetrahedron.position, tetrahedron.rotation, tetrahedron.scale);
		}
	}
}

void TetrahedronEffect::DrawTetrahedron(
	const RunaEngine::Vector3& position,
	const RunaEngine::Vector3& rotation,
	float scale
) {
	assert(triangleModel_);

	const float vertexScale = scale / kSqrt2;
	const std::array<RunaEngine::Vector3, 4> vertices = {
		RunaEngine::Vector3{ vertexScale, vertexScale, vertexScale },
		RunaEngine::Vector3{ -vertexScale, -vertexScale, vertexScale },
		RunaEngine::Vector3{ -vertexScale, vertexScale, -vertexScale },
		RunaEngine::Vector3{ vertexScale, -vertexScale, -vertexScale },
	};

	const std::array<std::array<int, 3>, 4> faces = {
		std::array<int, 3>{ 0, 2, 1 },
		std::array<int, 3>{ 0, 1, 3 },
		std::array<int, 3>{ 0, 3, 2 },
		std::array<int, 3>{ 1, 2, 3 },
	};

	Matrix4x4 bodyRotation = MakeRotationMatrix(rotation);
	const float modelScaleX = scale;
	const float modelScaleY = scale * kSqrt3 * 0.5f;
	const RunaEngine::Vector3 localTriangleCentroid = {
		0.0f,
		-modelScaleY / 3.0f,
		0.0f,
	};

	for (const std::array<int, 3>& face : faces) {
		FaceTransform faceTransform = MakeFaceTransform(
			vertices[face[0]],
			vertices[face[1]],
			vertices[face[2]]
		);

		Matrix4x4 finalRotation = ::Multiply(faceTransform.rotation, bodyRotation);
		RunaEngine::Vector3 finalEuler = ExtractEulerRotation(finalRotation);
		RunaEngine::Vector3 faceCenter = Add(
			TransformVector(faceTransform.center, bodyRotation),
			position
		);
		RunaEngine::Vector3 centroidOffset = TransformVector(localTriangleCentroid, finalRotation);
		RunaEngine::Vector3 translate = Subtract(faceCenter, centroidOffset);

		RunaEngine::DrawModel(
			triangleModel_,
			modelScaleX, modelScaleY, scale,
			finalEuler.x, finalEuler.y, finalEuler.z,
			translate.x, translate.y, translate.z
		);
	}
}
