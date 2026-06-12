#pragma once
#include "RunaEngine.h"
#include <array>

class TetrahedronEffect {
public:
	void Initialize();
	void Reset();
	void Update(float deltaTime = 1.0f / 60.0f);
	void Draw();

	bool IsFinished() const { return state_ == State::Finished; }

private:
	enum class State {
		Falling,
		Finished,
	};

	struct TetrahedronPiece {
		RunaEngine::Vector3 position{};
		RunaEngine::Vector3 velocity{};
		RunaEngine::Vector3 rotation{};
		RunaEngine::Vector3 rotationVelocity{};
		float scale = 1.0f;
		bool active = false;
	};

	void DrawTetrahedron(
		const RunaEngine::Vector3& position,
		const RunaEngine::Vector3& rotation,
		float scale
	);

	RunaEngine::Model* triangleModel_ = nullptr;
	State state_ = State::Falling;

	std::array<TetrahedronPiece, 8> tetrahedrons_{};
};
