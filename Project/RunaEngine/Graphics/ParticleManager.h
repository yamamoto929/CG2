#pragma once
#include "RunaEngine/Math/Matrix4x4.h"
#include "RunaEngine/Math/Vector3.h"
#include "RunaEngine/Math/Vector4.h"
#include "RunaEngine/Math/Transform.h"
#include "RunaEngine/Graphics/ConstantBuffer.h"
#include "RunaEngine/Graphics/Material.h"
#include <d3d12.h>
#include <wrl.h>
#include <list>
#include <string>
#include <unordered_map>
#include <cstdint>
class DirectXCommon;
class TextureManager;
class GraphicsPipeline;
class ShaderCompiler;

namespace RunaEngine {
	class Camera;
}
class ParticleManager final
{
private:
	ParticleManager() = default;
	~ParticleManager() = default;
public:
	ParticleManager(const ParticleManager&) = delete;
	ParticleManager& operator=(const ParticleManager&) = delete;
	ParticleManager(ParticleManager&&) = delete;
	ParticleManager& operator=(ParticleManager&&) = delete;

	static ParticleManager& GetInstance() {
		static ParticleManager instance;
		return instance;
	}

	// パーティクル1個分のデータ
	struct Particle {
		RunaEngine::Transform transform;
		RunaEngine::Vector3 velocity;
		RunaEngine::Vector4 color;
		float lifeTime;
		float currentTime;
	};

	struct DataForGPU {
		RunaEngine::Matrix4x4 WVP;
		RunaEngine::Matrix4x4 World;
		RunaEngine::Vector4 color;
	};

	struct Settings
	{
		float size; // 発生時のxyzのscale
		float lifetime; // 寿命(seconds)
		RunaEngine::Vector3 color; // 色
		float upwardSpeed; // Y方向の初速
		bool useWind; // 更新で風を適用するか
		float windStrength; // 風の強さ
		bool useBillboard; // 板ポリをカメラに合わせるかどうか
	};

	void Initialize(DirectXCommon* directXCommon,
		TextureManager* textureManager,
		GraphicsPipeline* graphicsPipeline,
		ShaderCompiler* shaderCompiler);
	void CreateParticleGroup(const std::string& name, const std::string& texturePath);
	void SetGroupSettings(const std::string& name, const Settings& settings);
	void ClearParticles(const std::string& name);
	void Emit(const std::string& name, const RunaEngine::Vector3& position, uint32_t count);
	void Update(float dt, const RunaEngine::Camera& camera);
	void Draw();

	bool UsesTexture(uint32_t textureHandle) const;
	void ClearGroups();
	void Shutdown();

private:
	struct Group {
		struct MaterialData {
			std::string textureFilePath;
			uint32_t textureSrvIndex = 0;
		} material;

		uint32_t textureHandle = 0;
		std::list<Particle> particles;
		uint32_t instancingSrvIndex = 0;
		Microsoft::WRL::ComPtr<ID3D12Resource> instancingResource;
		uint32_t instanceCount = 0;
		DataForGPU* instancingData = nullptr;
		Settings settings{};
	};

	Group* FindGroup(const std::string& name);

	DirectXCommon* directXCommon_ = nullptr;
	TextureManager* textureManager_ = nullptr;
	GraphicsPipeline* graphicsPipeline_ = nullptr;

	Microsoft::WRL::ComPtr<ID3D12Resource> vertexResource_;
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView_{};
	ConstantBuffer<RunaEngine::Material> materialBuffer_;

	std::unordered_map<std::string, Group> groups_;
};