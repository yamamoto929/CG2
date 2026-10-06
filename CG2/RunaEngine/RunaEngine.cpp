#include "RunaEngine.h"
#include "AffineMatrix.h"
#include "WVPMatrix.h"
#include <cassert>
#include <filesystem>
#include <algorithm>

namespace RunaEngine {
	namespace {
		template<class T> bool Pending(const std::vector<T*>& items, const T* value) {
			return std::find(items.begin(), items.end(), value) != items.end();
		}
	}

	Engine::Engine() = default;

	Engine::~Engine() {
		Shutdown();
	}

	void Engine::Initialize(int32_t width, int32_t height, const std::string& title) {
		width_ = width;
		height_ = height;
		msg_ = {};
		while (PeekMessage(&msg_, nullptr, WM_QUIT, WM_QUIT, PM_REMOVE)) {}

		HRESULT hr = CoInitializeEx(nullptr, COINIT_MULTITHREADED);
		assert(SUCCEEDED(hr));
		comInitialized_ = true;

		winApp_.CreateNewWindow(width_, height_, title);

		directXCommon_.Initialize(winApp_.GetHwnd(), width_, height_);

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
		gameTimer_.Reset();
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
		for (auto& sprite : sprites_) { sprite->BeginFrame(); }
		for (auto& object : object3Ds_) { object->BeginFrame(); }
		for (auto& primitive : primitive3Ds_) { primitive->BeginFrame(); }
		for (auto& object : drawModelObjects_) { object->BeginFrame(); }
		resourceManager_.BeginFrame();
		directionalLight_.BeginFrame();
		frameActive_ = true;

		gameTimer_.Tick();
		drawModelObjectIndex_ = 0;
		imGuiManager_.BeginFrame();
		input_.Update();
		soundManager_.Update();
		UpdateCameraMatrices();
		renderer_.Begin();
	}

	void Engine::EndFrame() {
		FlushSprites();
		imGuiManager_.Render(renderer_.GetCommandList());
		renderer_.End();
		frameActive_ = false;
		textureManager_.ReleaseUploadResources();
		ReleasePendingResources();
	}

	void Engine::Shutdown() {
		if (!initialized_ && !comInitialized_) {
			return;
		}

		if (initialized_) {
			// 未送信の描画を含めて完了させてから、参照先を片付ける。
			directXCommon_.FlushCommands();
			frameActive_ = false;
			textureManager_.ReleaseUploadResources();
			sceneClearRequested_ = true;
			ReleasePendingResources();
			imGuiManager_.Shutdown();
			soundManager_.Shutdown();
			input_.Shutdown();
			directionalLight_.Shutdown();
			graphicsPipeline_.Shutdown(); spriteGraphicsPipeline_.Shutdown(); primitiveGraphicsPipeline_.Shutdown();
			textureManager_.Shutdown(); shaderCompiler_.Shutdown();
			if (IsWindow(winApp_.GetHwnd())) { DestroyWindow(winApp_.GetHwnd()); }
			directXCommon_.Shutdown();
			initialized_ = false;
		}

		if (comInitialized_) {
			CoUninitialize();
			comInitialized_ = false;
		}
	}

	Sprite* Engine::CreateSprite(const std::string& texturePath) {
		std::unique_ptr<Sprite> sprite = std::make_unique<Sprite>();
		sprite->Initialize(directXCommon_.GetDevice(), &textureManager_, texturePath);
		if (sprite->GetTextureHandle() == 0) { return nullptr; }

		Sprite* result = sprite.get();
		sprites_.push_back(std::move(sprite));
		return result;
	}

	Model* Engine::CreateModel(const std::string& filePath) {
		std::filesystem::path path(filePath);

		std::string directoryPath = path.parent_path().generic_string();
		std::string fileName = path.filename().generic_string();
		if (directoryPath.empty()) {
			directoryPath = ".";
		}
		Model* model = resourceManager_.LoadModel(directXCommon_.GetDevice(), &textureManager_, directoryPath, fileName);

		return model;
	}

