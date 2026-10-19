#pragma once
#include "RunaEngine/RunaEngine.h"
class Player {
public:
    ~Player();

    void Initialize();
    void Update(float deltaTime);
    void Draw();

private:
    RunaEngine::Sprite* sprite_ = nullptr;
    RunaEngine::Vector2 position_{ 40.0f, 40.0f };
    float moveSpeed_ = 300.0f;
};