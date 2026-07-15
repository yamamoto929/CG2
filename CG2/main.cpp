#include "RunaEngine/RunaEngine.h"

void MoveCamera();

int WINAPI WinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPSTR, _In_ int) {
	RunaEngine::Initialize(1280, 720, "TITLE");
	RunaEngine::Model* model[5];
	model[0] = RunaEngine::CreateModel("Resources/Plane.obj");
	model[1] = RunaEngine::CreateModel("Resources/Sphere.obj");
	model[2] = RunaEngine::CreateModel("Resources/teapot.obj");
	model[3] = RunaEngine::CreateModel("Resources/bunny.obj");
	// suzzane
	model[4] = RunaEngine::CreateModel("Resources/bunny.obj");
	RunaEngine::Vector3 scale{};
	RunaEngine::Vector3 rotate{};
	RunaEngine::Vector3 translate{};
	int currentModel = 0;
	const char* modelLabel[] = { "None","Plane","Sphere","Utah Teapot","Stanford Bunny","Suzanne"};

	int currentLightingMode = 0;
	const char* lightingLabel[] = {"None","Lambertian Reflectance","Half Lambert"};

	RunaEngine::Sprite* sprite;
	sprite = RunaEngine::CreateSprite("Resources/uvChecker.png");

	while (RunaEngine::ProcessMessage()) {
		RunaEngine::BeginFrame();

		// 更新
		MoveCamera();
		ImGui::Begin("Settings");

		if (ImGui::Combo("Model", &currentModel, modelLabel, 5)) {
			scale = { 1.0f,1.0f,1.0f };
			rotate = { 0.0f,0.0f,0.0f };
			translate = { 0.0f,0.0f,0.0f };
		}

		ImGui::DragFloat3("scale", &scale.x, 0.01f, 0.1f, 2.0f);
		ImGui::DragFloat3("rotate", &rotate.x, 0.01f);
		ImGui::DragFloat3("translate", &translate.x, 0.01f);
		if (ImGui::Combo("Lighting", &currentLightingMode, lightingLabel, 3)) {
			
		}

		ImGui::Text("%d", currentModel);
		ImGui::End();
		
		if (RunaEngine::IsPushKey(DIK_LEFT)) {
			rotate.y += 0.01f;
		} else if (RunaEngine::IsPushKey(DIK_RIGHT)) {
			rotate.y -= 0.01f;
		}

		// 描画
		if (currentModel != 0) {
			RunaEngine::DrawModel(model[currentModel - 1], scale.x, scale.y, scale.z, rotate.x, rotate.y, rotate.z, translate.x, translate.y, translate.z);
		}

		RunaEngine::DrawSprite(sprite, RunaEngine::Vector3{ 0.0f,0.0f,0.0f });
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