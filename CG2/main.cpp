#include "RunaEngine/RunaEngine.h"
#include "RunaEngine/Math/AffineMatrix.h"

void MoveCamera();

enum class ModelList {
	NONE,
	PLANE,
	SPHERE,
	TEAPOT,
	BUNNY,
	SUZANNE,
	MULTIMESH
};

int WINAPI WinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPSTR, _In_ int) {
	RunaEngine::Initialize(1280, 720, "TITLE");
	RunaEngine::Transform initTransform{
		{1.0f, 1.0f, 1.0f},
		{0.0f, 3.1415f, 0.0f},
		{0.0f, 0.0f, 0.0f}
	};
	static RunaEngine::Model* model[7];
	model[static_cast<int>(ModelList::NONE)] = CreateModel("resources/plane.obj"); // 使わないけど生成
	model[static_cast<int>(ModelList::PLANE)] = CreateModel("resources/plane.obj");
	model[static_cast<int>(ModelList::SPHERE)] = CreateModel("resources/Sphere.obj");
	model[static_cast<int>(ModelList::TEAPOT)] = CreateModel("resources/teapot.obj");
	model[static_cast<int>(ModelList::BUNNY)] = CreateModel("resources/bunny.obj");
	model[static_cast<int>(ModelList::SUZANNE)] = CreateModel("resources/suzanne.obj");
	model[static_cast<int>(ModelList::MULTIMESH)] = CreateModel("resources/multiMesh.obj");

	bool drawSprite = true;
	RunaEngine::Sprite* sprite = RunaEngine::CreateSprite("resources/uvChecker.png");
	RunaEngine::Transform spriteTransform = {
		{1.0f,1.0f,1.0f},
		{0.0f,0.0f,0.0f},
		{0.0f,0.0f,0.0f}
	};

	const uint32_t testSound =
		RunaEngine::LoadSound("resources/Alarm01.wav");
	float soundVolume = 1.0f;

	//=========================================================
	// 左
	//=========================================================
	int currentLeftModel = static_cast<int>(ModelList::SPHERE);
	DirectionalLight& directionalLight =
		RunaEngine::GetDirectionalLight();

	RunaEngine::Vector3 lightDirection =
		directionalLight.GetDirection();

	RunaEngine::Vector4 lightColor =
		directionalLight.GetColor();

	float lightIntensity =
		directionalLight.GetIntensity();
	int leftLightingMode =
		static_cast<int>(LightingMode::HALF_LAMBERT);
	RunaEngine::ModelDrawParameters leftParameters{};
	leftParameters.lightingMode = LightingMode::HALF_LAMBERT;
	leftParameters.color = { 1.0f, 1.0f, 1.0f, 1.0f };
	RunaEngine::Transform leftUVTransform{
		{ 1.0f, 1.0f, 1.0f },
		{ 0.0f, 0.0f, 0.0f },
		{ 0.0f, 0.0f, 0.0f },
	};

	RunaEngine::Transform leftTransform = initTransform;

	//=========================================================
	// 右
	//=========================================================
	int currentRightModel = static_cast<int>(ModelList::NONE);
	int rightLightingMode =
		static_cast<int>(LightingMode::HALF_LAMBERT);
	RunaEngine::ModelDrawParameters rightParameters{};
	rightParameters.lightingMode = LightingMode::HALF_LAMBERT;
	rightParameters.color = { 1.0f, 1.0f, 1.0f, 1.0f };
	RunaEngine::Transform rightUVTransform{
		{ 1.0f, 1.0f, 1.0f },
		{ 0.0f, 0.0f, 0.0f },
		{ 0.0f, 0.0f, 0.0f },
	};

	RunaEngine::Transform rightTransform = initTransform;

	//=========================================================
	// ImGui用
	//=========================================================
	const char* modelItems[] = {
				"None",
				"Plane",
				"Sphere",
				"Utah Teapot",
				"Stanford Bunny",
				"Suzzane",
				"Multi Mesh Model",
	};

	const char* lightingItems[] = {
				"None",
				"Half Lambert",
				"Lambert"
	};

	struct GamepadButtonDisplay {
		const char* name;
		GamepadButton button;
	};

	constexpr GamepadButtonDisplay gamepadButtons[] = {
		{ "A", GamepadButton::A },
		{ "B", GamepadButton::B },
		{ "X", GamepadButton::X },
		{ "Y", GamepadButton::Y },
		{ "DPad Up", GamepadButton::DPadUp },
		{ "DPad Down", GamepadButton::DPadDown },
		{ "DPad Left", GamepadButton::DPadLeft },
		{ "DPad Right", GamepadButton::DPadRight },
		{ "Left Shoulder", GamepadButton::LeftShoulder },
		{ "Right Shoulder", GamepadButton::RightShoulder },
		{ "Left Stick", GamepadButton::LeftStick },
		{ "Right Stick", GamepadButton::RightStick },
		{ "Start", GamepadButton::Start },
		{ "Back", GamepadButton::Back },
	};

	while (RunaEngine::ProcessMessage()) {
		RunaEngine::BeginFrame();

		// 更新
		MoveCamera();
		bool shouldPlayTestSound =
			RunaEngine::IsTriggerKey(Key::P) ||
			RunaEngine::IsButtonTriggered(GamepadButton::A);
#ifdef USE_IMGUI
		ImGui::Begin("Settings");
		
		RunaEngine::Transform cameraTransform = RunaEngine::GetCameraTransform();
		ImGui::DragFloat3("cameraPos", &cameraTransform.translate.x);
		if (ImGui::TreeNode("light")) {
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

			ImGui::TreePop();
		}

		if (ImGui::TreeNode("mouse")) {
			const RunaEngine::Vector2 mousePosition =
				RunaEngine::GetMousePosition();

			ImGui::Text(
				"Position X: %.0f  Y: %.0f",
				mousePosition.x,
				mousePosition.y
			);

			ImGui::TreePop();
		}

		if (ImGui::TreeNode("controller")) {
			const bool isConnected = RunaEngine::IsGamepadConnected();
			const ImVec4 statusColor = isConnected
				? ImVec4{ 0.2f, 1.0f, 0.2f, 1.0f }
				: ImVec4{ 1.0f, 0.2f, 0.2f, 1.0f };

			ImGui::TextColored(
				statusColor,
				"Status: %s",
				isConnected ? "Connected" : "Disconnected"
			);

			if (isConnected) {
				const RunaEngine::Vector2 leftStick =
					RunaEngine::GetLeftStick();
				const RunaEngine::Vector2 rightStick =
					RunaEngine::GetRightStick();
				const float leftTrigger =
					RunaEngine::GetLeftTrigger();
				const float rightTrigger =
					RunaEngine::GetRightTrigger();

				ImGui::SeparatorText("Analog Input");
				ImGui::Text(
					"Left Stick  X: %.3f  Y: %.3f",
					leftStick.x,
					leftStick.y
				);
				ImGui::Text(
					"Right Stick X: %.3f  Y: %.3f",
					rightStick.x,
					rightStick.y
				);

				ImGui::Text("Left Trigger");
				ImGui::SameLine();
				ImGui::ProgressBar(leftTrigger, ImVec2{ 160.0f, 0.0f });

				ImGui::Text("Right Trigger");
				ImGui::SameLine();
				ImGui::ProgressBar(rightTrigger, ImVec2{ 160.0f, 0.0f });

				ImGui::SeparatorText("Buttons");
				if (ImGui::BeginTable(
					"ControllerButtons",
					4,
					ImGuiTableFlags_Borders |
					ImGuiTableFlags_RowBg |
					ImGuiTableFlags_SizingStretchProp
				)) {
					ImGui::TableSetupColumn("Button");
					ImGui::TableSetupColumn("Down");
					ImGui::TableSetupColumn("Triggered");
					ImGui::TableSetupColumn("Released");
					ImGui::TableHeadersRow();

					for (const GamepadButtonDisplay& display : gamepadButtons) {
						ImGui::TableNextRow();

						ImGui::TableSetColumnIndex(0);
						ImGui::TextUnformatted(display.name);

						ImGui::TableSetColumnIndex(1);
						ImGui::TextUnformatted(
							RunaEngine::IsButtonDown(display.button) ? "Yes" : "-"
						);

						ImGui::TableSetColumnIndex(2);
						ImGui::TextUnformatted(
							RunaEngine::IsButtonTriggered(display.button) ? "Yes" : "-"
						);

						ImGui::TableSetColumnIndex(3);
						ImGui::TextUnformatted(
							RunaEngine::IsButtonReleased(display.button) ? "Yes" : "-"
						);
					}

					ImGui::EndTable();
				}
			}

			ImGui::TreePop();
		}

		if (ImGui::TreeNode("sound")) {
			ImGui::TextUnformatted("File: resources/Alarm01.wav");
			ImGui::TextUnformatted("Shortcut: P key / Gamepad A button");
			ImGui::SliderFloat(
				"Volume",
				&soundVolume,
				0.0f,
				1.0f
			);

			if (ImGui::Button("Play Sound")) {
				shouldPlayTestSound = true;
			}

			ImGui::TreePop();
		}

		if (ImGui::TreeNode("sprite")) {
			ImGui::Checkbox("Draw", &drawSprite);
			ImGui::DragFloat2("translate", &spriteTransform.translate.x, 1.0f);
			ImGui::DragFloat("rotate", &spriteTransform.rotate.z, 0.01f);
			ImGui::DragFloat2("scale", &spriteTransform.scale.x, 0.01f);
			ImGui::TreePop();
		}
		//=========================================================
		// 左
		//=========================================================
		if (ImGui::TreeNode("model1")) {

			if (ImGui::Combo(
				"Model1",
				&currentLeftModel,
				modelItems,
				IM_ARRAYSIZE(modelItems))) {
				leftTransform = initTransform;
			}
			ImGui::DragFloat3("translate", &leftTransform.translate.x, 0.01f);
			ImGui::DragFloat3("rotate", &leftTransform.rotate.x, 0.01f);
			ImGui::DragFloat3("scale", &leftTransform.scale.x, 0.01f);			
			if (ImGui::TreeNode("UV Transform")) {
				ImGui::DragFloat2(
					"UV Translate",
					&leftUVTransform.translate.x,
					0.01f
				);
				ImGui::DragFloat(
					"UV Rotate",
					&leftUVTransform.rotate.z,
					0.01f
				);
				ImGui::DragFloat2(
					"UV Scale",
					&leftUVTransform.scale.x,
					0.01f
				);
				ImGui::TreePop();
			}
			ImGui::Combo(
				"Lighting Mode",
				&leftLightingMode,
				lightingItems,
				IM_ARRAYSIZE(lightingItems));

			leftParameters.lightingMode =
				static_cast<LightingMode>(leftLightingMode);
			ImGui::Text("Mesh Count: %zu", model[currentLeftModel]->GetMeshCount());
			ImGui::Text("SubMesh Count: %zu", model[currentLeftModel]->GetSubMeshCount());
			ImGui::Text("Material Count: %zu", model[currentLeftModel]->GetMaterialCount());
			ImGui::TreePop();
		}

		//=========================================================
		// 右
		//=========================================================
		if (ImGui::TreeNode("model2")) {

			if (ImGui::Combo(
				"Model2",
				&currentRightModel,
				modelItems,
				IM_ARRAYSIZE(modelItems))) {
				rightTransform = initTransform;
			}
			ImGui::DragFloat2("translate", &rightTransform.translate.x, 0.01f);
			ImGui::DragFloat("rotate", &rightTransform.rotate.z, 0.01f);
			ImGui::DragFloat2("scale", &rightTransform.scale.x, 0.01f);
			if (ImGui::TreeNode("UV Transform")) {
				ImGui::DragFloat2(
					"UV Translate",
					&rightUVTransform.translate.x,
					0.01f
				);
				ImGui::DragFloat(
					"UV Rotate",
					&rightUVTransform.rotate.z,
					0.01f
				);
				ImGui::DragFloat2(
					"UV Scale",
					&rightUVTransform.scale.x,
					0.01f
				);
				ImGui::TreePop();
			}
			ImGui::Combo(
				"Lighting Mode",
				&rightLightingMode,
				lightingItems,
				IM_ARRAYSIZE(lightingItems));
			

			rightParameters.lightingMode =
				static_cast<LightingMode>(rightLightingMode);
			ImGui::Text("Mesh Count: %zu", model[currentRightModel]->GetMeshCount());
			ImGui::Text("SubMesh Count: %zu", model[currentRightModel]->GetSubMeshCount());
			ImGui::Text("Material Count: %zu", model[currentRightModel]->GetMaterialCount());
			ImGui::TreePop();
		}

		ImGui::End();
#endif
		if (shouldPlayTestSound) {
			RunaEngine::PlaySound(testSound, false, soundVolume);
		}

		leftParameters.uvTransform = RunaEngine::MakeAffineMatrix(
			leftUVTransform.scale,
			leftUVTransform.rotate,
			leftUVTransform.translate
		);
		rightParameters.uvTransform = RunaEngine::MakeAffineMatrix(
			rightUVTransform.scale,
			rightUVTransform.rotate,
			rightUVTransform.translate
		);

		// 描画
		if (currentLeftModel != static_cast<int>(ModelList::NONE)) {
			RunaEngine::DrawModel(model[static_cast<int>(currentLeftModel)], leftTransform, leftParameters);
		}

		if (currentRightModel != static_cast<int>(ModelList::NONE)) {
			RunaEngine::DrawModel(model[static_cast<int>(currentRightModel)], rightTransform, rightParameters);
		}

		if (drawSprite) {
			RunaEngine::DrawSprite(sprite, spriteTransform);
		}

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
