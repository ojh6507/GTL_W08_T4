#pragma once

#include <fmod.hpp>
#include <unordered_map>
#include <vector>
#include <string>

// 선언만 함
extern std::vector<std::string> SoundFileNames;

class SoundManager {
public:
    static SoundManager& GetInstance();
    bool Initialize();
    void ShutDown();
    bool LoadSound(const std::string& name, const std::string& filePath, bool loop = false);
    void PlaySound(const std::string& name);
    void Update();
    void LoadSoundFiles();

private:
    SoundManager();
    ~SoundManager();
    SoundManager(const SoundManager&) = delete;
    SoundManager& operator=(const SoundManager&) = delete;

    FMOD::System* system;
    std::unordered_map<std::string, FMOD::Sound*> soundMap;
    std::vector<FMOD::Channel*> activeChannels; // 활성 채널들 관리
};