	Object3D* Engine::CreateObject3D(Model* model) {
		if (!model) { return nullptr; }

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
		std::unique_ptr<Primitive3D> primitive = std::make_unique<Primitive3D>();
		primitive->InitializeTriangle(directXCommon_.GetDevice());
		primitive->SetColor(color);

		Primitive3D* result = primitive.get();
		primitive3Ds_.push_back(std::move(primitive));
		return result;
	}

	void Engine::DestroySprite(Sprite*& sprite) {
		if (!sprite) { return; }

		pendingSprites_.push_back(sprite);
		sprite = nullptr;
		if (!frameActive_) { ReleasePendingResources(); }
	}
	void Engine::DestroyObject3D(Object3D*& object) {
		if (!object) { return; }

		pendingObjects_.push_back(object);
		object = nullptr;
		if (!frameActive_) { ReleasePendingResources(); }
	}
	void Engine::DestroyPrimitive3D(Primitive3D*& primitive) {
		if (!primitive) { return; }

		pendingPrimitives_.push_back(primitive);
		primitive = nullptr;
		if (!frameActive_) { ReleasePendingResources(); }
	}
	void Engine::DestroyModel(Model*& model) {
		if (!model) { return; }
		// この形を使う物体が残っている間は削除しない。
		for (const auto& object : object3Ds_) {
			if (object->GetModel() == model && !Pending(pendingObjects_, object.get())) { return; }
		}

		pendingModels_.push_back(model);
		model = nullptr;
		if (!frameActive_) { ReleasePendingResources(); }
	}
	void Engine::UnloadTexture(uint32_t handle) {
		if (IsTextureInUse(handle)) { return; }
		if (std::find(pendingTextures_.begin(), pendingTextures_.end(), handle) != pendingTextures_.end()) { return; }

		pendingTextures_.push_back(handle);
		if (!frameActive_) { ReleasePendingResources(); }
	}
	void Engine::ClearScene() {
		sceneClearRequested_ = true;
		if (!frameActive_) { ReleasePendingResources(); }
	}
	void Engine::SetObjectTexture(Object3D* object, const std::string& texturePath) {
		const uint32_t handle = LoadTexture(texturePath);
		if (handle != 0) { object->SetTextureHandle(handle); }
	}
	bool Engine::IsTextureInUse(uint32_t handle) const {
		return resourceManager_.UsesTexture(handle)
			|| std::any_of(sprites_.begin(), sprites_.end(), [handle](const auto& sprite) { return sprite->GetTextureHandle() == handle; })
			|| std::any_of(object3Ds_.begin(), object3Ds_.end(), [handle](const auto& object) { return object->GetTextureOverride() == handle; });
	}
	void Engine::ReleasePendingResources() {
		if (frameActive_) { return; }

		if (!sceneClearRequested_ && pendingSprites_.empty() && pendingObjects_.empty()
			&& pendingPrimitives_.empty() && pendingModels_.empty() && pendingTextures_.empty()) { return; }
		// LoadTexture直後の削除でも、まだ送信していないコピー命令を先に完了させる。
		if (textureManager_.HasPendingUploads()) {
			directXCommon_.FlushCommands();
			textureManager_.ReleaseUploadResources();
		} else { directXCommon_.WaitForIdle(); }
		if (sceneClearRequested_) {
			spriteDrawCommands_.clear();
			sprites_.clear(); object3Ds_.clear(); primitive3Ds_.clear(); drawModelObjects_.clear();
			resourceManager_.Clear();
			textureManager_.Clear();
			drawModelObjectIndex_ = 0; spriteSubmissionIndex_ = 0;
			sceneClearRequested_ = false;
		} else {
			std::erase_if(sprites_, [this](const auto& item) { return Pending(pendingSprites_, item.get()); });
			std::erase_if(object3Ds_, [this](const auto& item) { return Pending(pendingObjects_, item.get()); });
			std::erase_if(primitive3Ds_, [this](const auto& item) { return Pending(pendingPrimitives_, item.get()); });
			for (auto* model : pendingModels_) {
				bool inUse = false;
				for (const auto& object : object3Ds_) {
					if (object->GetModel() == model) { inUse = true; break; }
				}
				if (inUse) { continue; }

				std::erase_if(drawModelObjects_, [model](const auto& object) { return object->GetModel() == model; });
				resourceManager_.DestroyModel(model);
			}
			for (uint32_t handle : pendingTextures_) {
				if (IsTextureInUse(handle)) { continue; }

				textureManager_.Unload(handle);
			}
		}
		pendingSprites_.clear(); pendingObjects_.clear(); pendingPrimitives_.clear(); pendingModels_.clear(); pendingTextures_.clear();
	}

