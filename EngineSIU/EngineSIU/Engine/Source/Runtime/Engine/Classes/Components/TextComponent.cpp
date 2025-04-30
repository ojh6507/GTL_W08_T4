#include "TextComponent.h"

#include "World/World.h"
#include "Editor/PropertyEditor/ShowFlags.h"
#include "UnrealEd/EditorViewportClient.h"
#include "LevelEditor/SLevelEditor.h"
#include "UObject/Casts.h"

UTextComponent::UTextComponent()
{
    SetType(StaticClass()->GetName());
}

UObject* UTextComponent::Duplicate(UObject* InOuter)
{
    ThisClass* NewComponent = Cast<ThisClass>(Super::Duplicate(InOuter));

    NewComponent->Text = Text;
    NewComponent->QuadSize = QuadSize;
    NewComponent->RowCount = RowCount;
    NewComponent->ColumnCount = ColumnCount;
    NewComponent->QuadWidth = QuadWidth;
    NewComponent->QuadHeight = QuadHeight;

    return NewComponent;
}

void UTextComponent::GetProperties(TMap<FString, FString>& OutProperties) const
{
    Super::GetProperties(OutProperties);
    
    OutProperties.Add(TEXT("Text"), FString(Text.c_str()));
    OutProperties.Add(TEXT("RowCount"), FString::Printf(TEXT("%d"), RowCount));
    OutProperties.Add(TEXT("ColumnCount"), FString::Printf(TEXT("%d"), ColumnCount));
    OutProperties.Add(TEXT("QuadWidth"), FString::Printf(TEXT("%f"), QuadWidth));
    OutProperties.Add(TEXT("QuadHeight"), FString::Printf(TEXT("%f"), QuadHeight));
    OutProperties.Add(TEXT("QuadSize"), FString::Printf(TEXT("%i"), QuadSize));
}

void UTextComponent::SetProperties(const TMap<FString, FString>& InProperties)
{
    Super::SetProperties(InProperties);
    const FString* TempStr = nullptr;
    TempStr = InProperties.Find(TEXT("Text"));
    if (TempStr)
    {
        Text = TempStr->ToWideString();
    }
    TempStr = InProperties.Find(TEXT("RowCount"));
    if (TempStr)
    {
        RowCount = FString::ToInt(*TempStr);
    }
    TempStr = InProperties.Find(TEXT("ColumnCount"));
    if (TempStr)
    {
        ColumnCount = FString::ToInt(*TempStr);
    }
    TempStr = InProperties.Find(TEXT("QuadWidth"));
    if (TempStr)
    {
        QuadWidth = FString::ToFloat(*TempStr);
    }
    TempStr = InProperties.Find(TEXT("QuadHeight"));
    if (TempStr)
    {
        QuadHeight = FString::ToFloat(*TempStr);
    }
    TempStr = InProperties.Find(TEXT("QuadSize"));
    if (TempStr)
    {
        QuadSize = FString::ToFloat(*TempStr);
    }
    
}

void UTextComponent::InitializeComponent()
{
    Super::InitializeComponent();
}

void UTextComponent::TickComponent(float DeltaTime)
{
    Super::TickComponent(DeltaTime);
}

void UTextComponent::ClearText()
{
   // vertexTextureArr.Empty();
}

void UTextComponent::SetText(const FWString& text)
{
    // 입력 문자열 검사
    if (text.length() == 0)
    {
        return;
    }

    // Text가 null인지 안전하게 확인하는 방법
    // 여기서는 사용자 정의 타입인 FWString이 null 상태를 가질 수 있다고 가정
    bool isTextNull = false;

#ifdef _DEBUG
    // 디버그 모드에서 로깅 추가
    if (&Text == nullptr)
    {
        fprintf(stderr, "Warning: Text is null in UTextComponent::SetText\n");
        isTextNull = true;
    }
#else
    // 안전한 접근을 위한 널 체크
    isTextNull = (&Text == nullptr);
#endif

    if (isTextNull)
    {
        // 엔진/프레임워크에 따라 Text 초기화 방법이 다를 수 있음
        // 여기서는 단순히 관련 코드를 실행하지 않고 반환
        return;
    }

    // Text가 null이 아니면 원래 의도대로 처리
    // Text[0] && text[0] 조건을 Text.length() > 0으로 안전하게 대체
    if (Text.length() > 0)
    {
        Text = text;
    }
  
}

void UTextComponent::SetRowColumnCount(int cellsPerRow, int cellsPerColumn)
{
    RowCount = cellsPerRow;
    ColumnCount = cellsPerColumn;
}

int UTextComponent::CheckRayIntersection(FVector& rayOrigin, FVector& rayDirection, float& pfNearHitDistance)
{
    if (!(ShowFlags::GetInstance().currentFlags & static_cast<uint64>(EEngineShowFlags::SF_BillboardText))) {
        return 0;
    }
    //TODO: quadWidth 고정으로 font사이즈 변경시 문제 발생할 수 있음
    const float quadWidth = 2.0f;
    float totalTextWidth = quadWidth * Text.size();
    float centerOffset = totalTextWidth / 2.0f;

    for (int i = 0; i < Text.size(); i++)
    {
        float offsetX = quadWidth * i - centerOffset;
        TArray<FVector> LetterQuad;
        LetterQuad.Add(FVector(-1.0f + offsetX, 1.0f, 0.0f));
        LetterQuad.Add(FVector(1.0f + offsetX, 1.0f, 0.0f));
        LetterQuad.Add(FVector(1.0f + offsetX, -1.0f, 0.0f));
        LetterQuad.Add(FVector(-1.0f + offsetX, -1.0f, 0.0f));

        float hitDistance = 0.0f;
        if (CheckPickingOnNDC(LetterQuad, hitDistance))
        {
            pfNearHitDistance = hitDistance;
            return 1;
        }
    }

    return 0;
}
