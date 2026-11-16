#include "SpriteAnimator.h"
using namespace RunaEngine;
void SpriteAnimator::Initialize(
	Sprite* sprite,
	const SpriteAnimationClip& clip
) {
	if (!sprite || clip.frameCount == 0 || clip.columnCount == 0 || clip.secondsPerFrame <= 0) { return; }
	sprite_ = sprite;
	clip_ = clip;
	elapsedTime_ = 0.0f;
	currentFrame_ = 0;

	playing_ = true;
	ApplyCurrentFrame();
}

void SpriteAnimator::Update(float deltaTime) {
	if (!playing_ || !sprite_) {
		return;
	}

	elapsedTime_ += deltaTime;

	while (playing_&&
		elapsedTime_ >= clip_.secondsPerFrame) {
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
	elapsedTime_ = 0.0f;
	currentFrame_ = 0;
	ApplyCurrentFrame();
}

void SpriteAnimator::ApplyCurrentFrame()
{
	if (!sprite_) { return; }

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
