#include "RunaEngine/RunaEngine.h"
#include "RunaEngine/Math/AffineMatrix.h"


void MoveCamera();
using namespace RunaEngine;
int WINAPI WinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPSTR, _In_ int) {

	RunaEngine::Initialize(1280, 720, "TITLE");
	Sprite* sprite = CreateSprite("resources/sprite_test.png");
	if (!sprite) {
		RunaEngine::Shutdown();
		return 1;
	}
	while (RunaEngine::ProcessMessage()) {
		RunaEngine::BeginFrame();

		// 更新
		MoveCamera();

		sprite->SetColor({ 1.0f, 0.0f, 0.0f, 1.0f });
		DrawSprite(sprite, Vector2{ 40.0f,40.0f });
		sprite->SetColor({ 0.0f, 0.0f, 1.0f, 1.0f });
		DrawSprite(sprite, Vector2{ 552.0f,40.0f });
		RunaEngine::EndFrame();
	}
	RunaEngine::Shutdown();
	return 0;

}

void MoveCamera() {
	if (RunaEngine::IsPushKey(Key::A)) {
		RunaEngine::MoveCamera(RunaEngine::Vector3{ -0.1f,0.0f,0.0f });
	} else if (RunaEngine::IsPushKey(Key::D)) {
		RunaEngine::MoveCamera(RunaEngine::Vector3{ 0.1f,0.0f,0.0f });
	}

	if (RunaEngine::IsPushKey(Key::Space)) {
		RunaEngine::MoveCamera(RunaEngine::Vector3{ 0.0f,0.1f,0.0f });
	} else if (RunaEngine::IsPushKey(Key::LeftShift)) {
		RunaEngine::MoveCamera(RunaEngine::Vector3{ 0.0f,-0.1f,0.0f });
	}

	if (RunaEngine::IsPushKey(Key::W)) {
		RunaEngine::MoveCamera(RunaEngine::Vector3{ 0.0f,0.0f,0.1f });
	} else if (RunaEngine::IsPushKey(Key::S)) {
		RunaEngine::MoveCamera(RunaEngine::Vector3{ 0.0f,0.0f,-0.1f });
	}
}
