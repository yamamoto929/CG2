#include "RunaEngine.h"

int WINAPI WinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPSTR, _In_ int) {
	RunaEngine::Initialize(1280, 720, "CG2");

	RunaEngine::Model* model = RunaEngine::CreateModel("./resources/axis.obj");

	while (RunaEngine::ProcessMessage()) {
		RunaEngine::BeginFrame();

#ifdef USE_IMGUI
		ImGui::Begin("Debug");
		RunaEngine::Transform& cameraTransform = RunaEngine::GetCameraTransform();
		bool cameraChanged = false;
		cameraChanged |= ImGui::DragFloat3("CameraTranslate", &cameraTransform.translate.x, 0.01f);
		cameraChanged |= ImGui::DragFloat3("CameraRotate", &cameraTransform.rotate.x, 0.01f);
		if (cameraChanged) {
			RunaEngine::SetCameraTransform(cameraTransform);
		}
		if (ImGui::Button("Model Texture: UV")) {
			RunaEngine::SetModelTexture(model, "./resources/uvChecker.png");
		}
		if (ImGui::Button("Model Texture: MonsterBall")) {
			RunaEngine::SetModelTexture(model, "./resources/monsterBall.png");
		}
		ImGui::End();
#endif

		RunaEngine::DrawModel(model, RunaEngine::Vector3{ 0.0f, 0.0f, 0.0f });

		RunaEngine::EndFrame();
	}

	RunaEngine::Shutdown();
	return 0;
}
