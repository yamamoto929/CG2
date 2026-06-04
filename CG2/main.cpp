#include "RunaEngine.h"

int WINAPI WinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPSTR, _In_ int) {
	RunaEngine engine;
	engine.Initialize(1280, 720, "CG2");

	Model* model = engine.CreateModel("resources", "axis.obj");
	Sprite* sprite = engine.CreateSprite("./resources/uvChecker.png");

	uint32_t alarmSound = engine.LoadSound(L"Resources/Alarm01.wav");
	engine.PlaySound(alarmSound);

	while (engine.ProcessMessage()) {
		engine.BeginFrame();

		const float cameraSpeed = 0.1f;
		if (engine.IsPushKey(DIK_W)) {
			engine.MoveCamera({ 0.0f, 0.0f, cameraSpeed });
		}
		if (engine.IsPushKey(DIK_S)) {
			engine.MoveCamera({ 0.0f, 0.0f, -cameraSpeed });
		}
		if (engine.IsPushKey(DIK_A)) {
			engine.MoveCamera({ -cameraSpeed, 0.0f, 0.0f });
		}
		if (engine.IsPushKey(DIK_D)) {
			engine.MoveCamera({ cameraSpeed, 0.0f, 0.0f });
		}

#ifdef USE_IMGUI
		ImGui::Begin("Debug");
		Transform& cameraTransform = engine.GetCameraTransform();
		bool cameraChanged = false;
		cameraChanged |= ImGui::DragFloat3("CameraTranslate", &cameraTransform.translate.x, 0.01f);
		cameraChanged |= ImGui::DragFloat3("CameraRotate", &cameraTransform.rotate.x, 0.01f);
		if (cameraChanged) {
			engine.SetCameraTransform(cameraTransform);
		}
		if (ImGui::Button("Play Alarm")) {
			engine.PlaySound(alarmSound);
		}
		if (ImGui::Button("Model Texture: UV")) {
			engine.SetModelTexture(model, "./resources/uvChecker.png");
		}
		if (ImGui::Button("Model Texture: MonsterBall")) {
			engine.SetModelTexture(model, "./resources/monsterBall.png");
		}
		ImGui::End();
#endif

		engine.DrawModel(model, Vector3{ 0.0f, 0.0f, 0.0f });
		engine.DrawSprite(sprite, Vector2{ 0.0f, 0.0f });

		engine.EndFrame();
	}

	engine.Shutdown();
	return 0;
}
