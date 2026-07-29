#pragma once
#include "RunaEngine/RunaEngine.h"
#include <variant>
#include <string>
class Object
{
public:
	void CreateSprite(std::string filePath);
	void CreateModel(std::string filePath);
	void Initialize();
	void Update();
	void Draw();
private:
	std::variant<RunaEngine::Sprite*, RunaEngine::Model*>object_;
};

