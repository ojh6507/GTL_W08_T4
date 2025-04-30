function BeginPlay()

end

function EndPlay()
  
end

function OnOverlap(OtherActor)
    
end


local hasShakeBeenTriggered = false
function Tick(dt)
    if hasShakeBeenTriggered then
        return
    end
    
    local world = self:GetWorld()
    local PlayerCameraManager = world:GetPlayerCameraManager()

    if PlayerCameraManager then
        local moveModifier = PlayerCameraManager:GetMoveModifier()
        
        local first = world:GetViewTarget(FName("1"))
        local second = world:GetViewTarget(FName"MainCamera")
        local third = world:GetViewTarget(FName("3"))
        local forth = world:GetViewTarget(FName"4")
        
        moveModifier:Initialize(first, second, 2)

        Delay(2, function()
            moveModifier:Initialize(second, third, 2)

        end)

        Delay(4, function()
            moveModifier:Initialize(third, forth, 2)

        end)
        hasShakeBeenTriggered = true
    
    end
end