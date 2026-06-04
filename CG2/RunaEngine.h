#pragma once
#include "D3DResourceLeakChecker.h"
#include "DirectXCommon.h"
#include "DirectionalLight.h"
#include "GraphicsPipeline.h"
#include "ImGuiManager.h"
#include "Input.h"
#include "Object3D.h"
#include "Renderer.h"
#include "ResourceManager.h"
#include "ShaderCompiler.h"
#include "SoundManager.h"
#include "Sprite.h"
#include "TextureManager.h"
#include "Transform.h"
#include "Vector2.h"
#include "Vector3.h"
#include "WinApp.h"
#include <Windows.h>
#include <cstdint>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>
#ifdef USE_IMGUI
#include "externals/imgui/imgui.h"
#endif

namespace RunaEngine {

using Vector2 = ::Vector2;
using Vector3 = ::Vector3;
using Transform = ::Transform;
using Model = ::Model;
using Sprite = ::Sprite;
using Object3D = ::Object3D;

class Engine {
public:
	Engine();
	~Engine();

	void Initialize(int32_t width, int32_t height, const std::string& title);
	bool ProcessMessage();
	void BeginFrame();
	void EndFrame();
	void Shutdown();

	Sprite* CreateSprite(const std::string& texturePath);
	Model* CreateModel(const std::string& filePath);
	Object3D* CreateObject3D(Model* model);

	void DrawSprite(Sprite* sprite, const Vector2& position, float z = 0.0f);
	void DrawSprite(Sprite* sprite, const Vector3& translate);
	void DrawSprite(Sprite* sprite, const Transform& transform);

	void DrawModel(Model* model, const Vector3& translate);
	void DrawModel(Model* model, const Transform& transform);
	void DrawObject3D(Object3D* object, const Vector3& translate);
	void DrawObject3D(Object3D* object, const Transform& transform);
	void DrawObject3D(Object3D* object);

	uint32_t LoadTexture(const std::string& texturePath);
	void SetModelTexture(Model* model, const std::string& texturePath);
	void SetSpriteTexture(Sprite* sprite, const std::string& texturePath);

	uint32_t LoadSound(const std::wstring& filePath);
	uint32_t LoadSound(const std::string& filePath);
	void PlaySound(uint32_t soundHandle, bool loop = false, float volume = 1.0f);

	const BYTE* GetKey() const;
	bool IsPushKey(uint8_t key);
	Input& GetInput() { return input_; }
	const Input& GetInput() const { return input_; }

	Transform& GetCameraTransform() { return cameraTransform_; }
	const Transform& GetCameraTransform() const { return cameraTransform_; }
	void SetCameraTransform(const Transform& transform);
	void MoveCamera(const Vector3& move);

	ImGuiManager& GetImGuiManager() { return imGuiManager_; }
	TextureManager& GetTextureManager() { return textureManager_; }
	Renderer& GetRenderer() { return renderer_; }

private:
	Object3D* GetDefaultObject(Model* model);
	void UpdateCameraMatrices();

	D3DResourceLeakChecker leakChecker_;
	MSG msg_{};
	int32_t width_ = 0;
	int32_t height_ = 0;
	bool initialized_ = false;
	bool comInitialized_ = false;

	WinApp winApp_;
	DirectXCommon directXCommon_;
	TextureManager textureManager_;
	Input input_;
	SoundManager soundManager_;
	ShaderCompiler shaderCompiler_;
	GraphicsPipeline graphicsPipeline_;
	ResourceManager resourceManager_;
	DirectionalLight directionalLight_;
	Renderer renderer_;
	ImGuiManager imGuiManager_;

	Transform cameraTransform_{
		{ 1.0f, 1.0f, 1.0f },
		{ 0.0f, 0.0f, 0.0f },
		{ 0.0f, 0.0f, -10.0f },
	};
	Matrix4x4 viewMatrix_{};
	Matrix4x4 projectionMatrix_{};

	std::vector<std::unique_ptr<Sprite>> sprites_;
	std::vector<std::unique_ptr<Object3D>> object3Ds_;
	std::unordered_map<Model*, Object3D*> defaultObjects_;
};

Engine& GetEngine();

void Initialize(int32_t width, int32_t height, const std::string& title);
bool ProcessMessage();
void BeginFrame();
void EndFrame();
void Shutdown();

Sprite* CreateSprite(const std::string& texturePath);
Model* CreateModel(const std::string& filePath);
Object3D* CreateObject3D(Model* model);

void DrawSprite(Sprite* sprite, const Vector2& position, float z = 0.0f);
void DrawSprite(Sprite* sprite, const Vector3& translate);
void DrawSprite(Sprite* sprite, const Transform& transform);

void DrawModel(Model* model, const Vector3& translate);
void DrawModel(Model* model, const Transform& transform);
void DrawModel(
	Model* model,
	const float& scaleX,
	const float& scaleY,
	const float& scaleZ,
	const float& rotateX,
	const float& rotateY,
	const float& rotateZ,
	const float& translateX,
	const float& translateY,
	const float& translateZ
	);
void DrawObject3D(Object3D* object, const Vector3& translate);
void DrawObject3D(Object3D* object, const Transform& transform);
void DrawObject3D(Object3D* object);

uint32_t LoadTexture(const std::string& texturePath);
void SetModelTexture(Model* model, const std::string& texturePath);
void SetSpriteTexture(Sprite* sprite, const std::string& texturePath);

uint32_t LoadSound(const std::wstring& filePath);
uint32_t LoadSound(const std::string& filePath);
void PlaySound(uint32_t soundHandle, bool loop = false, float volume = 1.0f);

const BYTE* GetKey();
bool IsPushKey(uint8_t key);
Input& GetInput();

Transform& GetCameraTransform();
void SetCameraTransform(const Transform& transform);
void MoveCamera(const Vector3& move);

ImGuiManager& GetImGuiManager();
TextureManager& GetTextureManager();
Renderer& GetRenderer();

}
