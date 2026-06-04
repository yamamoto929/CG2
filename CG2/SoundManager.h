#pragma once
#include <Windows.h>
#include <cstdint>
#include <string>
#include <unordered_map>
#include <vector>
#include <wrl.h>
#include <xaudio2.h>
#pragma comment(lib,"xaudio2.lib")

class SoundManager {
public:
	~SoundManager();

	void Initialize();
	uint32_t Load(const std::wstring& filePath);
	uint32_t Load(const std::string& filePath);
	void Play(uint32_t soundHandle, bool loop = false, float volume = 1.0f);
	void Update();
	void StopAll();
	void Shutdown();

private:
	struct SoundData {
		WAVEFORMATEX waveFormat{};
		std::vector<BYTE> mediaData;
	};

	struct PlayingVoice {
		IXAudio2SourceVoice* voice = nullptr;
	};

	SoundData LoadWaveFile(const std::wstring& filePath);

	Microsoft::WRL::ComPtr<IXAudio2> xAudio2_;
	IXAudio2MasteringVoice* masterVoice_ = nullptr;
	std::vector<SoundData> sounds_;
	std::unordered_map<std::wstring, uint32_t> soundHandles_;
	std::vector<PlayingVoice> playingVoices_;
	bool initialized_ = false;
};
