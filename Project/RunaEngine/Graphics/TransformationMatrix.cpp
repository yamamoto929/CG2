#include "TransformationMatrix.h"
#include <cmath>

RunaEngine::Matrix4x4 MakeNormalMatrix(const RunaEngine::Matrix4x4& world) {
    auto linear = MakeIdentityMatrix();
    double lengths[3]{};
    for (size_t row = 0; row < 3; ++row) {
        for (size_t col = 0; col < 3; ++col) {
            const double value = world.m[row][col];
            lengths[row] += value * value;
        }
        lengths[row] = std::sqrt(lengths[row]);
        // 倍率0のときは逆行列を作れないため、割り算をしない。
        if (lengths[row] == 0.0) { return MakeIdentityMatrix(); }
        for (size_t col = 0; col < 3; ++col) {
            linear.m[row][col] = static_cast<float>(world.m[row][col] / lengths[row]);
        }
    }
    const double determinant =
        linear.m[0][0] * (linear.m[1][1] * linear.m[2][2] - linear.m[1][2] * linear.m[2][1]) -
        linear.m[0][1] * (linear.m[1][0] * linear.m[2][2] - linear.m[1][2] * linear.m[2][0]) +
        linear.m[0][2] * (linear.m[1][0] * linear.m[2][1] - linear.m[1][1] * linear.m[2][0]);
    if (std::abs(determinant) < 1e-6) { return MakeIdentityMatrix(); }
    auto result = Transpose(Inverse(linear));
    for (size_t row = 0; row < 3; ++row) {
        for (size_t col = 0; col < 3; ++col) {
            result.m[row][col] = static_cast<float>(result.m[row][col] / lengths[row]);
        }
    }
    return result;
}
