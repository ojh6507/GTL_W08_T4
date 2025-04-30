local initialLocation = nil
local moveSpeed = 5.0
local PlayerCameraManager = nil
function BeginPlay()
    Log("[Lua] BeginPlay called!")
    if self then
        local loc = self:GetActorLocation()
        initialLocation = loc -- Store initial location if needed
        Log(string.format("[Lua] Actor Initial Location: x=%.2f, y=%.2f, z=%.2f", loc.X, loc.Y, loc.Z))

        -- Test setting location (optional)
        -- local newLoc = FVector.New(loc.X + 100, loc.Y, loc.Z) -- Assuming FVector binding has a constructor
        -- Log("[Lua] Setting new location...")
        -- self:SetActorLocation(newLoc)
    else
        Log("[Lua] Error: Owning actor is nil in BeginPlay!")
    end
end

function EndPlay()
    Log("[Lua] EndPlay called!")
    if actor then
        local loc = self:GetActorLocation()
        Log(string.format("[Lua] Actor Final Location: x=%.2f, y=%.2f, z=%.2f", loc.X, loc.Y, loc.Z))
    end
end

function OnOverlap(OtherActor)
    Log("[Lua] OnOverlap called!")

    if OtherActor then
        gameUI:LoseLife()
        SoundManager:PlaySound("Hit")
         local otherLoc = OtherActor:GetActorLocation()
         if PlayerCameraManager == nil then 
            local world = self:GetWorld()
            PlayerCameraManager = world:GetPlayerCameraManager()
        end
        if PlayerCameraManager then
            local FadeColor = FLinearColor(0.5,0,0,1)
            PlayerCameraManager:StartCameraFade(0, 1, 1.2,FadeColor, false, false)
             

            local shakeModifier = PlayerCameraManager:GetShakeModifier()
            shakeModifier:StartShake(0.02, 1)
        end
    else
       
        Log("[Lua] Overlapped with nil Actor?")
    end
end

local tickCounter = 0
-- C++에서 전달하는 인자 순서에 맞게 파라미터 수정: (액터 객체, 델타 타임)
function Tick(dt)

    if not Timer or not Timer:IsRunning() then
        return
    end

    tickCounter = tickCounter + 1
    -- dt는 이제 두 번째 파라미터로 올바르게 float 값을 받음
    -- Log(string.format("[Lua] Tick called! DeltaTime: %.4f", dt)) -- 필요하다면 로그 출력

    -- 전역 'actor' 대신 파라미터 'self'를 사용
    if self then
        local currentLocation = self:GetActorLocation()
        local moveDelta = FVector(0, 0, 0) -- FVector 생성자 사용
        local rotDelta = FRotator(0, 0, 0)
    
        rotDelta.Yaw = 0 -- Yaw 회전 초기화
        if IsKeyDown(KEY_RIGHT) then
            moveDelta.Y = moveDelta.Y + moveSpeed * dt
            rotDelta.Yaw = rotDelta.Yaw + 1000 * dt
        end
        if IsKeyDown(KEY_LEFT) then
            moveDelta.Y = moveDelta.Y - moveSpeed * dt
            rotDelta.Yaw = rotDelta.Yaw - 1000 * dt
        end

        if moveDelta:LengthSquared() > 0 then
            local tempLocation = currentLocation + moveDelta
            -- Y 축 이동 제한 (-20 ~ 20)
            if tempLocation.Y < -5 then
                tempLocation.Y = -5
            elseif tempLocation.Y > 5 then
                tempLocation.Y = 5
            end
            self:SetActorLocation(tempLocation)
       end
       self:SetActorRotation(rotDelta)
    else
        -- self가 nil인 경우는 C++ 호출부에서 문제가 없는 한 발생하기 어려움
        Log("[Lua] Error: self is nil in Tick!")
    end
end