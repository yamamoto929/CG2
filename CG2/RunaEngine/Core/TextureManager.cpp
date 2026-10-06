#include <cassert>
#include "TextureManager.h"
#include "ConvertString.h"
#include <filesystem>
#include <algorithm>
void TextureManager::Initialize(ID3D12Device* device, ID3D12GraphicsCommandList* commandList, uint32_t maxTextureCount) {

	// SRV用のディスクリプタヒープ
	device_ = device;
	commandList_ = commandList;
	srvDescriptorHeap_.Initialize(device_, D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV, maxTextureCount, true);
	// 0番はImGui用に予約
	// 再初期化しても画像番号は戻さない。古い番号を新しい画像へ使い回さない。
	maxTextureCount_ = maxTextureCount;
	nextDescriptorIndex_ = 1;
}

uint32_t TextureManager::Load(const std::string& filePath) {

    const auto key = std::filesystem::weakly_canonical(filePath).generic_string();
    if (textureHandles_.contains(key)) {
        return textureHandles_[key];
    }
    if (freeDescriptorIndices_.empty() && nextDescriptorIndex_ >= maxTextureCount_) {
        return 0;
    }
    if (nextIndex_ == UINT32_MAX) { return 0; }
    const uint32_t descriptorIndex = freeDescriptorIndices_.empty() ? nextDescriptorIndex_ : freeDescriptorIndices_.back();
    uint32_t textureHandle = nextIndex_;

    DirectX::ScratchImage mipImages = LoadTexture(key);
    if (mipImages.GetImageCount() == 0) { return 0; }
    const DirectX::TexMetadata& metadata = mipImages.GetMetadata();

    Microsoft::WRL::ComPtr<ID3D12Resource> textureResource =
        CreateTextureResource(metadata);

    Microsoft::WRL::ComPtr<ID3D12Resource> intermediateResource =
        UploadTextureData(textureResource, mipImages);

    D3D12_SHADER_RESOURCE_VIEW_DESC srvDesc{};
    srvDesc.Format = metadata.format;
    srvDesc.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    srvDesc.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    srvDesc.Texture2D.MipLevels = UINT(metadata.mipLevels);

    device_->CreateShaderResourceView(
        textureResource.Get(),
        &srvDesc,
        srvDescriptorHeap_.GetCPUDescriptorHandle(descriptorIndex)
    );

    textureResources_[textureHandle] = textureResource;
    intermediateResources_.push_back(intermediateResource);
    textureHandles_[key] = textureHandle;
    textureSizes_[textureHandle] = {static_cast<uint32_t>(metadata.width), static_cast<uint32_t>(metadata.height)};
    descriptorIndices_[textureHandle] = descriptorIndex;
    if (freeDescriptorIndices_.empty()) { ++nextDescriptorIndex_; }
    else { freeDescriptorIndices_.pop_back(); }
    ++nextIndex_;

    return textureHandle;
}

D3D12_GPU_DESCRIPTOR_HANDLE TextureManager::GetSrvHandleGPU(uint32_t textureHandle) {
	if (textureHandle == 0) { return srvDescriptorHeap_.GetGPUDescriptorHandle(0); }
	const auto found = descriptorIndices_.find(textureHandle);
	if (found == descriptorIndices_.end()) { return {}; }
	return srvDescriptorHeap_.GetGPUDescriptorHandle(found->second);
}

ID3D12DescriptorHeap* TextureManager::GetSrvDescriptorHeap() const {
	return srvDescriptorHeap_.Get();
}

D3D12_CPU_DESCRIPTOR_HANDLE TextureManager::GetSrvHandleCPU(uint32_t index) {
	return srvDescriptorHeap_.GetCPUDescriptorHandle(index);
}

TextureSize TextureManager::GetTextureSize(uint32_t textureHandle)const {
	const auto found = textureSizes_.find(textureHandle);
	if (found == textureSizes_.end()) { return {}; }
	return found->second;
}

void TextureManager::ReleaseUploadResources() { intermediateResources_.clear(); }
void TextureManager::Unload(uint32_t handle) {
    if (HasPendingUploads()) { return; }
    const auto found = descriptorIndices_.find(handle);
    if (found == descriptorIndices_.end()) { return; }
    const auto slot = found->second;
    D3D12_SHADER_RESOURCE_VIEW_DESC nullView{};
    nullView.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    nullView.ViewDimension = D3D12_SRV_DIMENSION_TEXTURE2D;
    nullView.Shader4ComponentMapping = D3D12_DEFAULT_SHADER_4_COMPONENT_MAPPING;
    nullView.Texture2D.MipLevels = 1;
    device_->CreateShaderResourceView(nullptr, &nullView, srvDescriptorHeap_.GetCPUDescriptorHandle(slot));
    freeDescriptorIndices_.push_back(slot);
    descriptorIndices_.erase(handle);
    textureResources_.erase(handle);
    textureSizes_.erase(handle);
    std::erase_if(textureHandles_, [handle](const auto& entry) { return entry.second == handle; });
}
void TextureManager::Clear() {
    if (HasPendingUploads()) { return; }
    textureHandles_.clear(); textureResources_.clear(); textureSizes_.clear(); descriptorIndices_.clear();
    freeDescriptorIndices_.clear(); nextDescriptorIndex_ = 1;
    // nextIndex_は戻さない。古い画像番号が新しい画像を指さないため。
}

