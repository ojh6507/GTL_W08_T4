local GameManager = {}

function GameManager.StartGame()
   
    Timer:Start()

    SoundManager:PlaySound("Main")
   
end

function GameManager.PauseGame()
    Timer:Pause()
end

function GameManager.ResumeGame()
    if isGameStarted then
        if gameUI then
            local timer = gameUI:GetTimer()
            if timer then
                timer:Resume()
            end
            
            -- 필요한 경우 사운드 재개
        end
    end
end

function GameManager.ResetGame()
  
    Timer:Stop()
    Timer:Reset()
    SoundManager:Stop("Main")
  
end

function EndPlay()
    SoundManager:ShutDown()
end

function BeginPlay()
    SoundManager:Initialize()
    SoundManager:LoadSoundFiles()
    if gameUI then
        gameUI:SetStartButtonCallback(GameManager.StartGame)
        gameUI:SetPauseButtonCallback(GameManager.PauseGame)
        gameUI:SetResumeButtonCallback(GameManager.ResumeGame)
        gameUI:SetResetButtonCallback(GameManager.ResetGame)
    end
end

function Tick(dt)
        gameUI:Update(dt)
end

return GameManager