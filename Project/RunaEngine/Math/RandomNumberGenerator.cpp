#include "RandomNumberGenerator.h"

// コンストラクタ
// エンジンを初期化する
RNG::RNG() {
	std::random_device seedGenerator;
	randomEngine.seed(seedGenerator());
}

// インスタンスの取得
RNG& RNG::GetInstance() {
	static RNG instance;
	return instance;
}

// 乱数生成（float）
float RNG::Generate(float min, float max) {
	std::uniform_real_distribution<float> distribution(min, max);
	return distribution(randomEngine);
}

// 乱数生成（int）
int RNG::Generate(int min, int max) {
	std::uniform_int_distribution<int> distribution(min, max);
	return distribution(randomEngine);
}