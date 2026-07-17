#pragma once
#include <chrono>
class GameTimer
{
public:
	void Reset();
	void Tick();

	float GetDeltaTime() const {
		return deltaTime_;
	}
private:
	using Clock = std::chrono::steady_clock;

	Clock::time_point previousTime_{};
	float deltaTime_ = 0.0f;
	bool initialized_ = false;

	static constexpr float kMaxDeltaTime = 0.1f;
};

