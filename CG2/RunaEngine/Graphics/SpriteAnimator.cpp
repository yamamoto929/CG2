#include "SpriteAnimator.h"
#include <cassert>
using namespace RunaEngine;
void SpriteAnimator::Initialize(
	Sprite* sprite,
	const SpriteAnimationClip& clip
) {
	sprite_ = sprite;
	clip_ = clip;
	elapsedTime_ = 0.0f;
	currentFrame_ = 0;

	playing_ = true;
	ApplyCurrentFrame();
}

void SpriteAnimator::Update(float deltaTime) {
	elapsedTime_ += deltaTime;

	while (elapsedTime_ >= clip_.secondsPerFrame) {
		elapsedTime_ -= clip_.secondsPerFrame;
		++currentFrame_;

		if (currentFrame_ >= clip_.frameCount) {
			if (clip_.loop) {
				currentFrame_ = 0;
			} else {
				currentFrame_ = clip_.frameCount - 1;
				playing_ = false;
			}
		}

		ApplyCurrentFrame();
	}
}

void SpriteAnimator::Play() {
	playing_ = true;
	ApplyCurrentFrame();
}

void SpriteAnimator::Pause() {
	playing_ = false;
	ApplyCurrentFrame();
}

void SpriteAnimator::Reset() {
	currentFrame_ = 0;
	ApplyCurrentFrame();
}

void SpriteAnimator::ApplyCurrentFrame()
{
	assert(sprite_);
	assert(clip_.columnCount > 0);

	// シート全体でのフレーム番号
	const uint32_t frameIndex =
		clip_.startFrame + currentFrame_;

	// フレーム番号から列と行を求める
	const uint32_t column =
		frameIndex % clip_.columnCount;

	const uint32_t row =
		frameIndex / clip_.columnCount;

	// ピクセル単位の切り出し位置をSpriteへ適用
	sprite_->SetTextureRect(
		static_cast<float>(column) * clip_.frameWidth,
		static_cast<float>(row) * clip_.frameHeight,
		clip_.frameWidth,
		clip_.frameHeight
	);
}