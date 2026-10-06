#pragma once
#include "Vector4.h"
#include "Matrix4x4.h"

namespace RunaEngine {

    struct alignas(16) SpriteMaterial {
        Vector4 color;
        Matrix4x4 uvTransform;
    };


}
