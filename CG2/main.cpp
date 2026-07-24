#include "RunaEngine/RunaEngine.h"

void MoveCamera();

int WINAPI WinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPSTR, _In_ int) {
	RunaEngine::Initialize(1280, 720, "TITLE");
	RunaEngine::Model* model = RunaEngine::CreateModel("resources/suzanne.obj");


	while (RunaEngine::ProcessMessage()) {
		RunaEngine::BeginFrame();

		// 更新
		MoveCamera();
		ImGui::Begin("Settings");

		RunaEngine::Transform cameraTransform = RunaEngine::GetCameraTransform();
		ImGui::DragFloat3("cameraPos", &cameraTransform.translate.x);

		if (RunaEngine::IsButtonDown(GamepadButton::A)) {
			ImGui::Text("A");
		}

		const float deltaTime = RunaEngine::GetDeltaTime();
		ImGui::Text("%f", deltaTime);

		ImGui::End();

		// 描画
		RunaEngine::DrawModel(model, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
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