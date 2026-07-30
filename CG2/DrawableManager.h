#pragma once
#include "RunaEngine/RunaEngine.h"
#include <variant>

// 描画できるもの（Model, Sprite）のカタログID
enum class DrawableID {
	NONE,
	// --- Models ---
	PLANE,
	SPHERE,
	TEAPOT,
	BUNNY,
	SUZANNE,
	MULTIMESH,
	// --- Sprites ---
	UV_CHECKER,

	COUNT // 常に最後に配置
};

class DrawableManager {
public:
	// エンジン初期化時に1回だけ呼んで、すべてのデータをメモリに乗せる
	static void LoadAll() {
		drawables_[static_cast<int>(DrawableID::NONE)] = static_cast<RunaEngine::Model*>(nullptr);

		// モデルの読み込み
		drawables_[static_cast<int>(DrawableID::PLANE)] = RunaEngine::CreateModel("resources/plane.obj");
		drawables_[static_cast<int>(DrawableID::SPHERE)] = RunaEngine::CreateModel("resources/Sphere.obj");
		drawables_[static_cast<int>(DrawableID::TEAPOT)] = RunaEngine::CreateModel("resources/teapot.obj");
		drawables_[static_cast<int>(DrawableID::BUNNY)] = RunaEngine::CreateModel("resources/bunny.obj");
		drawables_[static_cast<int>(DrawableID::SUZANNE)] = RunaEngine::CreateModel("resources/suzanne.obj");
		drawables_[static_cast<int>(DrawableID::MULTIMESH)] = RunaEngine::CreateModel("resources/multiMesh.obj");

		// スプライトの読み込み
		drawables_[static_cast<int>(DrawableID::UV_CHECKER)] = RunaEngine::CreateSprite("resources/uvChecker.png");
	}

	// 指定されたIDのリソース（variant）を返す
	static std::variant<RunaEngine::Sprite*, RunaEngine::Model*> Get(DrawableID id) {
		return drawables_[static_cast<int>(id)];
	}

private:
	// SpriteとModelを共通して持てる配列
	static inline std::variant<RunaEngine::Sprite*, RunaEngine::Model*> drawables_[static_cast<int>(DrawableID::COUNT)];
};