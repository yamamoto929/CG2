#pragma once
struct ID3D12Device;

class DirectXDebug {
public:
	static void EnableDebugLayer();
	static void SetupInfoQueue(ID3D12Device* device);
};
