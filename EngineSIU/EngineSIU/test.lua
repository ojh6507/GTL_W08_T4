-- test.lua
function BeginPlay()
    Log("BeginPlay 호출됨! Actor 초기화")
end

function Tick(dt)
    -- 간단한 움직임 예제
    local pos = Vector3.new(0, 0, 0)
    pos.x = pos.x + dt
    SetActorLocation(obj, pos.x, pos.y, pos.z)
end

function OnOverlap(other)
    Log("다른 액터와 충돌했습니다!")
end

function EndPlay()
    Log("EndPlay 호출됨! Actor 제거")
end

Log("스크립트 로드 완료!")
