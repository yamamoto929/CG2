#include "GameTimer.h"
#include <algorithm>

void GameTimer::Reset()
{
    previousTime_ = Clock::now();
    deltaTime_ = 0.0f;
    initialized_ = true;
}

void GameTimer::Tick()
{
    if (!initialized_) {
        Reset();
        return;
    }

    const Clock::time_point currentTime = Clock::now();

    const float elapsedTime =
        std::chrono::duration<float>(
            currentTime - previousTime_
        ).count();

    previousTime_ = currentTime;

    deltaTime_ = std::clamp(
        elapsedTime,
        0.0f,
        kMaxDeltaTime
    );
}