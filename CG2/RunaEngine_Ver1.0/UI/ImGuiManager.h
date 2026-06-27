#pragma once
#include <d3d12.h>
#pragma comment(lib,"d3d12.lib")

class DirectXCommon;
class TextureManager;
class WinApp;

class ImGuiManager {
public:
	void Initialize(const WinApp& winApp, const DirectXCommon& directXCommon, TextureManager& textureManager);
	void BeginFrame();
	void Render(ID3D12GraphicsCommandList* commandList);
	void Shutdown();

private:
	bool initialized_ = false;
};
