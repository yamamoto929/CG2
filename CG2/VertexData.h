#pragma once
#include "Vector2.h"
#include "Vector4.h"
#include "Vector3.h"

namespace RunaEngine {
	struct VertexData {
		Vector4 position;
		Vector2 texCoord;
		Vector3 normal;
	};
}
