#include "GameUI.h"
#include "Timer.h"
#include "SoundManager.h"
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
        // 시간 표시
        std::string timeStr = FormatTime(gameTimer->GetTime());
        ImGui::Text("Time: %s", timeStr.c_str());

        // 상태에 따른 버튼 렌더링
        if (!gameTimer->IsRunning())
        {
            // 게임 시작 전 또는 게임 종료 후
            if (ImGui::Button("Start", ImVec2(200, 30)))
            {
                if (startButtonCallback)
                {
                    startButtonCallback();
                }
            }
        }
        else
        {
            // 게임 진행 중에는 버튼 숨김
            ImGui::Text("Game in Progress...");
        }

        // 게임 종료 여부를 확인하는 로직 필요 (예: 타이머의 최대 시간 도달 등)
        // 예시로 60초 지나면 게임 종료로 간주
        if (gameTimer->GetTime() >= 10.f)
        {
            if (ImGui::Button("Restart", ImVec2(200, 30)))
            {
                if (resetButtonCallback)
                {
                    resetButtonCallback();
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