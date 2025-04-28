#include "Timer.h"

Timer::Timer()
    : currentTime(0.0f)
    , running(false)
    , paused(false)
{
}

Timer::~Timer()
{
}

void Timer::Start()
{
    running = true;
    paused = false;
    currentTime = 0.0f;
}

void Timer::Stop()
{
    running = false;
    paused = false;
}

void Timer::Reset()
{
    currentTime = 0.0f;
}

void Timer::Pause()
{
    if (running)
        paused = true;
}

void Timer::Resume()
{
    paused = false;
}

bool Timer::IsRunning() const
{
    return running;
}

bool Timer::IsPaused() const
{
    return paused;
}

float Timer::GetTime() const
{
    return currentTime;
}

void Timer::Update(float deltaTime)
{
    if (running && !paused)
    {
        currentTime += deltaTime;
    }
}