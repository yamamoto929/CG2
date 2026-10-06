#include "ImGuiManager.h"
#include "DirectXCommon.h"
#include "TextureManager.h"
#include "WinApp.h"

#ifdef USE_IMGUI
#include "externals/imgui/imgui.h"
#include "externals/imgui/imgui_impl_dx12.h"
#include "externals/imgui/imgui_impl_win32.h"
#endif

void ImGuiManager::Initialize(const WinApp& winApp, const DirectXCommon& directXCommon, TextureManager& textureManager) {
#ifdef USE_IMGUI
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGui::StyleColorsDark();
	ImGui_ImplWin32_Init(winApp.GetHwnd());
	ImGui_ImplDX12_Init(
		directXCommon.GetDevice(),
		directXCommon.GetSwapChainDesc().BufferCount,
		directXCommon.GetDXGIFormat(),
		textureManager.GetSrvDescriptorHeap(),
		textureManager.GetSrvHandleCPU(0),
		textureManager.GetSrvHandleGPU(0)
	);
	ImGuiIO& io = ImGui::GetIO();
	io.Fonts->Build();
#endif
	initialized_ = true;
}

void ImGuiManager::BeginFrame() {
#ifdef USE_IMGUI
	ImGui_ImplDX12_NewFrame();
	ImGui_ImplWin32_NewFrame();
	ImGui::NewFrame();
#endif
}

void ImGuiManager::Render(ID3D12GraphicsCommandList* commandList) {
#ifdef USE_IMGUI
	ImGui::Render();
	ImGui_ImplDX12_RenderDrawData(ImGui::GetDrawData(), commandList);
#else
	(void)commandList;
#endif
}

void ImGuiManager::Shutdown() {
	if (!initialized_) {
		return;
	}

#ifdef USE_IMGUI
	ImGui_ImplDX12_Shutdown();
	ImGui_ImplWin32_Shutdown();
	ImGui::DestroyContext();
#endif
	initialized_ = false;
}
