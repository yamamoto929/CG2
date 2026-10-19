#include "RunaEngine/RunaEngine.h"
#include "Player.h"

using namespace RunaEngine;

int WINAPI WinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPSTR, _In_ int) {

	RunaEngine::Initialize(1280, 720, "TITLE");
	Player* player = new Player();
	player->Initialize();
	while (RunaEngine::ProcessMessage()) {
		RunaEngine::BeginFrame();
		player->Update(GetDeltaTime());
		player->Draw();
		RunaEngine::EndFrame();
	}
	delete player;
	RunaEngine::Shutdown();
	return 0;

}

