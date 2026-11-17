#include "Camera.h"
#include "RunaEngine/Math/AffineMatrix.h"
#include "RunaEngine/Math/WVPMatrix.h"
using namespace RunaEngine;
Camera::Camera()
	: transform_({ {1.0f,1.0f,1.0f},{0.0f,0.0f,0.0f},{0.0f,0.0f,-10.0f} })
	, fovy_(0.45f)
	, aspectRatio_(1280.0f / 720.0f)
	, nearClip_(0.1f)
	, farClip_(100.0f)
{	
	Update();
}
void Camera::Update() {
	worldMatrix_ = MakeAffineMatrix(
		transform_.scale,
		transform_.rotate,
		transform_.translate
	);

	viewMatrix_ = Inverse(worldMatrix_);
	projectionMatrix_ = MakePerspectiveFovMatrix(fovy_, aspectRatio_, nearClip_, farClip_);
	viewProjectionMatrix_ = Multiply(viewMatrix_, projectionMatrix_);
}