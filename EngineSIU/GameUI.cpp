
#include "GameUI.h"
#include "Timer.h"
#include "SoundManager.h"
#include "World/World.h"
#include "Engine/Engine.h"
#include <cstdio>

GameUI::GameUI()
    : initialized(false)
    , gameTimer(nullptr)
{
    Initialize();
}

GameUI::~GameUI()
{
    Shutdown();
}

void GameUI::Initialize()
{
    if (initialized)
        return;

    // 타이머 객체 생성
    gameTimer = std::make_unique<Timer>();

    initialized = true;
}

void GameUI::Shutdown()
{
    if (!initialized)
        return;
  
    // 타이머 정지
    if (gameTimer)
    {
        gameTimer->Stop();
    }

    initialized = false;
}

void GameUI::Update(float deltaTime)
{
    // 타이머 업데이트
    if (gameTimer)
    {
        gameTimer->Update(deltaTime);
    }
}

void GameUI::OnResize(HWND hWnd)
{
    RECT ClientRect;
    GetClientRect(hWnd, &ClientRect);
    Width = ClientRect.right - ClientRect.left;
    Height = ClientRect.bottom - ClientRect.top;
}

void GameUI::Render()
{
    if (!initialized)
        return;

    // 타이머 UI 렌더링
    RenderTimerUI();

    // 추가 UI 요소 렌더링...
}

void GameUI::RenderTimerUI()
{
    if (!gameTimer)
        return;
    if (ImGui::Begin("Game Timer"))
    {
        // 상태에 따른 UI 렌더링
        if (lives <= 0)
        {
            // 게임 오버 상태
            std::string timeStr = FormatTime(gameTimer->GetTime());
            ImGui::Text("Game Over!");
            ImGui::Text("Score Time: %s", timeStr.c_str());

            if (ImGui::Button("Restart", ImVec2(200, 30)))
            {
                if (resetButtonCallback)
                {
                    resetButtonCallback();
                }
            }
        }
        else
        {
            // 시간 표시
            std::string timeStr = FormatTime(gameTimer->GetTime());
            ImGui::Text("Time: %s", timeStr.c_str());

            // 목숨 표시 추가
            ImGui::Text("Lives: %d", lives);

            // 상태에 따른 버튼 렌더링
            if (!gameTimer->IsRunning())
            {
                // 게임 시작 전
                if (ImGui::Button("Start", ImVec2(200, 30)))
                {
                    if (startButtonCallback)
                    {
                        startButtonCallback();
                    }
                }
            }
        
        }
    }
    ImGui::End();
}

Timer* GameUI::GetTimer()
{
    return gameTimer.get();
}

std::string GameUI::FormatTime(float timeInSeconds)
{
    int minutes = static_cast<int>(timeInSeconds) / 60;
    int seconds = static_cast<int>(timeInSeconds) % 60;
    int milliseconds = static_cast<int>((timeInSeconds - static_cast<int>(timeInSeconds)) * 100);

    std::ostringstream oss;
    oss << std::setfill('0')
        << std::setw(2) << minutes << ":"
        << std::setw(2) << seconds << "."
        << std::setw(2) << milliseconds;

    return oss.str();
}