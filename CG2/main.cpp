#include "RunaEngine/RunaEngine.h"
#include "RunaEngine/Math/AffineMatrix.h"

using namespace RunaEngine;
int WINAPI WinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPSTR, _In_ int) {

	RunaEngine::Initialize(1280, 720, "TITLE");
	Model* model = CreateModel("resources/plane.obj");

	Transform pic1 = {
		{1.0f,1.0f,1.0f},
		{0.0f,0.0f,0.0f},
		{0.0f,0.0f,0.0f},
	};

	ModelDrawParameters param;
	param.blendMode = BlendMode::NONE;
	param.lightingMode = LightingMode::NONE;
	auto& engine = RunaEngine::GetEngine();
	auto* instancingData = engine.GetInstancingData();
	while (RunaEngine::ProcessMessage()) {
		RunaEngine::BeginFrame();
		const auto viewProjection = engine.GetViewProjectionMatrix();
		
		for (int i = 0; i < 10; ++i) {
			Vector3 translate{
			0.1f * float(i),
			0.1f * float(i),
			0.0f,
			};
			const auto world = MakeAffineMatrix(pic1.scale, pic1.rotate, pic1.translate + translate);
			instancingData[i].World = world;
			instancingData[i].WVP = Multiply(world, viewProjection);
		}
#ifdef USE_IMGUI
		ImGui::Begin("debug");
		ImGui::DragFloat3("pic1 scale", &pic1.scale.x, 0.01f);
		ImGui::DragFloat3("pic1 rotate", &pic1.rotate.x, 0.01f);
		ImGui::DragFloat3("pic1 translate", &pic1.translate.x, 0.01f);
		ImGui::End();
#endif // USE_IMGUI
		if (model) {
			auto* commandList = engine.GetRenderer().GetCommandList();
			engine.SetParticlePipeline();
			commandList->SetGraphicsRootDescriptorTable(
				1, engine.GetInstancingSrvGpuHandle());
			model->Draw(commandList, &engine.GetTextureManager(), param, 10);
		}
		RunaEngine::EndFrame();
	}

	RunaEngine::Shutdown();
	return 0;

}