	void Engine::DrawSprite(Sprite* sprite, const Vector2& position) {
		DrawSprite(sprite, Vector3{ position.x, position.y, 0.0f });
	}

	void Engine::DrawSprite(Sprite* sprite, const Vector3& translate) {
		Transform transform{
			{ 1.0f, 1.0f, 1.0f },
			{ 0.0f, 0.0f, 0.0f },
			translate,
		};
		DrawSprite(sprite, transform);
	}

	void Engine::DrawSprite(Sprite* sprite, const Transform& transform) {
		spriteDrawCommands_.push_back(sprite->CaptureDrawCommand(transform, spriteSubmissionIndex_++));
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
		DrawModel(model, transform, ModelDrawParameters{});
	}

	void Engine::DrawModel(
		Model* model,
		const Transform& transform,
		const ModelDrawParameters& parameters
	) {
		DrawObject3D(GetDrawModelObject(model), transform, parameters);
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
		DrawObject3D(object, transform, ModelDrawParameters{});
	}

	void Engine::DrawObject3D(
		Object3D* object,
		const Transform& transform,
		const ModelDrawParameters& parameters
	) {
		object->GetTransform() = transform;
		DrawObject3D(object, parameters);
	}

	void Engine::DrawObject3D(Object3D* object) {
		DrawObject3D(object, ModelDrawParameters{});
	}

	void Engine::DrawObject3D(Object3D* object, const ModelDrawParameters& parameters) {
		object->Update(viewMatrix_, projectionMatrix_);
		renderer_.Draw(*object, parameters);
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
		primitive->GetTransform() = transform;
		DrawPrimitive3D(primitive);
	}

	void Engine::DrawPrimitive3D(Primitive3D* primitive) {
		primitive->Update(viewMatrix_, projectionMatrix_);
		renderer_.Draw(*primitive);
	}

	uint32_t Engine::LoadTexture(const std::string& texturePath) {
		return textureManager_.Load(texturePath);
	}

	void Engine::SetModelTexture(Model* model, const std::string& texturePath) {
		const uint32_t handle = LoadTexture(texturePath);
		if (handle != 0) { model->SetTextureHandle(handle); }
	}

	void Engine::SetSpriteTexture(Sprite* sprite, const std::string& texturePath) {
		sprite->SetTextureHandle(LoadTexture(texturePath));
	}

	uint32_t Engine::LoadSound(const std::wstring& filePath) {
		return soundManager_.Load(filePath);
	}

	uint32_t Engine::LoadSound(const std::string& filePath) {
		return soundManager_.Load(filePath);
	}

	void Engine::PlaySound(uint32_t soundHandle, bool loop, float volume) {
		soundManager_.Play(soundHandle, loop, volume);
	}

	bool Engine::IsPushKey(Key key) {
		return input_.IsPushkey(key);
	}

	bool Engine::IsTriggerKey(Key key) {
		return input_.IsTriggerkey(key);
	}

	Vector2 Engine::GetMousePosition() const {
		const MousePosition position = input_.GetMousePosition();
		return {
			static_cast<float>(position.x),
			static_cast<float>(position.y),
		};
	}

	bool Engine::IsButtonDown(GamepadButton button) {
		return input_.IsGamepadButtonDown(button);
	}

	bool Engine::IsButtonTriggered(GamepadButton button) {
		return input_.IsGamepadButtonTriggered(button);
	}

	bool Engine::IsButtonReleased(GamepadButton button) {
		return input_.IsGamepadButtonReleased(button);
	}

