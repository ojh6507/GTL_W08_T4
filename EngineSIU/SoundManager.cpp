#include "SoundManager.h"
#include <iostream>

std::vector<std::string> SoundFileNames = { "Main" };

SoundManager& SoundManager::GetInstance() {
    static SoundManager instance;
    return instance;
}

SoundManager::SoundManager() : system(nullptr) {
}

SoundManager::~SoundManager() {
    ShutDown();
}

bool SoundManager::Initialize() {
    FMOD_RESULT result = FMOD::System_Create(&system);
    if (result != FMOD_OK) {
        std::cerr << "FMOD System_Create failed!" << std::endl;
        return false;
    }

    result = system->init(512, FMOD_INIT_NORMAL, nullptr);
    if (result != FMOD_OK) {
        std::cerr << "FMOD system init failed!" << std::endl;
        return false;
    }

   
    return true;
}

void SoundManager::ShutDown() {
    for (auto& pair : soundMap) {
        pair.second->release();
    }
    soundMap.clear();

    if (system) {
        system->close();
        system->release();
    }
}

bool SoundManager::LoadSound(const std::string& name, const std::string& filePath, bool loop) {


    if (soundMap.find(name) != soundMap.end()) {
        return true; // 이미 로드됨
    }

    FMOD::Sound* sound = nullptr;
    FMOD_MODE mode = loop ? FMOD_LOOP_NORMAL : FMOD_DEFAULT;

    if (system->createSound(filePath.c_str(), mode, nullptr, &sound) != FMOD_OK) {
        std::cerr << "Failed to load sound: " << filePath << std::endl;
        return false;
    }

    soundMap[name] = sound;
    return true;
}

void SoundManager::Stop(const std::string& name) {
    auto it = soundMap.find(name);
    if (it != soundMap.end()) {
        // 해당 사운드가 재생 중인 모든 채널 정지
        for (auto channelIt = activeChannels.begin(); channelIt != activeChannels.end(); ) {
            FMOD::Sound* currentSound = nullptr;
            (*channelIt)->getCurrentSound(&currentSound);

            if (currentSound == it->second) {
                (*channelIt)->stop();
                channelIt = activeChannels.erase(channelIt);
            }
            else {
                ++channelIt;
            }
        }
    }
}

void SoundManager::PlaySound(const std::string& name) {
    auto it = soundMap.find(name);
    if (it != soundMap.end()) {
        // 같은 사운드 먼저 정지
        Stop(name);

        // 새로 재생
        FMOD::Channel* newChannel = nullptr;
        system->playSound(it->second, nullptr, false, &newChannel);
        if (newChannel) {
            activeChannels.push_back(newChannel);
        }
    }
}

void SoundManager::Update() {
    system->update();

    // 채널 리스트에서 재생이 끝난 채널 제거
    activeChannels.erase(
        std::remove_if(activeChannels.begin(), activeChannels.end(),
            [](FMOD::Channel* channel) {
                bool isPlaying = false;
                if (channel) {
                    channel->isPlaying(&isPlaying);
                }
                return !isPlaying; // 재생이 끝난 채널 제거
            }),
        activeChannels.end()
    );
}

void SoundManager::LoadSoundFiles()
{
    for (auto name : SoundFileNames)
    {
        std::string FileName;
        bool ret = LoadSound(name, "Assets/Sound/"+name+".mp3", true);
    }
}
