local textComp 
function BeginPlay()
    textComp = self:GetTextComponent() -- TextComponent 인스턴스 얻기
    local text1 = "무단 외출해야지"
    if textComp then
        textComp:SetText(text1)
    end

end

function EndPlay()
  
end

function OnOverlap(OtherActor)
    
end

local PlayerCameraManager = nil
local IsWorldInit = false
function Tick(dt)

    if not PlayerCameraManager then
        local world = self:GetWorld()
       PlayerCameraManager = world:GetPlayerCameraManager()
       PlayerCameraManager:SetActiveLetterBox(true)
       IsWorldInit = true

       local moveModifier = PlayerCameraManager:GetMoveModifier()

       local first = world:GetViewTarget(FName("StartCamera"))
       local Main = world:GetViewTarget(FName("MainCamera"))
       local DAngry = world:GetViewTarget(FName("DAngry"))
       local ESHappy = world:GetViewTarget(FName("ESHappy"))
       local GetES = world:GetViewTarget(FName("GetES"))
       local RunJ = world:GetViewTarget(FName("RunJ"))

       moveModifier:Initialize(first, ESHappy, 3)
       SoundManager:PlaySound("ESHappy")
        local text1 = "크크크딱걸렸다"
        local text2 = "도라에몽 미안해"
        local test3 = "내가 구해줄게 이슬아"
       Delay(4.5, function()
        SoundManager:Stop("ESHappy")
        SoundManager:PlaySound("Angry")
        moveModifier:Initialize(ESHappy, DAngry, 2)
        textComp = self:GetTextComponent() -- TextComponent 인스턴스 얻기
   
        if textComp then
        textComp:SetText(text1)
        end
    end)


    Delay(9, function()
        moveModifier:Initialize(DAngry, GetES, 1)
        textComp = self:GetTextComponent() -- TextComponent 인스턴스 얻기
        SoundManager:PlaySound("ESScream")
        if textComp then
        textComp:SetText(text2)
        local FadeColor = FLinearColor(0,0,0,1)           
        PlayerCameraManager:StartCameraFade(1, 0, 5.5, FadeColor, false, false)

        end
    end)

    
    Delay(13, function()
        moveModifier:Initialize(GetES, RunJ, 1)
        textComp = self:GetTextComponent() -- TextComponent 인스턴스 얻기
   
        if textComp then
        textComp:SetText(test3)
        local FadeColor = FLinearColor(0,0,0,1)           
        PlayerCameraManager:StartCameraFade(0, 1, 4.8, FadeColor, false, false)
        
        end
    end)
   

    Delay(16, function()
        moveModifier:Initialize(RunJ, Main, 1)
        textComp = self:GetTextComponent() -- TextComponent 인스턴스 얻기
        if textComp then
        textComp:SetText("")

        PlayerCameraManager:SetActiveLetterBox(false)
        end
    end)

    end
   
end