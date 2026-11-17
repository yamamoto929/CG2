#include "RunaEngine/RunaEngine.h"
#include "RunaEngine/Math/AABB.h"
#include "RunaEngine/Math/AffineMatrix.h"
#include "RunaEngine/Math/RandomNumberGenerator.h"
#include <array>
#include <cmath>
#include <filesystem>
#include <fstream>
#include <iomanip>
#include <limits>
#include <list>
#include <locale>
#include <string>
#include <system_error>

using namespace RunaEngine;
namespace {
	// パーティクル1個分のデータ
	struct Particle {
		Transform transform;
		Vector3 velocity;
		Vector4 color;
		float lifeTime;
		float currentTime;
	};

	struct Emitter {
		Transform transform;
		uint32_t count;
		float frequency;
		float upwardSpeed = 0.0f;
	};

	struct AccelerationField {
		Vector3 acceleration;
		AABB area;
	};

	// Editorで切り替えるパーティクルの設定
	struct EffectPreset {
		const char* name = "Light Fountain";
		Emitter emitter{
			Transform{ { 1.0f, 1.0f, 1.0f }, {}, {} },
			4, 0.25f, 2.5f
		};
		float size = 0.14f;
		float minLife = 1.0f;
		float maxLife = 2.0f;
		std::array<float, 3> colorA{ 0.25f, 0.75f, 1.0f };
		std::array<float, 3> colorB{ 0.95f, 1.0f, 1.0f };
		bool useBillboard = true;
		bool useWind = false;
		float windStrength = 5.0f;
	};

	using EffectPresets = std::array<EffectPreset, 2>;

	// 光の泉の初期値をもとに、火の粉の設定を作る
	EffectPresets MakeDefaultPresets()
	{
		EffectPresets presets{};

		EffectPreset& embers = presets[1];
		embers.name = "Windblown Embers";
		embers.emitter.count = 3;
		embers.emitter.frequency = 0.20f;
		embers.emitter.upwardSpeed = 1.2f;
		embers.size = 0.09f;
		embers.minLife = 0.7f;
		embers.maxLife = 1.4f;
		embers.colorA = { 1.0f, 0.30f, 0.02f };
		embers.colorB = { 1.0f, 0.85f, 0.20f };
		embers.useWind = true;

		return presets;
	}

	// プリセットの範囲内でパーティクルを生成する
	Particle MakeNewParticle(RNG& rng, const EffectPreset& preset)
	{
		Particle particle{};
		particle.transform.scale = { preset.size, preset.size, preset.size };
		particle.lifeTime = rng.Generate(preset.minLife, preset.maxLife);

		// 2色の間からランダムに色を選ぶ
		const float t = rng.Generate(0.0f, 1.0f);
		particle.color = {
			std::lerp(preset.colorA[0], preset.colorB[0], t),
			std::lerp(preset.colorA[1], preset.colorB[1], t),
			std::lerp(preset.colorA[2], preset.colorB[2], t),
			1.0f
		};

		// Emitterの位置を中心に、発生位置と初速をばらつかせる
		const Vector3& center = preset.emitter.transform.translate;
		particle.transform.translate = center + Vector3{
			rng.Generate(-0.25f, 0.25f),
			rng.Generate(-0.10f, 0.10f),
			rng.Generate(-0.25f, 0.25f)
		};
		particle.velocity = {
			rng.Generate(-0.65f, 0.65f),
			rng.Generate(-0.35f, 0.35f) + preset.emitter.upwardSpeed,
			rng.Generate(-0.65f, 0.65f)
		};

		return particle;
	}

	void Emit(const EffectPreset& preset, uint32_t count, RNG& rng,
		std::list<Particle>& particles)
	{
		for (uint32_t i = 0; i < count; ++i) {
			particles.push_back(MakeNewParticle(rng, preset));
		}
	}

	//============================================================
	// プリセットの読み込みと保存
	//============================================================
	constexpr char kParticlePresetPath[] = "resources/particle_preset.txt";
	constexpr int kMinEmitCount = 1;
	constexpr int kMaxEmitCount = static_cast<int>(kMaxParticleInstances);
	constexpr float kMinFrequency = 0.05f;
	constexpr float kMaxFrequency = 5.0f;
	constexpr float kMaxEmitterPosition = 100.0f;
	constexpr float kMaxUpwardSpeed = 10.0f;
	constexpr float kMaxWindStrength = 50.0f;

