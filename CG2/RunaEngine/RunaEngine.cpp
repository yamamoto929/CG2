#include "RunaEngine.h"
#include "AffineMatrix.h"
#include "CrashHandler.h"
#include "DirectXDebug.h"
#include "Log.h"
#include "WVPMatrix.h"
#include <cassert>
#include <filesystem>
#include <algorithm>

namespace RunaEngine {

	Engine::Engine() = default;

	Engine::~Engine() {
		Shutdown();
	}

	void Engine::Initialize(int32_t width, int32_t height, const std::string& title) {
		assert(!initialized_);

		width_ = width;
		height_ = height;

		HRESULT hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
		assert(SUCCEEDED(hr));
		comInitialized_ = true;

		InitLog();
		CrashHandler::Initialize();

		winApp_.CreateNewWindow(width_, height_, title);

		DirectXDebug::EnableDebugLayer();
		directXCommon_.Initialize(winApp_.GetHwnd(), width_, height_);
		DirectXDebug::SetupInfoQueue();

		textureManager_.Initialize(directXCommon_.GetDevice(), directXCommon_.GetCommandList(), 128);
		input_.Initialize(winApp_.GetHInstance(), winApp_.GetHwnd());
		soundManager_.Initialize();

		shaderCompiler_.Initialize();
		spriteGraphicsPipeline_.Initialize(
			directXCommon_.GetDevice(),
			&shaderCompiler_,
			directXCommon_.GetDXGIFormat(),
			DXGI_FORMAT_D24_UNORM_S8_UINT
		);
		graphicsPipeline_.Initialize(
			directXCommon_.GetDevice(),
			&shaderCompiler_,
			directXCommon_.GetDXGIFormat(),
			DXGI_FORMAT_D24_UNORM_S8_UINT
		);
		primitiveGraphicsPipeline_.Initialize(
			directXCommon_.GetDevice(),
			&shaderCompiler_,
			directXCommon_.GetDXGIFormat(),
			DXGI_FORMAT_D24_UNORM_S8_UINT
		);

		directionalLight_.Initialize(directXCommon_.GetDevice());
		renderer_.Initialize(&directXCommon_, &textureManager_, &graphicsPipeline_, &primitiveGraphicsPipeline_, &directionalLight_, &spriteGraphicsPipeline_);
		imGuiManager_.Initialize(winApp_, directXCommon_, textureManager_);

		UpdateCameraMatrices();
		initialized_ = true;
	}

	bool Engine::ProcessMessage() {
		while (PeekMessage(&msg_, nullptr, 0, 0, PM_REMOVE)) {
			if (msg_.message == WM_QUIT) {
				return false;
			}
			TranslateMessage(&msg_);
			DispatchMessage(&msg_);
		}
		return true;
	}

	void Engine::BeginFrame() {
		assert(initialized_);

		drawModelObjectIndex_ = 0;
		imGuiManager_.BeginFrame();
		input_.Update();
		soundManager_.Update();
		UpdateCameraMatrices();
		renderer_.Begin();
	}

	void Engine::EndFrame() {
		assert(initialized_);

		FlushSprites();
		imGuiManager_.Render(renderer_.GetCommandList());
		renderer_.End();
	}

	void Engine::Shutdown() {
		if (!initialized_ && !comInitialized_) {
			return;
		}

		if (initialized_) {
			imGuiManager_.Shutdown();
			soundManager_.Shutdown();
			CloseWindow(winApp_.GetHwnd());
			initialized_ = false;
		}

		if (comInitialized_) {
			CoUninitialize();
			comInitialized_ = false;
		}
	}

	Sprite* Engine::CreateSprite(const std::string& texturePath) {
		assert(initialized_);

		std::unique_ptr<Sprite> sprite = std::make_unique<Sprite>();
		sprite->Initialize(directXCommon_.GetDevice(), &textureManager_, texturePath);

		Sprite* result = sprite.get();
		sprites_.push_back(std::move(sprite));
		return result;
	}

