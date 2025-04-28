local function LogMessage(message)
    if Log then
        Log("[ActorLogicAnd3DGridSpawner] " .. tostring(message))
    else
        print("[ActorLogicAnd3DGridSpawner] " .. tostring(message))
    end
end

-- Variables associated with the actor RUNNING this script
local initialLocation = nil
local tickCounter = 0
local spawnedCubes = {} -- Store multiple spawned cube instances

-- ===========================================================
-- Lifecycle & Event Functions for the Actor RUNNING this script
-- ===========================================================

function BeginPlay()
    LogMessage("BeginPlay called for the script runner actor!")
    if not self then
        LogMessage("Error: 'self' is nil in BeginPlay!")
        return 
    end
    initialLocation = self:GetActorLocation()
    LogMessage(string.format("Script Runner Initial Location: %s", initialLocation:ToString()))

    local world = self:GetWorld()
    if not world then
        LogMessage("Error: Cannot get world from 'self' in BeginPlay.")
        return
    end

    -- 스폰할 클래스 찾기 (클래스 이름 확인!)
    local CubeClass = FindClass("ACube") -- 클래스 이름이 정확해야 함
    if not CubeClass then
        LogMessage("Error: Cannot find UClass for 'ACube'.")
        return
    end
    LogMessage("UClass for ACube found.")

    -- --- 3D 그리드 형태로 Cube 스폰 ---
    local gridWidth = 20      -- 그리드 X축 개수 (오른쪽 방향)
    local gridDepth = 20      -- 그리드 Y축 개수 (앞쪽 방향) - 이름 변경 (Depth)
    local gridHeight = 2      -- 그리드 Z축 개수 (위쪽 방향) - 이름 변경 (Height)
    local spacingX = 5.0    -- X축 간격
    local spacingY = 5.0    -- Y축 간격 (앞뒤)
    local spacingZ = 5.0    -- Z축 간격 (위아래)

    local forwardVec = self:GetActorForwardVector() -- 스크립트 액터의 앞 방향 (Y축 오프셋에 사용)
    local rightVec = self:GetActorRightVector()     -- 스크립트 액터의 오른쪽 방향 (X축 오프셋에 사용)
    local upVec = self:GetActorUpVector()           -- 스크립트 액터의 위쪽 방향 (Z축 오프셋에 사용)

    -- 기본 스폰 위치 (그리드의 중심 근처)
    local baseSpawnLocation = initialLocation + (forwardVec * 1.0) -- 좀 더 앞쪽에 배치

    -- 그리드 중심을 baseSpawnLocation으로 맞추기 위한 오프셋 계산
    local offsetX = -(gridWidth - 1) * spacingX / 2.0
    local offsetY = -(gridDepth - 1) * spacingY / 2.0
    local offsetZ = -(gridHeight - 1) * spacingZ / 2.0 -- Z축 오프셋 추가

    LogMessage(string.format("Attempting to spawn %dx%dx%d grid of ACube instances...", gridWidth, gridDepth, gridHeight))

    local spawnCount = 0
    -- Z축 루프 추가
    for z = 0, gridHeight - 1 do
        for y = 0, gridDepth - 1 do
            for x = 0, gridWidth - 1 do
                -- 각 큐브의 위치 계산 (3D 오프셋 적용)
                local currentOffsetX = offsetX + x * spacingX
                local currentOffsetY = offsetY + y * spacingY
                local currentOffsetZ = offsetZ + z * spacingZ -- Z 오프셋 계산
                local spawnLocation = baseSpawnLocation + (rightVec * currentOffsetX) + (forwardVec * currentOffsetY) + (upVec * currentOffsetZ) -- Z 오프셋 추가

                -- 회전은 스크립트 액터와 동일하게 설정 (또는 FRotator()로 0 설정)
                local spawnRotation = self:GetActorRotation()
                local spawnName = FName(string.format("MyLuaCube_%d_%d_%d", x, y, z)) -- 3D 좌표로 이름 지정

                -- 액터 스폰
                local spawnedActor = world:SpawnActor(CubeClass, spawnLocation, spawnRotation, spawnName)

                if spawnedActor then
                    table.insert(spawnedCubes, spawnedActor)
                    spawnCount = spawnCount + 1
                else
                    LogMessage(string.format("    Failed to spawn Cube at grid (%d, %d, %d)", x, y, z))
                end
            end
        end
    end
    LogMessage(string.format("Finished spawning grid. %d cubes attempted, %d actually spawned.",
                 gridWidth * gridDepth * gridHeight, spawnCount))
    -- --- 스폰 완료 ---
end

function EndPlay()
    LogMessage("EndPlay called for the script runner actor!")
    if self then
        local finalLocation = self:GetActorLocation()
        LogMessage(string.format("Script Runner Final Location: %s", finalLocation:ToString()))
    end

    -- 스폰된 모든 큐브 정리
    LogMessage(string.format("Cleaning up %d spawned cubes...", #spawnedCubes))
    local world = GWorld -- 전역 월드 또는 self:GetWorld() 사용
    if world then
        for i = #spawnedCubes, 1, -1 do
            local cube = spawnedCubes[i]
            if cube  and not cube:IsActorBeingDestroyed() then
                world:DestroyActor(cube)
            end
            table.remove(spawnedCubes, i)
        end
    else
         LogMessage("  Warning: Could not get world to destroy spawned actors.")
    end
    spawnedCubes = {}
    LogMessage("Cleanup complete.")
end

-- Tick 함수: 스폰된 모든 큐브에 대해 작업 수행 가능
function Tick(dt)
    if not self then return end

    tickCounter = tickCounter + 1

    -- 스폰된 모든 큐브를 회전시키는 예제 (Z축 기준)
    local rotationSpeed = 90.0 -- 초당 회전 각도

    for i, cube in ipairs(spawnedCubes) do
        if cube then
            local rot = cube:GetActorRotation()
            local loc = cube:GetActorLocation()
            loc.X = loc.X - 4 * dt;
            rot.Yaw = rot.Yaw + (rotationSpeed * dt) -- Yaw 축(Z축 기준 회전) 변경
            -- 필요하다면 다른 축도 변경: rot.Pitch = ..., rot.Roll = ...
            cube:SetActorRotation(rot)
            cube:SetActorLocation(loc)
        end
    end

    -- 로그는 가끔씩만 출력
    -- if tickCounter % 180 == 0 then
    --     LogMessage(string.format("Tick %d. Processing %d cubes.", tickCounter, #spawnedCubes))
    -- end
end

-- OnOverlap 함수: 어떤 큐브와 겹쳤는지 확인 가능
function OnOverlap(OtherActor)
    LogMessage("OnOverlap called for the script runner actor!")
    if not self or not OtherActor then return end

    local selfName = self:GetName()
    local otherName = OtherActor:GetName()
    LogMessage(string.format("'%s' overlapped with '%s'", tostring(selfName), tostring(otherName)))

    -- 겹친 액터가 스폰된 큐브 중 하나인지 확인
    for i, cube in ipairs(spawnedCubes) do
        if OtherActor == cube then
            LogMessage(string.format("Overlap detected with spawned Cube #%d (%s)!", i, tostring(otherName)))
            -- 예: 겹친 큐브 제거
            -- local world = self:GetWorld()
            -- if world then world:DestroyActor(cube) end
            -- table.remove(spawnedCubes, i) -- 테이블에서도 제거 (주의: ipairs 순회 중 제거는 문제 발생 가능)
            break
        end
    end
end