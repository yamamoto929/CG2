#include "DirectXDebug.h"
#include <d3d12.h>
#include <wrl.h>
#include <d3d12sdklayers.h>
#include "EngineError.h"

void DirectXDebug::EnableDebugLayer() {
#ifdef _DEBUG
	Microsoft::WRL::ComPtr<ID3D12Debug1> debugController;
	if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)))) {
		debugController->EnableDebugLayer();
	}
#endif
}

void DirectXDebug::SetupInfoQueue(ID3D12Device* device) {
#ifdef _DEBUG
	Microsoft::WRL::ComPtr<ID3D12InfoQueue> queue;
	Require(device != nullptr, "DirectX debug setup requires a device");
	if (FAILED(device->QueryInterface(IID_PPV_ARGS(&queue)))) {
		Log("DirectX debug layer is unavailable; install Windows Graphics Tools for debug messages");
		return;
	}
	CheckHR(queue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_CORRUPTION, TRUE), "Break on DirectX corruption");
	CheckHR(queue->SetBreakOnSeverity(D3D12_MESSAGE_SEVERITY_ERROR, TRUE), "Break on DirectX error");
#else
	(void)device;
#endif
}
