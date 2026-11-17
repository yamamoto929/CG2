#pragma once
#include "RunaEngine/Math/Transform.h"
#include "RunaEngine/Math/Matrix4x4.h"

namespace RunaEngine {
	class Camera
	{
	private:
		Transform transform_{};
		Matrix4x4 worldMatrix_{};
		Matrix4x4 viewMatrix_{};

		// プロジェクション行列
		Matrix4x4 projectionMatrix_{};
		float fovy_ = 0.0f;
		float aspectRatio_ = 0.0f;
		float nearClip_ = 0.0f;
		float farClip_ = 0.0f;

		// 合成行列
		Matrix4x4 viewProjectionMatrix_{};

	public:
		Camera();
		void Update();
		// セッター
		void SetRotate(const Vector3& rotate) { transform_.rotate = rotate; }
		void SetTranslate(const Vector3& translate) { transform_.translate = translate; }
		void SetFovY(float fovy) { fovy_ = fovy; }
		void SetAspectRatio(float aspectRatio) { aspectRatio_ = aspectRatio; }
		void SetNearClip(float nearClip) { nearClip_ = nearClip; }
		void SetFarClip(float farClip) { farClip_ = farClip; }
		// ゲッター
		const Matrix4x4& Get() const { return worldMatrix_; }
		const Matrix4x4& GetViewMatrix() const { return viewMatrix_; }
		const Matrix4x4& GetProjectionMatrix() const { return projectionMatrix_; }
		const Matrix4x4& GetViewProjectionMatrix() const { return viewProjectionMatrix_; }
		const Vector3& GetRotate()const { return transform_.rotate; }
		const Vector3& GetTranslate()const { return transform_.translate; }
	};

}// namespace RunaEngine
