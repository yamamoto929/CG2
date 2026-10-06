#pragma once
#include "RunaEngine/Core/DirectXCommon.h"
#include "RunaEngine/Graphics/DirectionalLight.h"
#include "RunaEngine/Graphics/GraphicsPipeline.h"
#include "RunaEngine/UI/ImGuiManager.h"
#include "RunaEngine/Input/Input.h"
#include "RunaEngine/Input/InputTypes.h"
#include "RunaEngine/Graphics/Object3D.h"
#include "RunaEngine/Graphics/ModelDrawParameters.h"
#include "RunaEngine/Graphics/Primitive3D.h"
#include "RunaEngine/Graphics/PrimitiveGraphicsPipeline.h"
#include "RunaEngine/Graphics/Renderer.h"
#include "RunaEngine/Graphics/SpriteGraphicsPipeline.h"
#include "RunaEngine/Core/ResourceManager.h"
#include "RunaEngine/Core/ShaderCompiler.h"
#include "RunaEngine/Audio/SoundManager.h"
#include "RunaEngine/Graphics/Sprite.h"
#include "RunaEngine/Core/TextureManager.h"
#include "RunaEngine/Math/Transform.h"
#include "RunaEngine/Math/Vector2.h"
#include "RunaEngine/Math/Vector3.h"
#include "RunaEngine/Math/Vector4.h"
#include "RunaEngine/Core/WinApp.h"
#include "RunaEngine/Graphics/SpriteDrawCommand.h"
#include "RunaEngine/Core/GameTimer.h"
#include <Windows.h>
#include <cstdint>
#include <cstddef>
#include <memory>
#include <string>
#include <vector>
#ifdef USE_IMGUI
#include "RunaEngine/externals/imgui/imgui.h"
#endif

namespace RunaEngine {
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
	Primitive3D* CreateTriangle3D();
	Primitive3D* CreateTriangle3D(const Vector4& color);
	void DestroySprite(Sprite*& sprite);
	void DestroyObject3D(Object3D*& object);
	void DestroyPrimitive3D(Primitive3D*& primitive);
	void DestroyModel(Model*& model);
	void UnloadTexture(uint32_t handle);
	void ClearScene();
	void SetObjectTexture(Object3D* object, const std::string& texturePath);

	void DrawSprite(Sprite* sprite, const Vector2& position);
	void DrawSprite(Sprite* sprite, const Vector3& translate);
	void DrawSprite(Sprite* sprite, const Transform& transform);

	void DrawModel(Model* model, const Vector3& translate);
	void DrawModel(Model* model, const Transform& transform);
	void DrawModel(Model* model, const Transform& transform, const ModelDrawParameters& parameters);
	void DrawObject3D(Object3D* object, const Vector3& translate);
	void DrawObject3D(Object3D* object, const Transform& transform);
	void DrawObject3D(Object3D* object, const Transform& transform, const ModelDrawParameters& parameters);
	void DrawObject3D(Object3D* object);
	void DrawObject3D(Object3D* object, const ModelDrawParameters& parameters);
	void DrawPrimitive3D(Primitive3D* primitive, const Vector3& translate);
	void DrawPrimitive3D(Primitive3D* primitive, const Transform& transform);
	void DrawPrimitive3D(Primitive3D* primitive);

	uint32_t LoadTexture(const std::string& texturePath);
	void SetModelTexture(Model* model, const std::string& texturePath);
	void SetSpriteTexture(Sprite* sprite, const std::string& texturePath);

	uint32_t LoadSound(const std::wstring& filePath);
	uint32_t LoadSound(const std::string& filePath);
	void PlaySound(uint32_t soundHandle, bool loop = false, float volume = 1.0f);

	bool IsPushKey(Key key);
	bool IsTriggerKey(Key key);
	Vector2 GetMousePosition() const;
	bool IsButtonDown(GamepadButton button);
	bool IsButtonTriggered(GamepadButton button);
	bool IsButtonReleased(GamepadButton button);
	bool IsGamepadConnected() const;
	Vector2 GetLeftStick() const;
	Vector2 GetRightStick() const;
	float GetLeftTrigger() const;
	float GetRightTrigger() const;

	Transform& GetCameraTransform() { return cameraTransform_; }
	const Transform& GetCameraTransform() const { return cameraTransform_; }
	void SetCameraTransform(const Transform& transform);
	void MoveCamera(const Vector3& move);

