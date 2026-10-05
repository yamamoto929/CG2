#include "TransformationMatrix.h"
#include "EngineError.h"
#include <cmath>

RunaEngine::Matrix4x4 MakeNormalMatrix(const RunaEngine::Matrix4x4& world) {
    // 平行移動を含まない3x3部分を使う。逆行列の数値精度のため、各行を正規化する。
    auto linear = MakeIdentityMatrix();
    double lengths[3]{};
    for (size_t row = 0; row < 3; ++row) {
        for (size_t col = 0; col < 3; ++col) {
            const double value = world.m[row][col];
            Require(std::isfinite(value), "World matrix contains a non-finite value");
            lengths[row] += value * value;
        }
        lengths[row] = std::sqrt(lengths[row]);
        Require(lengths[row] > 0.0, "Cannot light an object with zero scale");
        for (size_t col = 0; col < 3; ++col) {
            linear.m[row][col] = static_cast<float>(world.m[row][col] / lengths[row]);
        }
    }
    const double determinant =
        linear.m[0][0] * (linear.m[1][1] * linear.m[2][2] - linear.m[1][2] * linear.m[2][1]) -
        linear.m[0][1] * (linear.m[1][0] * linear.m[2][2] - linear.m[1][2] * linear.m[2][0]) +
        linear.m[0][2] * (linear.m[1][0] * linear.m[2][1] - linear.m[1][1] * linear.m[2][0]);
    Require(std::abs(determinant) > 1e-6, "World matrix has no inverse for normal transformation");
    auto result = Transpose(Inverse(linear));
    for (size_t row = 0; row < 3; ++row) {
        for (size_t col = 0; col < 3; ++col) {
            result.m[row][col] = static_cast<float>(result.m[row][col] / lengths[row]);
            Require(std::isfinite(result.m[row][col]), "Normal matrix is out of float range");
        }
    }
    return result;
}
