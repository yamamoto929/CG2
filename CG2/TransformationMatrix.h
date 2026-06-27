#pragma once
#include "Matrix4x4.h"
namespace RunaEngine {
	struct TransformationMatrix {
		Matrix4x4 WVP;
		Matrix4x4 World;
	};
}