	bool IsInRange(float value, float minValue, float maxValue)
	{
		return minValue <= value && value <= maxValue;
	}

	// プリセットが1種類だったときの設定を読み込む
	bool LoadParticlePreset(Emitter& emitter, AccelerationField& wind,
		bool& useWind, bool& useBillboard)
	{
		std::ifstream file(kParticlePresetPath);
		if (!file.is_open()) {
			return false;
		}
		file.imbue(std::locale::classic());

		Emitter loadedEmitter = emitter;
		Vector3& position = loadedEmitter.transform.translate;
		int count = 0;
		float windStrength = 0.0f;
		bool loadedUseWind = false;
		bool loadedUseBillboard = false;
		file >> position.x >> position.y >> position.z
			 >> count >> loadedEmitter.frequency >> loadedEmitter.upwardSpeed
			 >> windStrength >> loadedUseWind >> loadedUseBillboard;

		if (file.fail()) {
			return false;
		}
		if (count < kMinEmitCount || kMaxEmitCount < count) {
			return false;
		}
		if (!IsInRange(loadedEmitter.frequency, kMinFrequency, kMaxFrequency)) {
			return false;
		}
		if (!IsInRange(loadedEmitter.upwardSpeed, -kMaxUpwardSpeed, kMaxUpwardSpeed)
			|| !IsInRange(windStrength, 0.0f, kMaxWindStrength)) {
			return false;
		}
		if (!IsInRange(position.x, -kMaxEmitterPosition, kMaxEmitterPosition)
			|| !IsInRange(position.y, -kMaxEmitterPosition, kMaxEmitterPosition)
			|| !IsInRange(position.z, -kMaxEmitterPosition, kMaxEmitterPosition)) {
			return false;
		}

		loadedEmitter.count = static_cast<uint32_t>(count);
		emitter = loadedEmitter;
		wind.acceleration.x = windStrength;
		useWind = loadedUseWind;
		useBillboard = loadedUseBillboard;
		return true;
	}

	constexpr char kEffectPresetsPath[] = "resources/particle_presets.txt";
	constexpr char kEffectPresetsVersion[] = "CG2_PARTICLE_PRESETS_2";
	constexpr char kPreviousEffectPresetsVersion[] = "CG2_PARTICLE_PRESETS_1";

	bool IsValidEffectPreset(const EffectPreset& preset, int count)
	{
		const Vector3& position = preset.emitter.transform.translate;
		if (count < kMinEmitCount || count > kMaxEmitCount) {
			return false;
		}
		if (!IsInRange(position.x, -kMaxEmitterPosition, kMaxEmitterPosition)
			|| !IsInRange(position.y, -kMaxEmitterPosition, kMaxEmitterPosition)
			|| !IsInRange(position.z, -kMaxEmitterPosition, kMaxEmitterPosition)) {
			return false;
		}
		if (!IsInRange(preset.emitter.frequency, kMinFrequency, kMaxFrequency)
			|| !IsInRange(preset.emitter.upwardSpeed, -kMaxUpwardSpeed, kMaxUpwardSpeed)
			|| !IsInRange(preset.windStrength, 0.0f, kMaxWindStrength)) {
			return false;
		}
		if (!IsInRange(preset.size, 0.02f, 1.0f)
			|| !IsInRange(preset.minLife, 0.05f, 5.0f)
			|| !IsInRange(preset.maxLife, preset.minLife, 5.0f)) {
			return false;
		}
		for (std::size_t i = 0; i < preset.colorA.size(); ++i) {
			if (!IsInRange(preset.colorA[i], 0.0f, 1.0f)
				|| !IsInRange(preset.colorB[i], 0.0f, 1.0f)) {
				return false;
			}
		}
		return true;
	}

