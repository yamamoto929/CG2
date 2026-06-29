#include "SoundManager.h"
#include "ConvertString.h"
#include <cassert>
#include <cstring>
#include <mfapi.h>
#include <mfidl.h>
#include <mfreadwrite.h>
#pragma comment(lib,"Mf.lib")
#pragma comment(lib,"mfplat.lib")
#pragma comment(lib,"Mfreadwrite.lib")
#pragma comment(lib,"mfuuid.lib")

SoundManager::~SoundManager() {
	Shutdown();
}

void SoundManager::Initialize() {
	if (initialized_) {
		return;
	}

	HRESULT hr = MFStartup(MF_VERSION, MFSTARTUP_NOSOCKET);
	assert(SUCCEEDED(hr));

	hr = XAudio2Create(xAudio2_.GetAddressOf(), 0, XAUDIO2_DEFAULT_PROCESSOR);
	assert(SUCCEEDED(hr));

	hr = xAudio2_->CreateMasteringVoice(&masterVoice_);
	assert(SUCCEEDED(hr));

	initialized_ = true;
}

uint32_t SoundManager::Load(const std::wstring& filePath) {
	assert(initialized_);

	if (soundHandles_.contains(filePath)) {
		return soundHandles_[filePath];
	}

	SoundData soundData = LoadWaveFile(filePath);
	uint32_t soundHandle = static_cast<uint32_t>(sounds_.size());
	sounds_.push_back(std::move(soundData));
	soundHandles_[filePath] = soundHandle;

	return soundHandle;
}

uint32_t SoundManager::Load(const std::string& filePath) {
	return Load(ConvertString(filePath));
}

void SoundManager::Play(uint32_t soundHandle, bool loop, float volume) {
	assert(initialized_);
	assert(soundHandle < sounds_.size());

	const SoundData& soundData = sounds_[soundHandle];

	IXAudio2SourceVoice* sourceVoice = nullptr;
	HRESULT hr = xAudio2_->CreateSourceVoice(&sourceVoice, &soundData.waveFormat);
	assert(SUCCEEDED(hr));

	XAUDIO2_BUFFER buffer{};
	buffer.pAudioData = soundData.mediaData.data();
	buffer.AudioBytes = static_cast<UINT32>(soundData.mediaData.size());
	buffer.Flags = XAUDIO2_END_OF_STREAM;
	buffer.LoopCount = loop ? XAUDIO2_LOOP_INFINITE : 0;

	hr = sourceVoice->SubmitSourceBuffer(&buffer);
	assert(SUCCEEDED(hr));

	sourceVoice->SetVolume(volume);
	hr = sourceVoice->Start();
	assert(SUCCEEDED(hr));

	playingVoices_.push_back({ sourceVoice });
}

void SoundManager::Update() {
	for (auto it = playingVoices_.begin(); it != playingVoices_.end();) {
		XAUDIO2_VOICE_STATE state{};
		it->voice->GetState(&state);
		if (state.BuffersQueued == 0) {
			it->voice->DestroyVoice();
			it = playingVoices_.erase(it);
		} else {
			++it;
		}
	}
}

void SoundManager::StopAll() {
	for (PlayingVoice& playingVoice : playingVoices_) {
		if (playingVoice.voice) {
			playingVoice.voice->Stop();
			playingVoice.voice->DestroyVoice();
			playingVoice.voice = nullptr;
		}
	}
	playingVoices_.clear();
}

void SoundManager::Shutdown() {
	if (!initialized_) {
		return;
	}

	StopAll();

	if (masterVoice_) {
		masterVoice_->DestroyVoice();
		masterVoice_ = nullptr;
	}

	xAudio2_.Reset();
	sounds_.clear();
	soundHandles_.clear();

	MFShutdown();
	initialized_ = false;
}

SoundManager::SoundData SoundManager::LoadWaveFile(const std::wstring& filePath) {
	Microsoft::WRL::ComPtr<IMFSourceReader> sourceReader;
	HRESULT hr = MFCreateSourceReaderFromURL(filePath.c_str(), nullptr, &sourceReader);
	assert(SUCCEEDED(hr));

	Microsoft::WRL::ComPtr<IMFMediaType> mediaType;
	hr = MFCreateMediaType(&mediaType);
	assert(SUCCEEDED(hr));
	mediaType->SetGUID(MF_MT_MAJOR_TYPE, MFMediaType_Audio);
	mediaType->SetGUID(MF_MT_SUBTYPE, MFAudioFormat_PCM);
	sourceReader->SetCurrentMediaType(MF_SOURCE_READER_FIRST_AUDIO_STREAM, nullptr, mediaType.Get());

	mediaType.Reset();
	sourceReader->GetCurrentMediaType(MF_SOURCE_READER_FIRST_AUDIO_STREAM, &mediaType);

	WAVEFORMATEX* waveFormat = nullptr;
	hr = MFCreateWaveFormatExFromMFMediaType(mediaType.Get(), &waveFormat, nullptr);
	assert(SUCCEEDED(hr));

	SoundData soundData;
	soundData.waveFormat = *waveFormat;
	CoTaskMemFree(waveFormat);

	while (true) {
		Microsoft::WRL::ComPtr<IMFSample> sample;
		DWORD streamFlags = 0;
		hr = sourceReader->ReadSample(
			MF_SOURCE_READER_FIRST_AUDIO_STREAM,
			0,
			nullptr,
			&streamFlags,
			nullptr,
			&sample
		);
		assert(SUCCEEDED(hr));

		if (streamFlags & MF_SOURCE_READERF_ENDOFSTREAM) {
			break;
		}

		if (!sample) {
			continue;
		}

		Microsoft::WRL::ComPtr<IMFMediaBuffer> mediaBuffer;
		hr = sample->ConvertToContiguousBuffer(&mediaBuffer);
		assert(SUCCEEDED(hr));

		BYTE* buffer = nullptr;
		DWORD currentLength = 0;
		hr = mediaBuffer->Lock(&buffer, nullptr, &currentLength);
		assert(SUCCEEDED(hr));

		size_t oldSize = soundData.mediaData.size();
		soundData.mediaData.resize(oldSize + currentLength);
		memcpy(soundData.mediaData.data() + oldSize, buffer, currentLength);

		mediaBuffer->Unlock();
	}

	return soundData;
}
