#pragma once
#include "RunaEngine/Math/Transform.h"
#include "RunaEngine/Math/Matrix4x4.h"

namespace RunaEngine{
	class Camera
	{
	private:
		Transform transform_;
		Matrix4x4 worldMatrix_;
		Matrix4x4 viewMatrix_;
	public:
		void Update();
	};
}