	Model* Engine::CreateModel(const std::string& filePath) {
		assert(initialized_);
		std::filesystem::path path(filePath);

		std::string directoryPath = path.parent_path().generic_string();
		std::string fileName = path.filename().generic_string();
		if (directoryPath.empty()) {
			directoryPath = ".";
		}
		return resourceManager_.LoadModel(directXCommon_.GetDevice(), &textureManager_, directoryPath, fileName);
	}

	Object3D* Engine::CreateObject3D(Model* model) {
		assert(initialized_);
		assert(model);

		std::unique_ptr<Object3D> object = std::make_unique<Object3D>();
		object->Initialize(directXCommon_.GetDevice(), model);

		Object3D* result = object.get();
		object3Ds_.push_back(std::move(object));
		return result;
	}

	Primitive3D* Engine::CreateTriangle3D() {
		return CreateTriangle3D({ 1.0f, 0.2f, 0.1f, 1.0f });
	}

	Primitive3D* Engine::CreateTriangle3D(const Vector4& color) {
		assert(initialized_);

		std::unique_ptr<Primitive3D> primitive = std::make_unique<Primitive3D>();
		primitive->InitializeTriangle(directXCommon_.GetDevice());
		primitive->SetColor(color);

		Primitive3D* result = primitive.get();
		primitive3Ds_.push_back(std::move(primitive));
		return result;
	}

	void Engine::DrawSprite(Sprite* sprite, const Vector2& position) {
		assert(sprite);
		DrawSprite(sprite, Vector3{ position.x, position.y, 0.0f });
	}

	void Engine::DrawSprite(Sprite* sprite, const Vector3& translate) {
		assert(sprite);
		Transform transform{
			{ 1.0f, 1.0f, 1.0f },
			{ 0.0f, 0.0f, 0.0f },
			translate,
		};
		DrawSprite(sprite, transform);
	}

	void Engine::DrawSprite(Sprite* sprite, const Transform& transform) {
		assert(sprite);
		spriteDrawCommands_.push_back({
			   sprite,
			   transform,
			   spriteSubmissionIndex_++
			});
	}

	void Engine::DrawModel(Model* model, const Vector3& translate) {
		Transform transform{
			{ 1.0f, 1.0f, 1.0f },
			{ 0.0f, 0.0f, 0.0f },
			translate,
		};
		DrawModel(model, transform);
	}

	void Engine::DrawModel(Model* model, const Transform& transform) {
		DrawObject3D(GetDrawModelObject(model), transform);
	}

	void Engine::DrawObject3D(Object3D* object, const Vector3& translate) {
		Transform transform{
			{ 1.0f, 1.0f, 1.0f },
			{ 0.0f, 0.0f, 0.0f },
			translate,
		};
		DrawObject3D(object, transform);
	}

	void Engine::DrawObject3D(Object3D* object, const Transform& transform) {
		assert(object);

		object->GetTransform() = transform;
		DrawObject3D(object);
	}

	void Engine::DrawObject3D(Object3D* object) {
		assert(object);

		object->Update(viewMatrix_, projectionMatrix_);
		renderer_.Draw(*object);
	}

	void Engine::DrawPrimitive3D(Primitive3D* primitive, const Vector3& translate) {
		Transform transform{
			{ 1.0f, 1.0f, 1.0f },
			{ 0.0f, 0.0f, 0.0f },
			translate,
		};
		DrawPrimitive3D(primitive, transform);
	}

	void Engine::DrawPrimitive3D(Primitive3D* primitive, const Transform& transform) {
		assert(primitive);

		primitive->GetTransform() = transform;
		DrawPrimitive3D(primitive);
	}

	void Engine::DrawPrimitive3D(Primitive3D* primitive) {
		assert(primitive);

		primitive->Update(viewMatrix_, projectionMatrix_);
		renderer_.Draw(*primitive);
	}

