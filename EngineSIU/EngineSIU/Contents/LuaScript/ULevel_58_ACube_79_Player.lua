local initialLocation = nil

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
        Log("[Lua] Overlapped with self: " .. Otherself:GetName():ToString())
         local otherLoc = Otherself:GetActorLocation()
         Log(string.format("[Lua] Other Actor Location: x=%.2f, y=%.2f, z=%.2f", otherLoc.X, otherLoc.Y, otherLoc.Z))
    else
        Log("[Lua] Overlapped with nil Actor?")
    end
end

local tickCounter = 0
-- C++에서 전달하는 인자 순서에 맞게 파라미터 수정: (액터 객체, 델타 타임)
function Tick(dt)
    tickCounter = tickCounter + 1
    -- dt는 이제 두 번째 파라미터로 올바르게 float 값을 받음
    -- Log(string.format("[Lua] Tick called! DeltaTime: %.4f", dt)) -- 필요하다면 로그 출력

    -- 전역 'actor' 대신 파라미터 'self'를 사용
    if self then
        -- self 변수가 이제 유효하므로 메서드 호출 가능
        local loc = self:GetActorLocation()
        if loc then -- GetActorLocation 이 유효한 FVector를 반환했는지 확인 (선택적이지만 안전)
            loc.X = loc.X + (1 * dt)
            self:SetActorLocation(loc) -- 수정된 위치 설정
        end

        local rot = self:GetActorRotation()
        if rot then -- GetActorRotation 이 유효한 FRotator를 반환했는지 확인
            rot.Yaw = rot.Yaw + (100 * dt)
            self:SetActorRotation(rot) -- 수정된 회전 설정
        end
    else
        -- self가 nil인 경우는 C++ 호출부에서 문제가 없는 한 발생하기 어려움
        Log("[Lua] Error: self is nil in Tick!")
    end
end