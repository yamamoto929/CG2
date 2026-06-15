#pragma once
#include "Vector3.h"
namespace RunaEngine{
struct Transform {
	RunaEngine::Vector3 scale;
	RunaEngine::Vector3 rotate;
	RunaEngine::Vector3 translate;
};
}