#include "RunaEngine.h"

int WINAPI WinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPSTR, _In_ int) {
	RunaEngine::Initialize(1280, 720, "TITLE");

	while (RunaEngine::ProcessMessage()) {
		RunaEngine::BeginFrame();

		RunaEngine::EndFrame();
	}

	RunaEngine::Shutdown();
	return 0;
}
