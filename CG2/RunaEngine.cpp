#include "RunaEngine.h"
#include "AffineMatrix.h"
#include "CrashHandler.h"
#include "DirectXDebug.h"
#include "Log.h"
#include "WVPMatrix.h"
#include <cassert>
#include <filesystem>

RunaEngine::RunaEngine() = default;

RunaEngine::~RunaEngine() {
	Shutdown();
}

void RunaEngine::Initialize(int32_t width, int32_t height, const std::string& title) {
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
	graphicsPipeline_.Initialize(
		directXCommon_.GetDevice(),
		&shaderCompiler_,
		directXCommon_.GetDXGIFormat(),
		DXGI_FORMAT_D24_UNORM_S8_UINT
	);

	directionalLight_.Initialize(directXCommon_.GetDevice());
	renderer_.Initialize(&directXCommon_, &textureManager_, &graphicsPipeline_, &directionalLight_);
	imGuiManager_.Initialize(winApp_, directXCommon_, textureManager_);

	UpdateCameraMatrices();
	initialized_ = true;
}

bool RunaEngine::ProcessMessage() {
	while (PeekMessage(&msg_, nullptr, 0, 0, PM_REMOVE)) {
		if (msg_.message == WM_QUIT) {
			return false;
		}
		TranslateMessage(&msg_);
		DispatchMessage(&msg_);
	}
	return true;
}

void RunaEngine::BeginFrame() {
	assert(initialized_);

	imGuiManager_.BeginFrame();
	input_.Update();
	soundManager_.Update();
	UpdateCameraMatrices();
	renderer_.Begin();
}

void RunaEngine::EndFrame() {
	assert(initialized_);

	imGuiManager_.Render(renderer_.GetCommandList());
	renderer_.End();
}

void RunaEngine::Shutdown() {
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

Sprite* RunaEngine::CreateSprite(const std::string& texturePath) {
	assert(initialized_);

	std::unique_ptr<Sprite> sprite = std::make_unique<Sprite>();
	sprite->Initialize(directXCommon_.GetDevice(), &textureManager_, texturePath);

	Sprite* result = sprite.get();
	sprites_.push_back(std::move(sprite));
	return result;
}

Model* RunaEngine::CreateModel(const std::string& directoryPath) {
	assert(initialized_);
	std::filesystem::path path(directoryPath);

	std::string filePath = path.parent_path().generic_string();
	std::string fileName = path.filename().generic_string();
	return resourceManager_.LoadModel(directXCommon_.GetDevice(), &textureManager_, filePath, fileName);
}

Object3D* RunaEngine::CreateObject3D(Model* model) {
	assert(initialized_);
	assert(model);

	std::unique_ptr<Object3D> object = std::make_unique<Object3D>();
	object->Initialize(directXCommon_.GetDevice(), model);

	Object3D* result = object.get();
	object3Ds_.push_back(std::move(object));
	return result;
}

void RunaEngine::DrawSprite(Sprite* sprite, const Vector2& position, float z) {
	DrawSprite(sprite, Vector3{ position.x, position.y, z });
}

void RunaEngine::DrawSprite(Sprite* sprite, const Vector3& translate) {
	Transform transform{
		{ 1.0f, 1.0f, 1.0f },
		{ 0.0f, 0.0f, 0.0f },
		translate,
	};
	DrawSprite(sprite, transform);
}

void RunaEngine::DrawSprite(Sprite* sprite, const Transform& transform) {
	assert(sprite);

	sprite->GetTransform() = transform;
	sprite->Update(width_, height_);
	renderer_.Draw(*sprite);
}

void RunaEngine::DrawModel(Model* model, const Vector3& translate) {
	Transform transform{
		{ 1.0f, 1.0f, 1.0f },
		{ 0.0f, 0.0f, 0.0f },
		translate,
	};
	DrawModel(model, transform);
}

void RunaEngine::DrawModel(Model* model, const Transform& transform) {
	DrawObject3D(GetDefaultObject(model), transform);
}

void RunaEngine::DrawObject3D(Object3D* object, const Vector3& translate) {
	Transform transform{
		{ 1.0f, 1.0f, 1.0f },
		{ 0.0f, 0.0f, 0.0f },
		translate,
	};
	DrawObject3D(object, transform);
}

void RunaEngine::DrawObject3D(Object3D* object, const Transform& transform) {
	assert(object);

	object->GetTransform() = transform;
	DrawObject3D(object);
}

void RunaEngine::DrawObject3D(Object3D* object) {
	assert(object);

	object->Update(viewMatrix_, projectionMatrix_);
	renderer_.Draw(*object);
}

uint32_t RunaEngine::LoadTexture(const std::string& texturePath) {
	assert(initialized_);
	return textureManager_.Load(texturePath);
}

void RunaEngine::SetModelTexture(Model* model, const std::string& texturePath) {
	assert(model);
	model->SetTextureHandle(LoadTexture(texturePath));
}

void RunaEngine::SetSpriteTexture(Sprite* sprite, const std::string& texturePath) {
	assert(sprite);
	sprite->SetTextureHandle(LoadTexture(texturePath));
}

uint32_t RunaEngine::LoadSound(const std::wstring& filePath) {
	assert(initialized_);
	return soundManager_.Load(filePath);
}

uint32_t RunaEngine::LoadSound(const std::string& filePath) {
	assert(initialized_);
	return soundManager_.Load(filePath);
}

void RunaEngine::PlaySound(uint32_t soundHandle, bool loop, float volume) {
	assert(initialized_);
	soundManager_.Play(soundHandle, loop, volume);
}

const BYTE* RunaEngine::GetKey() const {
	return input_.GetKey();
}

bool RunaEngine::IsPushKey(uint8_t key) {
	return input_.IsPushkey(key);
}

void RunaEngine::SetCameraTransform(const Transform& transform) {
	cameraTransform_ = transform;
	UpdateCameraMatrices();
}

void RunaEngine::MoveCamera(const Vector3& move) {
	cameraTransform_.translate += move;
	UpdateCameraMatrices();
}

Object3D* RunaEngine::GetDefaultObject(Model* model) {
	assert(model);

	auto it = defaultObjects_.find(model);
	if (it != defaultObjects_.end()) {
		return it->second;
	}

	Object3D* object = CreateObject3D(model);
	defaultObjects_[model] = object;
	return object;
}

void RunaEngine::UpdateCameraMatrices() {
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
