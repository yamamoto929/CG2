#include "Camera.h"
#include "RunaEngine/Math/AffineMatrix.h"
using namespace RunaEngine;
void Camera::Update() {
    worldMatrix_ = MakeAffineMatrix(
        transform_.scale,
        transform_.rotate,
        transform_.translate
    );

    viewMatrix_ = Inverse(worldMatrix_);
}