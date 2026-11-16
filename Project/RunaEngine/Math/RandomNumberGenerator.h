#pragma once
#include <random>

class RNG final {
private:
	std::mt19937 randomEngine;

	// コンストラクタとデストラクタの宣言
	RNG();
	~RNG() = default;

public:
	// コピーと代入を禁止
	RNG(const RNG&) = delete;
	RNG& operator=(const RNG&) = delete;

	// インスタンス取得関数の宣言
	static RNG& GetInstance();

	// 乱数生成関数の宣言
	float Generate(float min, float max);
	int Generate(int min, int max);
};