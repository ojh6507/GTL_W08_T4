#pragma once

#include <filesystem>
#include "Components/ActorComponent.h"
#include "UnrealEd/EditorPanel.h"


class Timer;

class GameUI : public UEditorPanel
{
public:
    GameUI();
    ~GameUI();

    // 초기화 및 종료
    void Initialize();
    void Shutdown();

    // 프레임 업데이트 및 렌더링
    void Render() override;
    void Update(float deltaTime);
    void OnResize(HWND hwnd) override;

    // 타이머 관련 UI 함수
    void RenderTimerUI();

    // Timer 객체 접근자
    Timer* GetTimer();

    void SetStartButtonCallback(std::function<void()> callback) {
        startButtonCallback = callback;
    }

    void SetPauseButtonCallback(std::function<void()> callback) {
        pauseButtonCallback = callback;
    }

    void SetResumeButtonCallback(std::function<void()> callback) {
        resumeButtonCallback = callback;
    }

    void SetResetButtonCallback(std::function<void()> callback) {
        resetButtonCallback = callback;
    }

    void SetDamageEventCallback(std::function<void()> callback) {
        damageEventCallback = callback;
    }


    void SetLives(int newLives) {
        lives = newLives; 
    }

    int GetLives() const {
        return lives;
    }

    void LoseLife() {
        if (lives > 0) {
            lives--;
        }
    }

    void ResetLives(int live) {
        lives = live;
    }
private:
    // 타이머 객체
    std::unique_ptr<Timer> gameTimer;

    // UI 관련 상태 변수들
    bool initialized;
    float Width = 0, Height = 0;
    // 시간 형식 변환 유틸리티 함수
    std::string FormatTime(float timeInSeconds);

    std::function<void()> startButtonCallback;
    std::function<void()> pauseButtonCallback;
    std::function<void()> resumeButtonCallback;
    std::function<void()> resetButtonCallback;
    std::function<void()> damageEventCallback;
    int lives;
};