#pragma once
#include "Sprite.h"
#include "RunaEngine/math/Transform.h"
#include <cstdint>
namespace RunaEngine{
    struct SpriteDrawCommand {
        Sprite* sprite;
        Transform transform;
        uint64_t submissionIndex;
    };
}
