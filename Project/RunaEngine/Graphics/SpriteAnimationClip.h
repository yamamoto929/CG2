#pragma once
#include <cstdint>
struct SpriteAnimationClip {
    uint32_t startFrame = 0;
    uint32_t frameCount = 1;
    uint32_t columnCount = 1;

    float frameWidth = 0.0f;
    float frameHeight = 0.0f;
    float secondsPerFrame = 0.1f;

    bool loop = true;
};