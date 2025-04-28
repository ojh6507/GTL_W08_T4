#pragma once

class Timer
{
public:
    Timer();
    ~Timer();

    // 타이머 제어 함수
    void Start();
    void Stop();
    void Reset();
    void Pause();
    void Resume();

    // 타이머 상태 확인 함수
    bool IsRunning() const;
    bool IsPaused() const;
    float GetTime() const;

    // 타이머 업데이트 (매 프레임 호출)
    void Update(float deltaTime);

private:
    // 타이머 상태 변수
    float currentTime;
    bool running;
    bool paused;
};