#include "RunaEngine/RunaEngine.h"

void MoveCamera();

int WINAPI WinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPSTR, _In_ int) {
	RunaEngine::Initialize(1280, 720, "TITLE");

	RunaEngine::Sprite* front;
	RunaEngine::Vector2 frontPos = { 150.0f,150.0f };
	front = RunaEngine::CreateSprite("Resources/sprite_test.png");

	RunaEngine::Sprite* back;
	RunaEngine::Vector2 backPos = { 100.0f,100.0f };
	back = RunaEngine::CreateSprite("Resources/uvChecker.png");

	back->SetDrawOrder(0);
	front->SetDrawOrder(10);

	front->SetTextureRect(
		0.0f,
		0.0f,
		256.0f,
		256.0f
	);

	front->SetSize(256.0f, 256.0f);

	while (RunaEngine::ProcessMessage()) {
		RunaEngine::BeginFrame();

		// 更新
		MoveCamera();
		ImGui::Begin("Settings");
		ImGui::DragFloat2("frontPos", &frontPos.x);
		ImGui::DragFloat2("backPos", &backPos.x);
		RunaEngine::Transform cameraTransform=RunaEngine::GetCameraTransform();
		ImGui::DragFloat3("cameraPos", &cameraTransform.translate.x);

		if (RunaEngine::IsButtonDown(GamepadButton::A)) {
			ImGui::Text("A");
		}

		const float deltaTime = RunaEngine::GetDeltaTime();
		ImGui::Text("%f", deltaTime);
		
		ImGui::End();
		
		// 描画
		RunaEngine::DrawSprite(front, frontPos);
		RunaEngine::DrawSprite(back, backPos);
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