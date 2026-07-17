#pragma once
#include "SpriteAnimationClip.h"
#include "Sprite.h"
#include <cstdint>
using namespace RunaEngine;
class SpriteAnimator {
public:
	void Initialize(
		Sprite* sprite,
		const SpriteAnimationClip& clip
	);

	void Update(float deltaTime);

	void Play();
	void Pause();
	void Reset();
	void ApplyCurrentFrame();

private:
	Sprite* sprite_ = nullptr;
	SpriteAnimationClip clip_{};

	float elapsedTime_ = 0.0f;
	uint32_t currentFrame_ = 0;

	bool playing_ = true;
};

