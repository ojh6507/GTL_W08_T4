local soundManager

function BeginPlay()
    soundManager = GetSoundManager()

    -- 초기화
    soundManager:Initialize() 
  

    soundManager:LoadSound("Main", "Main.mp3", true) 


    soundManager:PlaySound("Main")
end



function EndPlay()
    Log("End")
    soundManager:ShutDown()
end

function Tick(dt)
  
end