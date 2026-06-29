#include "Matrix4x4Function.h"
#include <cmath>
// 1. 行列の加法
Matrix4x4 Add(const Matrix4x4& m1, const Matrix4x4& m2) {
	Matrix4x4 result;
	for (size_t ai = 0; ai < 4; ai++) {
		for (size_t bi = 0; bi < 4; bi++) {
			result.m[ai][bi] = m1.m[ai][bi] + m2.m[ai][bi];
		}
	}
	return result;
};

// 2. 行列の減法
Matrix4x4 Subtract(const Matrix4x4& m1, const Matrix4x4& m2) {
	Matrix4x4 result;
	for (size_t ai = 0; ai < 4; ai++) {
		for (size_t bi = 0; bi < 4; bi++) {
			result.m[ai][bi] = m1.m[ai][bi] - m2.m[ai][bi];
		}
	}
	return result;
};

// 3. 行列の積
Matrix4x4 Multiply(const Matrix4x4& m1, const Matrix4x4& m2) {
	Matrix4x4 result;
	for (size_t ai = 0; ai < 4; ai++) {
		for (size_t bi = 0; bi < 4; bi++) {
			result.m[ai][bi] =
				m1.m[ai][0] * m2.m[0][bi] +
				m1.m[ai][1] * m2.m[1][bi] +
				m1.m[ai][2] * m2.m[2][bi] +
				m1.m[ai][3] * m2.m[3][bi];
		}
	}
	return result;
};

//4.逆行列
// 掃き出し法
Matrix4x4 Inverse(const Matrix4x4& m) {
	// 掃き出し法を行うために、左側の行列（元の行列のコピー）を用意
	Matrix4x4 matrix = m;
	// 右側の行列（単位行列からスタートし、最終的に逆行列になる）を用意
	Matrix4x4 result = MakeIdentity4x4();

	for (size_t i = 0; i < 4; i++) {

		// --- 0. 部分ピボット選択 ---
		// i列目の中で、i行目以降から絶対値が最大の要素を持つ行を探す
		size_t maxRow = i;
		float maxVal = std::abs(matrix.m[i][i]);

		for (size_t j = i + 1; j < 4; j++) {
			float val = std::abs(matrix.m[j][i]);
			if (val > maxVal) {
				maxVal = val;
				maxRow = j;
			}
		}

		// もし最大の絶対値がほぼ0なら、逆行列は存在しない（特異行列）
		// ※ここでは計算を打ち切って、安全のために単位行列を返す処理にしています
		const float epsilon = 1e-6f;
		if (maxVal < epsilon) {
			return MakeIdentity4x4();
		}

		// 最大の絶対値を持つ行が現在の行(i)と違う場合、行全体を入れ替える
		if (maxRow != i) {
			for (size_t j = 0; j < 4; j++) {
				std::swap(matrix.m[i][j], matrix.m[maxRow][j]);
				std::swap(result.m[i][j], result.m[maxRow][j]);
			}
		}

		// --- 1. ピボット（対角成分）の処理 ---
		float pivot = matrix.m[i][i];

		// 対角成分を1にするために、この行全体をピボットで割る（逆数を掛ける）
		float invPivot = 1.0f / pivot;
		for (size_t j = 0; j < 4; j++) {
			matrix.m[i][j] *= invPivot;
			result.m[i][j] *= invPivot;
		}

		// --- 2. ピボット列の掃き出し（他の行を0にする） ---
		for (size_t k = 0; k < 4; k++) {
			if (i == k) { continue; } // ピボット行自体はスキップ

			float factor = matrix.m[k][i]; // 削除するために掛ける係数

			// 行基本変形：k行目 = k行目 - factor * i行目
			for (size_t j = 0; j < 4; j++) {
				matrix.m[k][j] -= factor * matrix.m[i][j];
				result.m[k][j] -= factor * result.m[i][j];
			}
		}
	}

	return result;
};

//5.転置行列
Matrix4x4 Transpose(const Matrix4x4& m) {
	Matrix4x4 result = m;
	for (size_t ai = 0; ai < 4; ai++) {
		for (size_t bi = 0; bi < 4; bi++) {
			if (ai == bi) { continue; }
			result.m[ai][bi] = m.m[bi][ai];
		}
	}
	return result;
};

//6.単位行列の作成
Matrix4x4 MakeIdentity4x4() {
	Matrix4x4 identityMatrix;
	for (size_t ai = 0; ai < 4; ai++) {
		for (size_t bi = 0; bi < 4; bi++) {

			if (ai == bi) {
				identityMatrix.m[ai][bi] = 1.0f;
			} else {
				identityMatrix.m[ai][bi] = 0.0f;
			}
		}
	}
	return identityMatrix;
};