	uint32_t Engine::LoadTexture(const std::string& texturePath) {
		assert(initialized_);
		return textureManager_.Load(texturePath);
	}

	void Engine::SetModelTexture(Model* model, const std::string& texturePath) {
		assert(model);
		model->SetTextureHandle(LoadTexture(texturePath));
	}

	void Engine::SetSpriteTexture(Sprite* sprite, const std::string& texturePath) {
		assert(sprite);
		sprite->SetTextureHandle(LoadTexture(texturePath));
	}

	uint32_t Engine::LoadSound(const std::wstring& filePath) {
		assert(initialized_);
		return soundManager_.Load(filePath);
	}

	uint32_t Engine::LoadSound(const std::string& filePath) {
		assert(initialized_);
		return soundManager_.Load(filePath);
	}

	void Engine::PlaySound(uint32_t soundHandle, bool loop, float volume) {
		assert(initialized_);
		soundManager_.Play(soundHandle, loop, volume);
	}

	const BYTE* Engine::GetKey() const {
		return input_.GetKey();
	}

	bool Engine::IsPushKey(uint8_t key) {
		return input_.IsPushkey(key);
	}

	void Engine::SetCameraTransform(const Transform& transform) {
		cameraTransform_ = transform;
		UpdateCameraMatrices();
	}

	void Engine::MoveCamera(const Vector3& move) {
		cameraTransform_.translate += move;
		UpdateCameraMatrices();
	}

	Object3D* Engine::GetDrawModelObject(Model* model) {
		assert(model);

		if (drawModelObjectIndex_ >= drawModelObjects_.size()) {
			std::unique_ptr<Object3D> object = std::make_unique<Object3D>();
			object->Initialize(directXCommon_.GetDevice(), model);
			drawModelObjects_.push_back(std::move(object));
		}

		Object3D* object = drawModelObjects_[drawModelObjectIndex_].get();
		object->SetModel(model);
		++drawModelObjectIndex_;
		return object;
	}

	void Engine::UpdateCameraMatrices() {
		Matrix4x4 cameraMatrix = MakeAffineMatrix(
			cameraTransform_.scale,
			cameraTransform_.rotate,
			cameraTransform_.translate
		);
		viewMatrix_ = Inverse(cameraMatrix);
		projectionMatrix_ = MakePerspectiveFovMatrix(
			0.45f,
			float(width_) / float(height_),
			0.1f,
			100.0f
		);
	}

	void Engine::FlushSprites()
	{
		std::sort(
			spriteDrawCommands_.begin(),
			spriteDrawCommands_.end(),
			[](const SpriteDrawCommand& a, const SpriteDrawCommand& b)
			{
				const int32_t aOrder = a.sprite->GetDrawOrder();
				const int32_t bOrder = b.sprite->GetDrawOrder();

				if (aOrder != bOrder) {
					return aOrder < bOrder;
				}

				return a.submissionIndex < b.submissionIndex;
			}
		);

		for (const SpriteDrawCommand& command : spriteDrawCommands_) {
			command.sprite->GetTransform() = command.transform;
			command.sprite->Update(width_, height_);
			renderer_.Draw(*command.sprite);
		}

		spriteDrawCommands_.clear();
		spriteSubmissionIndex_ = 0;
	}

	Engine& GetEngine() {
		static Engine engine;
		return engine;
	}

	void Initialize(int32_t width, int32_t height, const std::string& title) {
		GetEngine().Initialize(width, height, title);
	}

	bool ProcessMessage() {
		return GetEngine().ProcessMessage();
	}

	void BeginFrame() {
		GetEngine().BeginFrame();
	}

	void EndFrame() {
		GetEngine().EndFrame();
	}

	void Shutdown() {
		GetEngine().Shutdown();
	}

	Sprite* CreateSprite(const std::string& texturePath) {
		return GetEngine().CreateSprite(texturePath);
	}

	Model* CreateModel(const std::string& filePath) {
		return GetEngine().CreateModel(filePath);
	}

