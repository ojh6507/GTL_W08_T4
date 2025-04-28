local GameManager = {}
local isGameStarted = false
local MAX_LIFE =2


function GameManager.IsGameStarted()
    return isGameStarted
end

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



function GameManager.ResetGame()
    if isGameStarted then
        Timer:Stop()
        Timer:Reset()
        SoundManager:Stop("Main")
        gameUI:ResetLives(MAX_LIFE)
        isGameStarted = false
        ControlEditorPanelInstance:EndPIE()
        ControlEditorPanelInstance:StartPIE()

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
    end
end

function Tick(dt)


    if isGameStarted and gameUI then
        gameUI:Update(dt)
        
    end

     if gameUI:GetLives() <= 0 then
            Timer:Pause()
     end
end

return GameManager

