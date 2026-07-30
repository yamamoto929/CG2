#pragma once
#include "RunaEngine/RunaEngine.h"
#include "DrawableManager.h"
#include <variant>
#include <string>

class Object
{
public:
	Object(const std::string& name = "Object");

	// どの描画物を表示するかセットする
	void SetDrawable(DrawableID id);

	void Update();
	void Draw();

private:
	std::string name_;
	std::variant<RunaEngine::Sprite*, RunaEngine::Model*> object_;

	int currentDrawableID_ = static_cast<int>(DrawableID::NONE);

	int lightingModeIndex_ = 0;

	RunaEngine::Transform transform_{};
	RunaEngine::ModelDrawParameters modelParameters_{};

	static const inline RunaEngine::Transform initTransform{
		{1.0f, 1.0f, 1.0f},
		{0.0f, 3.1415f, 0.0f},
		{-2.0f, 0.0f, 0.0f}
	};
};