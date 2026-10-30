#pragma once
#include<Gorgon/Main.h>
#include<Gorgon/Multimedia/Wave.h>
#include<Gorgon/Audio/Controllers.h>
#include<iostream>
#include<unordered_map>
class AudioManager {
private:
    std::unordered_map<std::string, Gorgon::Multimedia::Wave> soundCache;
    std::unordered_map<std::string, Gorgon::Audio::BasicController> activeControllers;
    int nextControllerID = 0;

public:

    static AudioManager& Get(){
        static AudioManager instance;
        return instance;
    }

    void LoadSound(const std::string& name, const std::string& filePath) {
        Gorgon::Multimedia::Wave wave;
        wave.Import(filePath);
        soundCache[name] = std::move(wave);
    }

    int PlaySound(const std::string& soundName,float volume=0.6f, bool loop = false) {
        if (soundCache.find(soundName) == soundCache.end()) {
            return -1;  
        }

        int id = nextControllerID++;
        activeControllers[soundName + "_" + std::to_string(id)]
            .SetData(soundCache[soundName]);
        activeControllers[soundName + "_" + std::to_string(id)]
            .SetVolume(volume);
        if (loop) {
            activeControllers[soundName + "_" + std::to_string(id)].Loop();
        } else {
            activeControllers[soundName + "_" + std::to_string(id)].Play();
        }
        
        return id;
    }

    void StopSound(const std::string& soundName, int id) {
        std::string key = soundName + "_" + std::to_string(id);
        if (activeControllers.find(key) != activeControllers.end()) {
            activeControllers[key].Pause();
            activeControllers[key].ReleaseData();
            activeControllers.erase(key);
        }
    }
    void Update() {
        auto it = activeControllers.begin();
        while (it != activeControllers.end()) {
            if (it->second.IsFinished() && !it->second.IsLooping()) {
                it->second.ReleaseData();
                it = activeControllers.erase(it);
            } else {
                ++it;
            }
        }
    }
};