	bool LoadEffectPresets(EffectPresets& presets, std::size_t& selected)
	{
		std::ifstream file(kEffectPresetsPath);
		if (!file.is_open()) {
			return false;
		}
		file.imbue(std::locale::classic());

		std::string version;
		int loadedIndex = 0;
		file >> version >> loadedIndex;
		if (file.fail()) {
			return false;
		}
		const bool previousFormat = version == kPreviousEffectPresetsVersion;
		if (!previousFormat && version != kEffectPresetsVersion) {
			return false;
		}

		const int presetCount = static_cast<int>(presets.size());
		// 旧ファイルで3番が選ばれていたら光の泉から始める
		if (previousFormat && loadedIndex == presetCount) {
			loadedIndex = 0;
		}
		if (loadedIndex < 0 || loadedIndex >= presetCount) {
			return false;
		}

		// 2種類とも読み込めたら、再生する設定に反映する
		EffectPresets loaded = presets;
		for (EffectPreset& preset : loaded) {
			Vector3& position = preset.emitter.transform.translate;
			int count = 0;
			file >> position.x >> position.y >> position.z
				 >> count >> preset.emitter.frequency >> preset.emitter.upwardSpeed
				 >> preset.size >> preset.minLife >> preset.maxLife
				 >> preset.colorA[0] >> preset.colorA[1] >> preset.colorA[2]
				 >> preset.colorB[0] >> preset.colorB[1] >> preset.colorB[2]
				 >> preset.useBillboard >> preset.useWind >> preset.windStrength;
			if (file.fail()) {
				return false;
			}
			if (!IsValidEffectPreset(preset, count)) {
				return false;
			}
			preset.emitter.count = static_cast<uint32_t>(count);
			if (previousFormat) {
				// 旧ファイルでは行の末尾に円周用の設定が入っている
				file.ignore((std::numeric_limits<std::streamsize>::max)(), '\n');
			}
		}

		presets = loaded;
		selected = static_cast<std::size_t>(loadedIndex);
		return true;
	}

#ifdef USE_IMGUI
	bool SaveEffectPresets(const EffectPresets& presets, std::size_t selected)
	{
		for (const EffectPreset& preset : presets) {
			if (!IsValidEffectPreset(preset, static_cast<int>(preset.emitter.count))) {
				return false;
			}
		}

		// 一時ファイルに書き終わってから保存ファイルを置き換える
		std::filesystem::path tempPath(kEffectPresetsPath);
		tempPath += "." + std::to_string(GetCurrentProcessId()) + ".tmp";

		std::ofstream file(tempPath);
		if (!file.is_open()) {
			return false;
		}
		file.imbue(std::locale::classic());

		file << std::setprecision(std::numeric_limits<float>::max_digits10);
		file << kEffectPresetsVersion << ' ' << selected << '\n';
		for (const EffectPreset& preset : presets) {
			const Vector3& position = preset.emitter.transform.translate;
			file << position.x << ' ' << position.y << ' ' << position.z << ' '
				 << preset.emitter.count << ' '
				 << preset.emitter.frequency << ' ' << preset.emitter.upwardSpeed << ' '
				 << preset.size << ' ' << preset.minLife << ' ' << preset.maxLife << ' '
				 << preset.colorA[0] << ' ' << preset.colorA[1] << ' ' << preset.colorA[2] << ' '
				 << preset.colorB[0] << ' ' << preset.colorB[1] << ' ' << preset.colorB[2] << ' '
				 << preset.useBillboard << ' ' << preset.useWind << ' '
				 << preset.windStrength << '\n';
		}
		file.close();

		std::error_code error;
		if (!file.fail()) {
			std::filesystem::rename(tempPath, kEffectPresetsPath, error);
			if (!error) {
				return true;
			}
		}

		std::filesystem::remove(tempPath, error);
		return false;
	}
#endif
}
int WINAPI WinMain(_In_ HINSTANCE, _In_opt_ HINSTANCE, _In_ LPSTR, _In_ int) {

	//============================================================
	// 初期化
	//============================================================
	RunaEngine::Initialize(1280, 720, "LE2B_30_ヤマモト_ルナ");
	Model* model = CreateModel("resources/plane.obj");

	ModelDrawParameters param;
	param.lightingMode = LightingMode::NONE;
	auto& engine = RunaEngine::GetEngine();
	auto* instancingData = engine.GetInstancingData();

	RNG& rng = RNG::GetInstance();

	std::list<Particle> particles;
	EffectPresets presets = MakeDefaultPresets();
	std::size_t selected = 0;
	float emissionTime = 0.0f;
	const AABB windArea{
		{ -1.0f, -1.0f, -1.0f },
		{  1.0f,  1.0f,  1.0f }
	};

	// 保存した設定を読み込んでから、最初の10個を発生させる
	const bool presetsLoaded = LoadEffectPresets(presets, selected);
	if (!presetsLoaded) {
		EffectPreset& fountain = presets[0];
		AccelerationField oldWind{
			{ fountain.windStrength, 0.0f, 0.0f }, windArea
		};
		if (LoadParticlePreset(fountain.emitter, oldWind,
			fountain.useWind, fountain.useBillboard)) {
			fountain.windStrength = oldWind.acceleration.x;
		}
	}

	Emit(presets[selected], 10, rng, particles);

#ifdef USE_IMGUI
	const char* saveMessage = nullptr;
#endif
	while (RunaEngine::ProcessMessage()) {
		RunaEngine::BeginFrame();

		const std::size_t previousSelection = selected;
		bool useKeyboard = true;

#ifdef USE_IMGUI
		useKeyboard = !ImGui::GetIO().WantCaptureKeyboard;
#endif

		// 1と2で使うプリセットを切り替える
		if (useKeyboard) {
			if (engine.IsTriggerKey(Key::Digit1)) {
				selected = 0;
			} else if (engine.IsTriggerKey(Key::Digit2)) {
				selected = 1;
			}
		}

#ifdef USE_IMGUI
		// 選択中のプリセットをEditorで調整する
		if (ImGui::Begin("Particle Editor")) {
			constexpr ImGuiSliderFlags clamp = ImGuiSliderFlags_AlwaysClamp;
			if (ImGui::BeginCombo("Effect", presets[selected].name)) {
				for (std::size_t i = 0; i < presets.size(); ++i) {
					if (ImGui::Selectable(presets[i].name, selected == i)) {
						selected = i;
					}
				}
				ImGui::EndCombo();
			}

			EffectPreset& edited = presets[selected];
			ImGui::PushID(static_cast<int>(selected));

			Vector3& position = edited.emitter.transform.translate;
			float positionValues[3] = { position.x, position.y, position.z };
			if (ImGui::DragFloat3("Emitter Position", positionValues, 0.01f,
				-kMaxEmitterPosition, kMaxEmitterPosition, "%.2f", clamp)) {
				position = { positionValues[0], positionValues[1], positionValues[2] };
			}

			int count = static_cast<int>(edited.emitter.count);
			if (ImGui::DragInt("Particle Count", &count, 0.1f,
				kMinEmitCount, kMaxEmitCount, "%d", clamp)) {
				edited.emitter.count = static_cast<uint32_t>(count);
			}
			ImGui::DragFloat("Emission Interval (s)", &edited.emitter.frequency, 0.01f,
				kMinFrequency, kMaxFrequency, "%.2f", clamp);
			ImGui::DragFloat("Upward Speed", &edited.emitter.upwardSpeed, 0.05f,
				-kMaxUpwardSpeed, kMaxUpwardSpeed, "%.2f", clamp);

			ImGui::DragFloat("Particle Size", &edited.size, 0.01f,
				0.02f, 1.0f, "%.2f", clamp);
			ImGui::DragFloatRange2("Lifetime (s)", &edited.minLife, &edited.maxLife,
				0.01f, 0.05f, 5.0f, "%.2f", "%.2f", clamp);
			ImGui::ColorEdit3("Color A", edited.colorA.data());
			ImGui::ColorEdit3("Color B", edited.colorB.data());
			ImGui::Checkbox("Billboard", &edited.useBillboard);
			ImGui::Checkbox("Wind", &edited.useWind);
			ImGui::DragFloat("Wind Strength", &edited.windStrength, 0.1f,
				0.0f, kMaxWindStrength, "%.1f", clamp);
			ImGui::PopID();

			Transform camera = engine.GetCameraTransform();

			float positions[3] = {
				camera.translate.x, camera.translate.y, camera.translate.z
			};
			float rotation[3] = {
				camera.rotate.x, camera.rotate.y, camera.rotate.z
			};

			const bool positionChanged =
				ImGui::DragFloat3("Camera Position", &position.x, 0.05f);
			const bool rotationChanged =
				ImGui::DragFloat3("Camera Rotation (rad)", rotation, 0.01f);

			if (positionChanged || rotationChanged) {
				camera.translate = { positions[0], positions[1], positions[2] };
				camera.rotate = { rotation[0], rotation[1], rotation[2] };
				engine.SetCameraTransform(camera);
			}
			if (ImGui::Button("Save Presets")) {
				if (SaveEffectPresets(presets, selected)) {
					saveMessage = "Presets saved.";
				} else {
					saveMessage = "Could not save the presets.";
				}
			}
			if (saveMessage) {
				ImGui::TextUnformatted(saveMessage);
			}
		}
		ImGui::End();

		// Editorを閉じている間も操作が分かるように表示する
		const std::string controls = std::string("1: Light Fountain   2: Windblown Embers   Space: Emit   |   ")
			+ presets[selected].name;
		const ImVec2 textSize = ImGui::CalcTextSize(controls.c_str());
		const ImGuiViewport* viewport = ImGui::GetMainViewport();
		const ImVec2 position{ viewport->Pos.x + 16.0f,
			viewport->Pos.y + viewport->Size.y - textSize.y - 16.0f };
		ImDrawList* foreground = ImGui::GetForegroundDrawList();
		foreground->AddRectFilled(
			{ position.x - 6.0f, position.y - 4.0f },
			{ position.x + textSize.x + 6.0f, position.y + textSize.y + 4.0f },
			IM_COL32(16, 20, 28, 230), 4.0f
		);
		foreground->AddText(position, IM_COL32_WHITE, controls.c_str());
#endif

		//============================================================
		// パーティクルの発生
		//============================================================
		const EffectPreset& preset = presets[selected];
		const float dt = engine.GetDeltaTime();
		if (selected != previousSelection) {
			// 切り替えたときは粒と発生タイマーをリセットする
			particles.clear();
			emissionTime = 0.0f;
			Emit(preset, preset.emitter.count, rng, particles);
		} else if (useKeyboard && engine.IsTriggerKey(Key::Space)) {
			Emit(preset, preset.emitter.count, rng, particles);
		}

		// 発生間隔を超えた分の時間は次の発生に持ち越す
		emissionTime += dt;
		while (emissionTime >= preset.emitter.frequency) {
			Emit(preset, preset.emitter.count, rng, particles);
			emissionTime -= preset.emitter.frequency;
		}

		const auto viewProjection = engine.GetViewProjectionMatrix();
		// カメラの回転だけを取り出し、全パーティクルで使う
		const Matrix4x4 billboard = MakeAffineMatrix(
			{ 1.0f, 1.0f, 1.0f },
			engine.GetCameraTransform().rotate,
			{ 0.0f, 0.0f, 0.0f }
		);

		//============================================================
		// パーティクルの更新と描画データの作成
		//============================================================
		uint32_t drawCount = 0;

		for (auto it = particles.begin(); it != particles.end();) {
			Particle& particle = *it;
			particle.currentTime += dt;
			if (particle.currentTime >= particle.lifeTime) {
				// 寿命が切れた粒を削除し、次の粒に進む
				it = particles.erase(it);
				continue;
			}
			++it;

			// 風の範囲内にいる粒だけを加速させる
			if (preset.useWind && windArea.Contains(particle.transform.translate)) {
				particle.velocity.x += preset.windStrength * dt;
			}
			particle.transform.translate += particle.velocity * dt;

			// 描画できる数までGPUのバッファに詰める
			if (drawCount >= kMaxParticleInstances) {
				continue;
			}

			const Transform& transform = particle.transform;
			Matrix4x4 world;
			if (preset.useBillboard) {
				world = Multiply(
					Multiply(MakeScaleMatrix(transform.scale), billboard),
					MakeTranslateMatrix(transform.translate)
				);
			} else {
				world = MakeAffineMatrix(transform.scale, transform.rotate, transform.translate);
			}

			ParticleForGPU& gpuParticle = instancingData[drawCount];
			gpuParticle.World = world;
			gpuParticle.WVP = Multiply(world, viewProjection);
			gpuParticle.color = particle.color;
			// 残りの寿命に合わせて徐々に消す
			gpuParticle.color.w *= 1.0f - particle.currentTime / particle.lifeTime;

			++drawCount;
		}

		// GPUに書き込んだ数だけ板ポリを描画する
		if (model && drawCount > 0) {
			auto* commandList = engine.GetRenderer().GetCommandList();
			engine.SetParticlePipeline();
			commandList->SetGraphicsRootDescriptorTable(
				1, engine.GetInstancingSrvGpuHandle()
			);
			model->Draw(commandList, &engine.GetTextureManager(), param, drawCount);
		}
		RunaEngine::EndFrame();
	}

	RunaEngine::Shutdown();
	return 0;

}
