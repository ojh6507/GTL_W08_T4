local soundManager

function Initialize()
    soundManager = GetSoundManager()

    soundManager:Initialize() 
    soundManager:LoadSound("Main", "Main.mp3", true) 
end

function PlayBGM()
    soundManager:PlaySound("Main")
end



function EndPlay()
    soundManager:ShutDown()
end

function Tick(dt)
  
end