	Object3D* CreateObject3D(Model* model) {
		return GetEngine().CreateObject3D(model);
	}

	Primitive3D* CreateTriangle3D() {
		return GetEngine().CreateTriangle3D();
	}

	Primitive3D* CreateTriangle3D(const Vector4& color) {
		return GetEngine().CreateTriangle3D(color);
	}

	void DrawSprite(Sprite* sprite, const Vector2& position) {
		GetEngine().DrawSprite(sprite, position);
	}

	void DrawSprite(Sprite* sprite, const Vector3& translate) {
		GetEngine().DrawSprite(sprite, translate);
	}

	void DrawSprite(Sprite* sprite, const Transform& transform) {
		GetEngine().DrawSprite(sprite, transform);
	}

	void DrawModel(Model* model, const Vector3& translate) {
		GetEngine().DrawModel(model, translate);
	}

	void DrawModel(Model* model, const Transform& transform) {
		GetEngine().DrawModel(model, transform);
	}

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
	) {
		Transform transform{
			{ scaleX, scaleY, scaleZ },
			{ rotateX, rotateY, rotateZ },
			{ translateX, translateY, translateZ },
		};
		GetEngine().DrawModel(model, transform);
	}


	void DrawObject3D(Object3D* object, const Vector3& translate) {
		GetEngine().DrawObject3D(object, translate);
	}

	void DrawObject3D(Object3D* object, const Transform& transform) {
		GetEngine().DrawObject3D(object, transform);
	}

	void DrawObject3D(Object3D* object) {
		GetEngine().DrawObject3D(object);
	}

	void DrawPrimitive3D(Primitive3D* primitive, const Vector3& translate) {
		GetEngine().DrawPrimitive3D(primitive, translate);
	}

	void DrawPrimitive3D(Primitive3D* primitive, const Transform& transform) {
		GetEngine().DrawPrimitive3D(primitive, transform);
	}

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
	) {
		Transform transform{
			{ scaleX, scaleY, scaleZ },
			{ rotateX, rotateY, rotateZ },
			{ translateX, translateY, translateZ },
		};
		GetEngine().DrawPrimitive3D(primitive, transform);
	}

	void DrawPrimitive3D(Primitive3D* primitive) {
		GetEngine().DrawPrimitive3D(primitive);
	}

	uint32_t LoadTexture(const std::string& texturePath) {
		return GetEngine().LoadTexture(texturePath);
	}

	void SetModelTexture(Model* model, const std::string& texturePath) {
		GetEngine().SetModelTexture(model, texturePath);
	}

	void SetSpriteTexture(Sprite* sprite, const std::string& texturePath) {
		GetEngine().SetSpriteTexture(sprite, texturePath);
	}

	uint32_t LoadSound(const std::wstring& filePath) {
		return GetEngine().LoadSound(filePath);
	}

	uint32_t LoadSound(const std::string& filePath) {
		return GetEngine().LoadSound(filePath);
	}

	void PlaySound(uint32_t soundHandle, bool loop, float volume) {
		GetEngine().PlaySound(soundHandle, loop, volume);
	}

	const BYTE* GetKey() {
		return GetEngine().GetKey();
	}

	bool IsPushKey(uint8_t key) {
		return GetEngine().IsPushKey(key);
	}

	Input& GetInput() {
		return GetEngine().GetInput();
	}

	Transform& GetCameraTransform() {
		return GetEngine().GetCameraTransform();
	}

	void SetCameraTransform(const Transform& transform) {
		GetEngine().SetCameraTransform(transform);
	}

	void MoveCamera(const Vector3& move) {
		GetEngine().MoveCamera(move);
	}

	ImGuiManager& GetImGuiManager() {
		return GetEngine().GetImGuiManager();
	}

	TextureManager& GetTextureManager() {
		return GetEngine().GetTextureManager();
	}

	Renderer& GetRenderer() {
		return GetEngine().GetRenderer();
	}

}
