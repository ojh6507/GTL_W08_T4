local cameraMgr = nil
local isShakeStarted = false

function BeginPlay()
    
end

function EndPlay()
  
end

function OnOverlap(OtherActor)
    
end

function Tick(dt)
    if not cameraMgr then
        local world = self:GetWorld()
        cameraMgr = world:GetPlayerCameraManager()
    end

    
    if cameraMgr and not isShakeStarted then
        local shakeModifier = cameraMgr:GetShakeModifier()
        if shakeModifier then
            shakeModifier:StartShake(.2, 3.0)
            isShakeStarted = true
        end
    end
    
end