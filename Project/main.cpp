#include "RunaEngine/RunaEngine.h"
int WINAPI WinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPSTR, _In_ int) {
	RunaEngine::Initialize(1280, 720, "LE2B_30_ヤマモト_ルナ");
	while (RunaEngine::ProcessMessage()) {
		RunaEngine::BeginFrame();
#ifdef USE_IMGUI
		ImGui::Begin("ImGui");
		ImGui::End();
#endif
		RunaEngine::EndFrame();
	}
	RunaEngine::Shutdown();
	return 0;
}
