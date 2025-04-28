local GameManager = {}
local isGameStarted = false
local MAX_LIFE =10

function GameManager.StartGame()
    isGameStarted = true
    Timer:Start()
    SoundManager:PlaySound("Main")
end

function GameManager.PauseGame()
    if isGameStarted then
        Timer:Pause()
    end
end

function GameManager.ResumeGame()
    if isGameStarted then
        Timer:Resume()
    end
end

function GameManager.LoseLife()
    if isGameStarted then
        gameUI:LoseLife()
        
        if gameUI:GetLives() <= 0 then
            Timer:Pause()
        end
    end
end

function GameManager.ResetGame()
    if isGameStarted then
        Timer:Stop()
        Timer:Reset()
        SoundManager:Stop("Main")
        gameUI:ResetLives(MAX_LIFE)
        isGameStarted = false
    end
end

function EndPlay()
    if SoundManager then
        isGameStarted = false
        SoundManager:ShutDown()
    end
end

function BeginPlay()
    gameUI:ResetLives(MAX_LIFE)
    SoundManager:Initialize()
    SoundManager:LoadSoundFiles()
    if gameUI then
        gameUI:SetStartButtonCallback(GameManager.StartGame)
        gameUI:SetPauseButtonCallback(GameManager.PauseGame)
        gameUI:SetResumeButtonCallback(GameManager.ResumeGame)
        gameUI:SetResetButtonCallback(GameManager.ResetGame)
        gameUI:SetDamageEventCallback(GameManager.LoseLife)
    end
end

function Tick(dt)
    if isGameStarted and gameUI then
        gameUI:Update(dt)
        
    end
end
