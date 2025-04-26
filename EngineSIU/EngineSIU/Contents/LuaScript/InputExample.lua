function BeginPlay()
    
end

function EndPlay()
  
end

function OnOverlap(OtherActor)
    
end

function Tick(dt)
  
    if IsKeyDown(KEY_LEFT) then
        Log("왼쪽")
    end

    if IsKeyDown(KEY_UP) then
        Log("위")
    end

    if IsKeyDown(KEY_RIGHT) then
        Log("오른쪽")
    end

    if IsKeyDown(KEY_DOWN) then
        Log("아래")
    end

    
    if IsKeyDown(KEY_A) then
        Log("왼쪽")
    end

    if IsKeyDown(KEY_W) then
        Log("위")
    end

    if IsKeyDown(KEY_D) then
        Log("오른쪽")
    end

    if IsKeyDown(KEY_S) then
        Log("아래")
    end


end