// =========================================================
// CreateTextureResource
// =========================================================
Microsoft::WRL::ComPtr<ID3D12Resource> TextureManager::CreateTextureResource(const DirectX::TexMetadata& metadata) {
	// metadataを基にResourceの設定
	D3D12_RESOURCE_DESC resourceDesc{};
	resourceDesc.Width = UINT(metadata.width); // Textureの幅
	resourceDesc.Height = UINT(metadata.height); // Textureの高さ
	resourceDesc.MipLevels = UINT16(metadata.mipLevels); // mipmapの数
	resourceDesc.DepthOrArraySize = UINT16(metadata.arraySize); // 奥行き or 配列Textureの配列数
	resourceDesc.Format = metadata.format; // TextureのFormat
	resourceDesc.SampleDesc.Count = 1; // サンプリングカウント。1固定。
	resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION(metadata.dimension); // Textureの次元数。普段使っているのは2次元
	// 利用するHeapの設定。
	D3D12_HEAP_PROPERTIES heapProperties{};
	heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT;
	// Resourceの生成
	Microsoft::WRL::ComPtr<ID3D12Resource> resource;
	HRESULT hr = device_->CreateCommittedResource(
		&heapProperties, // Heapの設定
		D3D12_HEAP_FLAG_NONE, // Heapの特殊な設定。特になし。
		&resourceDesc, // Resourceの設定
		D3D12_RESOURCE_STATE_COPY_DEST, // データ転送される設定
		nullptr, // Clear最適値。使わないのでnullptr
		IID_PPV_ARGS(&resource)); // 作成するResourceポインタへのポインタ
	assert(SUCCEEDED(hr));
	return resource;
}

// =========================================================
// LoadTexture 
// =========================================================
DirectX::ScratchImage TextureManager::LoadTexture(const std::string& filePath) {
	// テクスチャファイルを読んでプログラムで扱えるようにする
	DirectX::ScratchImage image{};
	std::wstring filePathW = ConvertString(filePath);
	HRESULT hr = DirectX::LoadFromWICFile(filePathW.c_str(), DirectX::WIC_FLAGS_FORCE_SRGB, nullptr, image);
	if (FAILED(hr)) { return {}; }
	if (image.GetMetadata().dimension != DirectX::TEX_DIMENSION_TEXTURE2D || image.GetMetadata().arraySize != 1) {
	    return {};
	}

	// ミップマップの作成
	DirectX::ScratchImage mipImages{};
	hr = DirectX::GenerateMipMaps(image.GetImages(), image.GetImageCount(), image.GetMetadata(), DirectX::TEX_FILTER_SRGB, 0, mipImages);
	if (FAILED(hr)) { return {}; }

	// ミップマップ付きのデータを返す
	return mipImages;
}

// =========================================================
// UploadTextureData
// =========================================================
Microsoft::WRL::ComPtr<ID3D12Resource> TextureManager::UploadTextureData(
	const Microsoft::WRL::ComPtr<ID3D12Resource>& texture,
	const DirectX::ScratchImage& mipImages) {

	std::vector<D3D12_SUBRESOURCE_DATA> subresources;
	HRESULT hr = DirectX::PrepareUpload(device_, mipImages.GetImages(), mipImages.GetImageCount(), mipImages.GetMetadata(), subresources);
	assert(SUCCEEDED(hr));

	uint64_t intermediateSize = GetRequiredIntermediateSize(texture.Get(), 0, UINT(subresources.size()));
	Microsoft::WRL::ComPtr<ID3D12Resource> intermediateResource = CreateBufferResource(intermediateSize);

	UINT64 copiedBytes = UpdateSubresources(commandList_, texture.Get(), intermediateResource.Get(), 0, 0, UINT(subresources.size()), subresources.data());
	assert(copiedBytes != 0);

	// Textureへの転送後は利用できるよう、D3D12_RESOURCE_STATE_COPY_DESTからD3D12_RESOURCE_STATE_GENERIC_READへ変更する
	D3D12_RESOURCE_BARRIER barrier{};
	barrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	barrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	barrier.Transition.pResource = texture.Get();
	barrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_COPY_DEST;
	barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_GENERIC_READ;
	commandList_->ResourceBarrier(1, &barrier);

	return intermediateResource;
}

// =========================================================
// CreateBufferResource
// =========================================================
Microsoft::WRL::ComPtr<ID3D12Resource> TextureManager::CreateBufferResource(size_t sizeInBytes) {
	// 頂点リソース用のヒープの設定
	D3D12_HEAP_PROPERTIES uploadHeapProperties{};
	uploadHeapProperties.Type = D3D12_HEAP_TYPE_UPLOAD; // UploadHeapを使う
	// 頂点リソースの設定
	D3D12_RESOURCE_DESC resourceDesc{};
	// バッファリソース。テクスチャの場合はまた別の設定をする
	resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	resourceDesc.Width = sizeInBytes; // リソースのサイズ
	// バッファの場合はこれらを1にする決まり
	resourceDesc.Height = 1;
	resourceDesc.DepthOrArraySize = 1;
	resourceDesc.MipLevels = 1;
	resourceDesc.SampleDesc.Count = 1;
	// バッファの場合はこれにする決まり
	resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	// 実際にリソースを作る
	Microsoft::WRL::ComPtr<ID3D12Resource> resource;
	HRESULT hr = device_->CreateCommittedResource(
		&uploadHeapProperties,
		D3D12_HEAP_FLAG_NONE,
		&resourceDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ,
		nullptr,
		IID_PPV_ARGS(&resource)
	);
	assert(SUCCEEDED(hr));

	return resource;
}

