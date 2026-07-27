#include "RunaEngine/RunaEngine.h"

void MoveCamera();

int WINAPI WinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPSTR, _In_ int) {
	RunaEngine::Initialize(1280, 720, "TITLE");
	RunaEngine::Model* model = RunaEngine::CreateModel("resources/suzanne.obj");

	DirectionalLight& directionalLight =
		RunaEngine::GetDirectionalLight();

	RunaEngine::Vector3 lightDirection =
		directionalLight.GetDirection();

	RunaEngine::Vector4 lightColor =
		directionalLight.GetColor();

	float lightIntensity =
		directionalLight.GetIntensity();
	RunaEngine::ModelDrawParameters modelParameters{};
	int lightingMode =
		static_cast<int>(LightingMode::HALF_LAMBERT);

	RunaEngine::ModelDrawParameters leftParameters{};
	leftParameters.lightingMode = LightingMode::LAMBERT;
	leftParameters.color = { 1.0f, 1.0f, 1.0f, 1.0f };

	RunaEngine::ModelDrawParameters rightParameters{};
	rightParameters.lightingMode = LightingMode::HALF_LAMBERT;
	rightParameters.color = { 1.0f, 1.0f, 1.0f, 1.0f };

	RunaEngine::Transform leftTransform{
		{1.0f, 1.0f, 1.0f},
		{0.0f, 3.1415f, 0.0f},
		{-2.0f, 0.0f, 0.0f}
	};

	RunaEngine::Transform rightTransform{
		{1.0f, 1.0f, 1.0f},
		{0.0f, 3.1415f, 0.0f},
		{2.0f, 0.0f, 0.0f}
	};
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

		const char* lightingItems[] = {
			"None",
			"Half Lambert",
			"Lambert"
		};

		if (ImGui::DragFloat3(
			"Light Direction",
			&lightDirection.x,
			0.01f)) {

			if (lightDirection.Length() > 0.0001f) {
				directionalLight.SetDirection(lightDirection);
			}
		}

		if (ImGui::ColorEdit4(
			"Light Color",
			&lightColor.x)) {

			directionalLight.SetColor(lightColor);
		}

		if (ImGui::SliderFloat(
			"Light Intensity",
			&lightIntensity,
			0.0f,
			10.0f)) {

			directionalLight.SetIntensity(lightIntensity);
		}

		ImGui::Combo(
			"Lighting Mode",
			&lightingMode,
			lightingItems,
			IM_ARRAYSIZE(lightingItems));

		leftParameters.lightingMode =
			static_cast<LightingMode>(lightingMode);

		ImGui::End();

		// 描画
		RunaEngine::DrawModel(model, leftTransform, leftParameters);
		RunaEngine::DrawModel(model, rightTransform, rightParameters);
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