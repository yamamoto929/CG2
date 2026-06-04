#include "RunaEngine.h"

int WINAPI WinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPSTR, _In_ int) {
	RunaEngine::Initialize(1280, 720, "CG2");

	RunaEngine::Primitive3D* triangle = RunaEngine::CreateTriangle3D();
	RunaEngine::Vector3 scale = { 1.0f,1.0f,1.0f };
	RunaEngine::Vector3 rotate = { 0.0f, -0.2f, 0.0f };
	RunaEngine::Vector3 translate = { 0.0f, 0.0f, 10.0f };
	RunaEngine::Vector4 color = { 1.0f, 0.2f, 0.1f, 1.0f };

	RunaEngine::Model* triangle2 = RunaEngine::CreateModel("./Resources/triangle.obj");
	RunaEngine::Vector3 scale2 = { 1.0f,1.3f,1.0f };
	RunaEngine::Vector3 rotate2 = { 0.0f, 0.2f, 0.0f };
	RunaEngine::Vector3 translate2 = { 0.0f, 0.0f, 10.0f };
	triangle2->SetEnableLighting(false);

	while (RunaEngine::ProcessMessage()) {
		RunaEngine::BeginFrame();

#ifdef USE_IMGUI
		ImGui::Begin("Debug");
		ImGui::DragFloat3("scale", &scale.x, 0.01f);
		ImGui::DragFloat3("rotate", &rotate.x, 0.01f);
		ImGui::DragFloat3("translate", &translate.x, 0.01f);
		ImGui::ColorEdit4("color", &color.x);

		ImGui::DragFloat3("scale2", &scale2.x, 0.01f);
		ImGui::DragFloat3("rotate2", &rotate2.x, 0.01f);
		ImGui::DragFloat3("translate2", &translate2.x, 0.01f);
		ImGui::End();
#endif

		triangle->SetColor(color);
		RunaEngine::DrawPrimitive3D(
			triangle,
			scale.x, scale.y, scale.z,
			rotate.x, rotate.y, rotate.z,
			translate.x, translate.y, translate.z
		);

		RunaEngine::DrawModel(
			triangle2,
			scale2.x, scale2.y, scale2.z,
			rotate2.x, rotate2.y, rotate2.z,
			translate2.x, translate2.y, translate2.z);

		RunaEngine::EndFrame();
	}

	RunaEngine::Shutdown();
	return 0;
}
