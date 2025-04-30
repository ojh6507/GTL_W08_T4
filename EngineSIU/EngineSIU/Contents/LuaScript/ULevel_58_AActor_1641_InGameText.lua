function BeginPlay()
    local textComp = self:GetTextComponent() -- TextComponent 인스턴스 얻기

    if textComp then
        textComp:SetText("루아에서 됐다")
        print("한글 텍스트 설정 완료.")
    else
     print("TextComponent를 찾을 수 없습니다.")
    end
end

function EndPlay()
  
end

function OnOverlap(OtherActor)
    
end

function Tick(dt)

end