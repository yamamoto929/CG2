#include "Sprite.h"
#include "VertexData.h"
#include "AffineMatrix.h"
#include "WVPMatrix.h"

namespace RunaEngine {

void Sprite::Initialize(
    ID3D12Device* device,
    TextureManager* textureManager,
    const std::string& texturePath
) {
	// Sprite用の頂点リソースを作る
	vertexResource_ = CreateBufferResource(device, sizeof(VertexData) * 4);
	// 頂点バッファビューを作成する
	// リソースの先頭のアドレスから使う
	vertexBufferView_.BufferLocation = vertexResource_->GetGPUVirtualAddress();
	// 使用するリソースのサイズは頂点6つ分のサイズ
	vertexBufferView_.SizeInBytes = sizeof(VertexData) * 4;
	// 1頂点あたりのサイズ
	vertexBufferView_.StrideInBytes = sizeof(VertexData);
	vertexResource_->Map(0, nullptr, reinterpret_cast<void**>(&vertexData_));

	size_ = { 640.0f, 360.0f };
	pivot_ = { 0.0f, 0.0f };

	UpdateVertexData();

	material_.Initialize(device);
	materialData_ = material_.GetData();
	color_ = { 1.0f,1.0f,1.0f,1.0f };
	materialData_->color = color_;
	materialData_->enableLighting = false;
	materialData_->uvTransform = MakeIdentityMatrix();

	// Sprite用のTransformation Matrix用のリソースを作る。Matrix4x4 1つ分のサイズを用意する
	transformationMatrix_.Initialize(device);
	// データを書き込む
	// 書き込むためのアドレスを取得
	transformationMatrixData_ = transformationMatrix_.GetData();
	// 単位行列を書きこんでおく
	transformationMatrixData_->World = MakeIdentityMatrix();
	transform_={ {1.0f, 1.0f, 1.0f}, {0.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 0.0f} };

	indexResource_ = CreateBufferResource(device, sizeof(uint32_t) * 6);
	// IBVの作成
	indexBufferView_.BufferLocation = indexResource_->GetGPUVirtualAddress();
	indexBufferView_.SizeInBytes = sizeof(uint32_t) * 6;
	indexBufferView_.Format = DXGI_FORMAT_R32_UINT;

	// インデックスリソースにデータを書き込む
	uint32_t* indexData = nullptr;
	indexResource_->Map(0, nullptr, reinterpret_cast<void**>(&indexData));
	indexData[0] = 0;
	indexData[1] = 1;
	indexData[2] = 2;
	indexData[3] = 1;
	indexData[4] = 3;
	indexData[5] = 2;

	textureHandle_ = textureManager->Load(texturePath);
}

void  Sprite::Update(int32_t screenWidth, int32_t screenHeight) {
	transformationMatrixData_->World = MakeAffineMatrix(transform_.scale, transform_.rotate, transform_.translate);
	Matrix4x4 viewMatrixSprite = MakeIdentityMatrix();
	Matrix4x4 projectionMatrixSprite = MakeOrthographicMatrix(0.0f, 0.0f, float(screenWidth), float(screenHeight), 0.0f, 100.0f);
	Matrix4x4 worldViewProjectionMatrixSprite = Multiply(transformationMatrixData_->World, Multiply(viewMatrixSprite, projectionMatrixSprite));
	transformationMatrixData_->WVP = worldViewProjectionMatrixSprite;
	Matrix4x4 uvTransformMatrix = MakeScaleMatrix(uvTransform_.scale);
	uvTransformMatrix = Multiply(uvTransformMatrix, MakeRotateZMatrix(uvTransform_.rotate.z));
	uvTransformMatrix = Multiply(uvTransformMatrix, MakeTranslateMatrix(uvTransform_.translate));

	materialData_->color = color_;
	materialData_->enableLighting = false;
	materialData_->uvTransform = uvTransformMatrix;
}

void  Sprite::Draw(ID3D12GraphicsCommandList* commandList, TextureManager* textureManager) {
	commandList->IASetVertexBuffers(0, 1, &vertexBufferView_);
	commandList->IASetIndexBuffer(&indexBufferView_);
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	commandList->SetGraphicsRootConstantBufferView(
		0,
		material_.GetGPUVirtualAddress()
	);

	commandList->SetGraphicsRootConstantBufferView(
		1,
		transformationMatrix_.GetGPUVirtualAddress()
	);

	commandList->SetGraphicsRootDescriptorTable(
		2,
		textureManager->GetSrvHandleGPU(textureHandle_)
	);

	commandList->DrawIndexedInstanced(6, 1, 0, 0, 0);
}

Transform& Sprite::GetTransform() {
	return transform_;
}

const Transform& Sprite::GetTransform() const {
	return transform_;
}

void Sprite::SetSize(const float& sizeX, const float& sizeY) {
	size_ = Vector2{ sizeX,sizeY };
	UpdateVertexData();
}

void Sprite::SetSize(const Vector2& size) {
	size_ = size;
	UpdateVertexData();
}

void Sprite::SetPivot(const float& pivotX, const float& pivotY) {
	pivot_ = Vector2{ pivotX,pivotY };
	UpdateVertexData();
}

void Sprite::SetPivot(const Vector2& pivot) {
	pivot_ = pivot;
	UpdateVertexData();
}

void Sprite::SetColor(const Vector4& color) {
	color_ = color;
}

void Sprite::SetUVTransform(const Matrix4x4& uvTransform) {
	if (materialData_) {
		materialData_->uvTransform = uvTransform;
	}
}

void Sprite::SetDrawOrder(int32_t drawOrder) {
	drawOrder_ = drawOrder;
}

int32_t Sprite::GetDrawOrder() const {
	return drawOrder_;
}

// =========================================================
// CreateBufferResource
// =========================================================
Microsoft::WRL::ComPtr<ID3D12Resource> Sprite::CreateBufferResource(ID3D12Device* device, size_t sizeInBytes) {
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
	HRESULT hr = device->CreateCommittedResource(
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

void Sprite::UpdateVertexData() {
	float left = -pivot_.x * size_.x;
	float right = left + size_.x;
	float top = -pivot_.y * size_.y;
	float bottom = top + size_.y;

	vertexData_[0].position = { left,  bottom, 0.0f, 1.0f };
	vertexData_[0].texCoord = { 0.0f, 1.0f };
	vertexData_[0].normal = { 0.0f, 0.0f, -1.0f };

	vertexData_[1].position = { left,  top, 0.0f, 1.0f };
	vertexData_[1].texCoord = { 0.0f, 0.0f };
	vertexData_[1].normal = { 0.0f, 0.0f, -1.0f };

	vertexData_[2].position = { right, bottom, 0.0f, 1.0f };
	vertexData_[2].texCoord = { 1.0f, 1.0f };
	vertexData_[2].normal = { 0.0f, 0.0f, -1.0f };

	vertexData_[3].position = { right, top, 0.0f, 1.0f };
	vertexData_[3].texCoord = { 1.0f, 0.0f };
	vertexData_[3].normal = { 0.0f, 0.0f, -1.0f };
}

} // namespace RunaEngine
