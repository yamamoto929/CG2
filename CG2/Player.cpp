#include "Player.h"
Player::~Player() {
	DestroySprite(sprite_);
}

void Player::Initialize() {
	sprite_ = CreateSprite("resources/sprite_test.png");
}

void Player::Update(float deltaTime) {
	const float move = moveSpeed_ * deltaTime;
	if (IsPushKey(Key::D)) {
		position_.x += move;
	} else if (IsPushKey(Key::A)) {
		position_.x -= move;
	}

	if (IsPushKey(Key::S)) {
		position_.y += move;
	} else if (IsPushKey(Key::W)) {
		position_.y -= move;
	}
}

void Player::Draw() {
	DrawSprite(sprite_, position_);
}