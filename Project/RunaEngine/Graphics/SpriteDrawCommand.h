#pragma once
#include "Sprite.h"
#include "Vector2.h"
#include "Vector4.h"
#include "RunaEngine/math/Transform.h"
#include <cstdint>
namespace RunaEngine{
    struct SpriteDrawCommand {
        Sprite* sprite = nullptr;
        Transform transform{};
        Vector4 color{};
        Vector2 size{};
        Vector2 pivot{};
        Vector2 uvLeftTop{};
        Vector2 uvRightBottom{};
        Matrix4x4 uvTransform = MakeIdentityMatrix();
        uint32_t textureHandle = 0;
        int32_t drawOrder = 0;
        uint64_t submissionIndex = 0;
    };
}
