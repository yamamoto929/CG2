#include "DirectXDebug.h"
#include <d3d12.h>
#include <wrl.h>

void DirectXDebug::EnableDebugLayer() {
#ifdef _DEBUG
	Microsoft::WRL::ComPtr<ID3D12Debug1> debugController;
	if (SUCCEEDED(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)))) {
		debugController->EnableDebugLayer();
	}
#endif
}

void DirectXDebug::SetupInfoQueue() {
}
