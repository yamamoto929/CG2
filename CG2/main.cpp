#include "RunaEngine/RunaEngine.h"

int WINAPI WinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPSTR, _In_ int) {
	RunaEngine::Initialize(1280, 720, "TITLE");
	RunaEngine::Model* model = RunaEngine::CreateModel("Resources/triangle.obj");

	while (RunaEngine::ProcessMessage()) {
		RunaEngine::BeginFrame();
		ImGui::Begin("debug");
		if (RunaEngine::IsPushKey(DIK_A)) {
			ImGui::Text("true");
		} else {
			ImGui::Text("false");
		}
		ImGui::End();

		RunaEngine::DrawModel(model, 1.0f, 1.0f, 1.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f, 0.0f);
		RunaEngine::EndFrame();
	}

	RunaEngine::Shutdown();
	return 0;
}
