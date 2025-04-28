local velocity
local maxSpeed
local accelerationRate
local friction


function BeginPlay()
    velocity = FVector(0,0,0)
    maxSpeed = 100
    accelerationRate = 100
    friction = 60
end

function EndPlay()
  
end

function OnOverlap(OtherActor)
    
end


function Tick(dt)
    -- 1) 입력 방향 구하기
    local inputDir = FVector(0, 0, 0)
    if IsKeyDown(KEY_W) then inputDir = inputDir + self:GetActorForwardVector() end
    if IsKeyDown(KEY_S) then inputDir = inputDir - self:GetActorForwardVector() end
    if IsKeyDown(KEY_D) then inputDir = inputDir + self:GetActorRightVector() end
    if IsKeyDown(KEY_A) then inputDir = inputDir - self:GetActorRightVector() end

    if inputDir:LengthSquared() > 0 then
        inputDir = inputDir:GetSafeNormal(0.001)
        -- 2) 가속도 적용w
        local accel = inputDir * accelerationRate
        velocity = velocity + accel * dt
    else
        -- 3) 마찰 적용
        local speed = velocity:Length()
        if speed > 0 then
            local brake = math.min(speed, friction * dt)
            velocity = velocity - velocity:GetSafeNormal(0.0001) * brake
        end
    end

   
    -- 5) 위치 갱신
    local newLoc = self:GetActorLocation() + velocity * dt
    self:SetActorLocation(newLoc)
end