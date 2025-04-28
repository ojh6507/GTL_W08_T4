local function LogMessage(message)
    if Log then
        Log("[ActorSpawner] " .. tostring(message))
    else
        print("[ActorSpawner] " .. tostring(message))
    end
end

-- Variables associated with the actor RUNNING this script
local initialLocation = nil
local tickCounter = 0
local spawnedObjs = {} -- Store multiple spawned cube instances
local spawnedDeltaTime = 0.0
local spawnInterval = 1.0 -- Spawn interval in seconds
local spawnCount = 0

-- ===========================================================
-- Lifecycle & Event Functions for the Actor RUNNING this script
-- ===========================================================

function BeginPlay()
    LogMessage("[BeginPlay] BeginPlay called for the script runner actor!")
    if not self then
        LogMessage("[BeginPlay] Error: 'self' is nil in BeginPlay!")
        return 
    end
    initialLocation = FVector(100, 0, 1) -- 초기 위치 설정 (필요시 수정)
    LogMessage(string.format("[BeginPlay] Script Runner Initial Location: %s", initialLocation:ToString()))

    local world = self:GetWorld()
    if not world then
        LogMessage("[BeginPlay] Error: Cannot get world from 'self' in BeginPlay.")
        return
    end

    LogMessage("[BeginPlay] Script Runner Actor BeginPlay completed.")
end

function EndPlay()
    LogMessage("[EndPlay] EndPlay called for the script runner actor!")
    if self then
        local finalLocation = self:GetActorLocation()
        LogMessage(string.format("[EndPlay] Script Runner Final Location: %s", finalLocation:ToString()))
    end

    -- 스폰된 모든 큐브 정리
    LogMessage(string.format("[EndPlay] Cleaning up %d spawned cubes...", #spawnedObjs))
    local world = GWorld -- 전역 월드 또는 self:GetWorld() 사용
    if world then
        for i = #spawnedObjs, 1, -1 do
            local cube = spawnedObjs[i]
            if cube  and not cube:IsActorBeingDestroyed() then
                world:DestroyActor(cube)
            end
            table.remove(spawnedObjs, i)
        end
    else
         LogMessage("[EndPlay] Warning: Could not get world to destroy spawned actors.")
    end
    spawnedObjs = {}
    LogMessage("[EndPlay] Cleanup complete.")
end

-- Tick 함수: 스폰된 모든 큐브에 대해 작업 수행 가능
function Tick(dt)
    if not self then
        return
    end

    if not Timer or not Timer:IsRunning() then
        return
    end
    
    local world = self:GetWorld()
    tickCounter = tickCounter + 1
    spawnedDeltaTime = spawnedDeltaTime + dt
    
    --LogMessage(string.format("[Tick] [%d] sdt: %.2f | si: %.2f", tickCounter, spawnedDeltaTime, spawnInterval))
    -- 주기적으로 큐브를 스폰함
    if spawnedDeltaTime >= spawnInterval then
        spawnedDeltaTime = 0.0 -- 리셋
        if world then
            local spawnLocation = initialLocation + FVector(0, math.random(-5, 5), 0) -- Y축 방향으로 랜덤 스폰
            local spawnRotation = FRotator(0, 0, 0) -- 회전 없음
            local spawnName = FName(string.format("MyLuaCube_%d", spawnCount))

            -- 큐브 스폰
            local CubeClass = FindClass("ACube")
            if CubeClass then
                local spawnedActor = world:SpawnActor(CubeClass, spawnLocation, spawnRotation, spawnName)
                if spawnedActor then
                    table.insert(spawnedObjs, spawnedActor)
                    spawnCount = spawnCount + 1
                    if spawnInterval > 0.5 then
                        spawnInterval = spawnInterval - 0.02 -- 스폰 간격 줄이기
                    end
                end
            end
        end
    end

    -- 스폰된 모든 큐브를 회전 및 이동
    local rotationSpeed = 90.0 -- 초당 회전 각도
    local moveSpeed = 10.0 -- 초당 이동 거리
    local moveDirection = FVector(-1, 0, 0) -- -X축 방향으로 이동

    for i, cube in ipairs(spawnedObjs) do
        if cube then
            local rot = cube:GetActorRotation()
            rot.Yaw = rot.Yaw + (rotationSpeed * dt)
            cube:SetActorRotation(rot)
            local loc = cube:GetActorLocation()
            loc = loc + (moveDirection * moveSpeed * dt)
            cube:SetActorLocation(loc)
        end
    end

    -- 큐브가 월드 경계를 넘어가면 제거
    local xConstraint = -10.0 -- X축 경계
    for i, cube in ipairs(spawnedObjs) do
        if cube then
            local loc = cube:GetActorLocation()
            if loc.X < xConstraint then
                if cube  and not cube:IsActorBeingDestroyed() then
                    world:DestroyActor(cube)
                end
                table.remove(spawnedObjs, i)
            end
        end
    end
end

-- OnOverlap 함수: 어떤 큐브와 겹쳤는지 확인 가능
function OnOverlap(OtherActor)
    LogMessage("OnOverlap called for the script runner actor!")
    if not self or not OtherActor then return end

    local selfName = self:GetName()
    local otherName = OtherActor:GetName()
    LogMessage(string.format("'%s' overlapped with '%s'", tostring(selfName), tostring(otherName)))

    -- 겹친 액터가 스폰된 큐브 중 하나인지 확인
    for i, cube in ipairs(spawnedObjs) do
        if OtherActor == cube then
            LogMessage(string.format("Overlap detected with spawned Cube #%d (%s)!", i, tostring(otherName)))
            -- 예: 겹친 큐브 제거
            -- local world = self:GetWorld()
            -- if world then world:DestroyActor(cube) end
            -- table.remove(spawnedObjs, i) -- 테이블에서도 제거 (주의: ipairs 순회 중 제거는 문제 발생 가능)
            break
        end
    end
end