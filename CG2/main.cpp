#include "RunaEngine/RunaEngine.h"

using namespace RunaEngine;

int WINAPI WinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPSTR, _In_ int) {

	RunaEngine::Initialize(1280, 720, "TITLE");
	Sprite* sprite = CreateSprite("resources/sprite_test.png");
	Vector2 position{ 40.0f, 40.0f };
	const float moveSpeed = 300.0f;

	while (RunaEngine::ProcessMessage()) {
		RunaEngine::BeginFrame();

		const float move = moveSpeed * RunaEngine::GetDeltaTime();
		if (IsPushKey(Key::D)) {
			position.x += move;
		} else if (IsPushKey(Key::A)) {
			position.x -= move;
		}
		
		if (IsPushKey(Key::S)) {
			position.y += move;
		} else if (IsPushKey(Key::W)) {
			position.y -= move;
		}

		DrawSprite(sprite, position);
		RunaEngine::EndFrame();
	}
	RunaEngine::Shutdown();
	return 0;

}

