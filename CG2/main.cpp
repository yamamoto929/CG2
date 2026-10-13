#include "RunaEngine/RunaEngine.h"
#include "RunaEngine/Math/AffineMatrix.h"


void MoveCamera();
using namespace RunaEngine;
int WINAPI WinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPSTR, _In_ int) {

	RunaEngine::Initialize(1280, 720, "TITLE");
	Sprite* sprite = CreateSprite("resources/sprite_test.png");
	Model* model = CreateModel("resources/plane.obj");
	float intensity = 0.0f;
	Transform transform = {
		{1.0f,1.0f,1.0f},
		{0.0f,3.14f,0.0f},
		{0.0f,0.0f,0.0f}
	};

	ModelDrawParameters param;
	param.lightingMode = LightingMode::HALF_LAMBERT;

	while (RunaEngine::ProcessMessage()) {
		RunaEngine::BeginFrame();
#ifdef USE_IMGUI
		ImGui::Begin("Model Parameters");
		ImGui::DragFloat4("Color", &param.color.x, 0.01f);
		ImGui::DragFloat("Intensity", &intensity, 0.01f);
		GetDirectionalLight().SetIntensity(intensity);
		ImGui::End();
#endif // USE_IMGUI
		// 更新
		MoveCamera();

		DrawModel(model, transform, param);
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
