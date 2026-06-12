#include "RunaEngine.h"
#include "TetrahedronEffect.h"

int WINAPI WinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPSTR, _In_ int) {
	RunaEngine::Initialize(1280, 720, "CG2");

	RunaEngine::Primitive3D* triangle = RunaEngine::CreateTriangle3D();
	RunaEngine::Vector3 scale = { 1.0f, 1.0f, 1.0f };
	RunaEngine::Vector3 rotate = { 0.0f, -0.2f, 0.0f };
	RunaEngine::Vector3 translate = { 0.0f, 0.0f, 10.0f };
	RunaEngine::Vector4 color = { 1.0f, 0.2f, 0.1f, 1.0f };

	RunaEngine::Model* modelTriangle = RunaEngine::CreateModel("./Resources/triangle.obj");
	RunaEngine::Vector3 modelScale = { 1.0f, 1.3f, 1.0f };
	RunaEngine::Vector3 modelRotate = { 0.0f, 0.2f, 0.0f };
	RunaEngine::Vector3 modelTranslate = { 0.0f, 0.0f, 10.0f };
	modelTriangle->SetEnableLighting(false);

	TetrahedronEffect tetrahedronEffect;
	tetrahedronEffect.Initialize();

	const char* modes[] = { "Triangle Demo", "Tetrahedron Effect" };
	int currentMode = 0;
	const char* textures[] = { "uvChecker", "MonsterBall" };
	int currentTexture = 0;

	while (RunaEngine::ProcessMessage()) {
		RunaEngine::BeginFrame();

#ifdef USE_IMGUI
		ImGui::Begin("Debug");

		int previousMode = currentMode;
		ImGui::Combo("Mode", &currentMode, modes, IM_ARRAYSIZE(modes));
		if (previousMode != currentMode && currentMode == 1) {
			tetrahedronEffect.Reset();
		}

		if (currentMode == 0) {
			ImGui::DragFloat3("scale", &scale.x, 0.01f);
			ImGui::DragFloat3("rotate", &rotate.x, 0.01f);
			ImGui::DragFloat3("translate", &translate.x, 0.01f);
			ImGui::ColorEdit4("color", &color.x);

			if (ImGui::Combo("Texture", &currentTexture, textures, IM_ARRAYSIZE(textures))) {
				if (currentTexture == 0) {
					RunaEngine::SetModelTexture(modelTriangle, "./resources/uvChecker.png");
				} else if (currentTexture == 1) {
					RunaEngine::SetModelTexture(modelTriangle, "./resources/monsterBall.png");
				}
			}

			ImGui::DragFloat3("modelScale", &modelScale.x, 0.01f);
			ImGui::DragFloat3("modelRotate", &modelRotate.x, 0.01f);
			ImGui::DragFloat3("modelTranslate", &modelTranslate.x, 0.01f);
		} else {
			if (ImGui::Button("Reset Effect")) {
				tetrahedronEffect.Reset();
			}
		}

		ImGui::End();
#endif

		if (currentMode == 0) {
			triangle->SetColor(color);
			RunaEngine::DrawPrimitive3D(
				triangle,
				scale.x, scale.y, scale.z,
				rotate.x, rotate.y, rotate.z,
				translate.x, translate.y, translate.z
			);

			RunaEngine::DrawModel(
				modelTriangle,
				modelScale.x, modelScale.y, modelScale.z,
				modelRotate.x, modelRotate.y, modelRotate.z,
				modelTranslate.x, modelTranslate.y, modelTranslate.z
			);
		} else {
			tetrahedronEffect.Update();
			tetrahedronEffect.Draw();
		}

		RunaEngine::EndFrame();
	}

	RunaEngine::Shutdown();
	return 0;
}