	bool Engine::IsGamepadConnected() const {
		return input_.IsGamepadConnected();
	}

	Vector2 Engine::GetLeftStick() const {
		const Input::StickState stick = input_.GetLeftStick();
		return { stick.horizontal, stick.vertical };
	}

	Vector2 Engine::GetRightStick() const {
		const Input::StickState stick = input_.GetRightStick();
		return { stick.horizontal, stick.vertical };
	}

	float Engine::GetLeftTrigger() const {
		return input_.GetLeftTrigger();
	}

	float Engine::GetRightTrigger() const {
		return input_.GetRightTrigger();
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
				const int32_t aOrder = a.drawOrder;
				const int32_t bOrder = b.drawOrder;

				if (aOrder != bOrder) {
					return aOrder < bOrder;
				}

				return a.submissionIndex < b.submissionIndex;
			}
		);

		for (const SpriteDrawCommand& command : spriteDrawCommands_) {
			renderer_.Draw(*command.sprite, command, width_, height_);
		}

		spriteDrawCommands_.clear();
		spriteSubmissionIndex_ = 0;
	}

	float Engine::GetDeltaTime()const {
		return gameTimer_.GetDeltaTime();
	}

	//=================================================================================
	// Engine
	//=================================================================================

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

	void DrawSprite(Sprite* sprite, const float& posX, const float& posY) {
		GetEngine().DrawSprite(sprite, Vector2{ posX,posY });
	}

	void DestroySprite(Sprite*& sprite) { GetEngine().DestroySprite(sprite); }
	void DestroyObject3D(Object3D*& object) { GetEngine().DestroyObject3D(object); }
	void DestroyPrimitive3D(Primitive3D*& primitive) { GetEngine().DestroyPrimitive3D(primitive); }
	void DestroyModel(Model*& model) { GetEngine().DestroyModel(model); }
	void UnloadTexture(uint32_t handle) { GetEngine().UnloadTexture(handle); }
	void ClearScene() { GetEngine().ClearScene(); }
	void SetObjectTexture(Object3D* object, const std::string& texturePath) { GetEngine().SetObjectTexture(object, texturePath); }

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
		const Transform& transform,
		const ModelDrawParameters& parameters
	) {
		GetEngine().DrawModel(model, transform, parameters);
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

	void DrawObject3D(
		Object3D* object,
		const Transform& transform,
		const ModelDrawParameters& parameters
	) {
		GetEngine().DrawObject3D(object, transform, parameters);
	}

	void DrawObject3D(Object3D* object) {
		GetEngine().DrawObject3D(object);
	}

	void DrawObject3D(Object3D* object, const ModelDrawParameters& parameters) {
		GetEngine().DrawObject3D(object, parameters);
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

	bool IsPushKey(Key key) {
		return GetEngine().IsPushKey(key);
	}

	bool IsTriggerKey(Key key) {
		return GetEngine().IsTriggerKey(key);
	}

	Vector2 GetMousePosition() {
		return GetEngine().GetMousePosition();
	}

	bool IsButtonDown(GamepadButton button) {
		return GetEngine().IsButtonDown(button);
	}

	bool IsButtonTriggered(GamepadButton button) {
		return GetEngine().IsButtonTriggered(button);
	}

	bool IsButtonReleased(GamepadButton button) {
		return GetEngine().IsButtonReleased(button);
	}

	bool IsGamepadConnected() {
		return GetEngine().IsGamepadConnected();
	}

	Vector2 GetLeftStick() {
		return GetEngine().GetLeftStick();
	}

	Vector2 GetRightStick() {
		return GetEngine().GetRightStick();
	}

	float GetLeftTrigger() {
		return GetEngine().GetLeftTrigger();
	}

	float GetRightTrigger() {
		return GetEngine().GetRightTrigger();
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

	DirectionalLight& GetDirectionalLight() {
		return GetEngine().GetDirectionalLight();
	}

	float GetDeltaTime()
	{
		return GetEngine().GetDeltaTime();
	}
}