	ImGuiManager& GetImGuiManager() { return imGuiManager_; }
	TextureManager& GetTextureManager() { return textureManager_; }
	Renderer& GetRenderer() { return renderer_; }
	DirectionalLight& GetDirectionalLight() { return directionalLight_; }
	// deltaTime取得
	float GetDeltaTime()const;

private:
	Object3D* GetDrawModelObject(Model* model);
	void UpdateCameraMatrices();

	MSG msg_{};
	int32_t width_ = 0;
	int32_t height_ = 0;
	bool initialized_ = false;
	bool comInitialized_ = false;
	bool frameActive_ = false;
	bool sceneClearRequested_ = false;
	std::vector<Sprite*> pendingSprites_;
	std::vector<Object3D*> pendingObjects_;
	std::vector<Primitive3D*> pendingPrimitives_;
	std::vector<Model*> pendingModels_;
	std::vector<uint32_t> pendingTextures_;
	void ReleasePendingResources();
	bool IsTextureInUse(uint32_t handle) const;

	WinApp winApp_;
	DirectXCommon directXCommon_;
	TextureManager textureManager_;
	std::unique_ptr<Input> input_;
	SoundManager soundManager_;
	ShaderCompiler shaderCompiler_;
	SpriteGraphicsPipeline spriteGraphicsPipeline_;
	GraphicsPipeline graphicsPipeline_;
	PrimitiveGraphicsPipeline primitiveGraphicsPipeline_;
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
	std::vector<std::unique_ptr<Primitive3D>> primitive3Ds_;
	std::vector<std::unique_ptr<Object3D>> drawModelObjects_;
	size_t drawModelObjectIndex_ = 0;
	// Sprite描画順用キュー
	std::vector<SpriteDrawCommand> spriteDrawCommands_;
	uint64_t spriteSubmissionIndex_ = 0;
	// Sprite描画順並び替え
	void FlushSprites();
	// deltaTime用
	GameTimer gameTimer_;
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
Primitive3D* CreateTriangle3D();
Primitive3D* CreateTriangle3D(const Vector4& color);
void DestroySprite(Sprite*& sprite);
void DestroyObject3D(Object3D*& object);
void DestroyPrimitive3D(Primitive3D*& primitive);
void DestroyModel(Model*& model);
void UnloadTexture(uint32_t handle);
void ClearScene();
void SetObjectTexture(Object3D* object, const std::string& texturePath);

void DrawSprite(Sprite* sprite, const float& posX, const float& posY);
void DrawSprite(Sprite* sprite, const Vector2& position);
void DrawSprite(Sprite* sprite, const Vector3& translate);
void DrawSprite(Sprite* sprite, const Transform& transform);

void DrawModel(Model* model, const Vector3& translate);
void DrawModel(Model* model, const Transform& transform);
void DrawModel(Model* model, const Transform& transform, const ModelDrawParameters& parameters);
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
void DrawObject3D(Object3D* object, const Transform& transform, const ModelDrawParameters& parameters);
void DrawObject3D(Object3D* object);
void DrawObject3D(Object3D* object, const ModelDrawParameters& parameters);
void DrawPrimitive3D(Primitive3D* primitive, const Vector3& translate);
void DrawPrimitive3D(Primitive3D* primitive, const Transform& transform);
void DrawPrimitive3D(
	Primitive3D* primitive,
	float scaleX,
	float scaleY,
	float scaleZ,
	float rotateX,
	float rotateY,
	float rotateZ,
	float translateX,
	float translateY,
	float translateZ
);
void DrawPrimitive3D(Primitive3D* primitive);

uint32_t LoadTexture(const std::string& texturePath);
void SetModelTexture(Model* model, const std::string& texturePath);
void SetSpriteTexture(Sprite* sprite, const std::string& texturePath);

uint32_t LoadSound(const std::wstring& filePath);
uint32_t LoadSound(const std::string& filePath);
void PlaySound(uint32_t soundHandle, bool loop = false, float volume = 1.0f);

bool IsPushKey(Key key);
bool IsTriggerKey(Key key);
Vector2 GetMousePosition();
bool IsButtonDown(GamepadButton button);
bool IsButtonTriggered(GamepadButton button);
bool IsButtonReleased(GamepadButton button);
bool IsGamepadConnected();
Vector2 GetLeftStick();
Vector2 GetRightStick();
float GetLeftTrigger();
float GetRightTrigger();

Transform& GetCameraTransform();
void SetCameraTransform(const Transform& transform);
void MoveCamera(const Vector3& move);

ImGuiManager& GetImGuiManager();
TextureManager& GetTextureManager();
Renderer& GetRenderer();
DirectionalLight& GetDirectionalLight();
float GetDeltaTime();
}
