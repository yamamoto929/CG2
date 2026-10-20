#include "RunaEngine/RunaEngine.h"

using namespace RunaEngine;

int WINAPI WinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPSTR, _In_ int) {

	RunaEngine::Initialize(1280, 720, "TITLE");
	Model* model = CreateModel("resources/fence.obj");
	
	Transform pic1 = {
		{1.0f,1.0f,1.0f},
		{0.0f,0.0f,0.0f},
		{0.0f,0.0f,0.0f},
	};

	ModelDrawParameters param;
	param.blendMode = BlendMode::NORMAL;
	
	while (RunaEngine::ProcessMessage()) {
		RunaEngine::BeginFrame();
#ifdef USE_IMGUI
		ImGui::Begin("debug");
		ImGui::DragFloat3("pic1 scale", &pic1.scale.x, 0.01f);
		ImGui::DragFloat3("pic1 rotate", &pic1.rotate.x, 0.01f);
		ImGui::DragFloat3("pic1 translate", &pic1.translate.x, 0.01f);
		ImGui::End();
#endif // USE_IMGUI
		DrawModel(model, pic1,param);
		RunaEngine::EndFrame();
	}

	RunaEngine::Shutdown();
	return 0;

}

