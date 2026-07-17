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

	while (RunaEngine::ProcessMessage()) {
		RunaEngine::BeginFrame();

		// 更新
		MoveCamera();
		ImGui::Begin("Settings");
		ImGui::DragFloat2("frontPos", &frontPos.x);
		ImGui::DragFloat2("backPos", &backPos.x);
		
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
	if (RunaEngine::IsPushKey(DIK_A)) {
		RunaEngine::MoveCamera(RunaEngine::Vector3{ -0.1f,0.0f,0.0f });
	} else if (RunaEngine::IsPushKey(DIK_D)) {
		RunaEngine::MoveCamera(RunaEngine::Vector3{ 0.1f,0.0f,0.0f });
	}

	if (RunaEngine::IsPushKey(DIK_SPACE)) {
		RunaEngine::MoveCamera(RunaEngine::Vector3{ 0.0f,0.1f,0.0f });
	} else if (RunaEngine::IsPushKey(DIK_LSHIFT)) {
		RunaEngine::MoveCamera(RunaEngine::Vector3{ 0.f,-0.1f,0.0f });
	}

	if (RunaEngine::IsPushKey(DIK_W)) {
		RunaEngine::MoveCamera(RunaEngine::Vector3{ 0.0f,0.0f,0.1f });
	} else if (RunaEngine::IsPushKey(DIK_S)) {
		RunaEngine::MoveCamera(RunaEngine::Vector3{ 0.0f,0.0f,-0.1f });
	}


}