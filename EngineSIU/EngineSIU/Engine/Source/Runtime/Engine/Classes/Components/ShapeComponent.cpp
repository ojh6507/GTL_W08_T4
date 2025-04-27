#include "ShapeComponent.h"

#include "Engine/Physics/PhysicsSystem.h"
#include "UObject/Casts.h"

UShapeComponent::UShapeComponent()
{
    SetGenerateOverlapEvents(true);
}

UShapeComponent::~UShapeComponent()
{
}

void UShapeComponent::UninitializeComponent()
{
    Super::UninitializeComponent();
}

void UShapeComponent::InitializeComponent()
{
    Super::InitializeComponent();
    // TODO : 일단 테스트용 나중에 빼기
    //FPhysicsSystem::Get().RegisterComponent(this);
}

void UShapeComponent::BeginPlay()
{
    Super::BeginPlay();
    FPhysicsSystem::Get().RegisterComponent(this);
}

void UShapeComponent::TickComponent(float DeltaTime)
{
    Super::TickComponent(DeltaTime);
}

void UShapeComponent::OnComponentDestroyed()
{
    Super::OnComponentDestroyed();
}

void UShapeComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    Super::EndPlay(EndPlayReason);
    FPhysicsSystem::Get().UnRegisterComponent(this);
}

void UShapeComponent::DestroyComponent()
{
    Super::DestroyComponent();
}

UObject* UShapeComponent::Duplicate(UObject* InOuter)
{
    ThisClass* NewComponent = Cast<ThisClass>(Super::Duplicate(InOuter));
    ShapeColor = NewComponent->ShapeColor;
    bDrawOnlyIfSelected = NewComponent->bDrawOnlyIfSelected;

    return NewComponent;
}

int UShapeComponent::CheckRayIntersection(FVector& rayOrigin, FVector& rayDirection, float& pfNearHitDistance)
{
    return Super::CheckRayIntersection(rayOrigin, rayDirection, pfNearHitDistance);
}

void UShapeComponent::GetProperties(TMap<FString, FString>& OutProperties) const
{
    Super::GetProperties(OutProperties);
    OutProperties.Add(TEXT("ShapeColor"), ShapeColor.ToString());
    OutProperties.Add(TEXT("bDrawOnlyIfSelected"), bDrawOnlyIfSelected ? TEXT("true") : TEXT("false"));
}

void UShapeComponent::SetProperties(const TMap<FString, FString>& InProperties)
{
    Super::SetProperties(InProperties);
    const FString* TempStr = nullptr;
    TempStr = InProperties.Find(TEXT("bIsLoop"));
    if (TempStr)
    {
        bDrawOnlyIfSelected = (*TempStr == TEXT("true"));
    }

    auto SetPropertyHelper = [&InProperties] <typename T, typename Fn>(const FString& Key, T& MemberVariable, const Fn& ConversionFunc)
    {
        if (const FString* TempStr = InProperties.Find(Key))
        {
            MemberVariable = ConversionFunc(*TempStr);
        }
    };

    SetPropertyHelper(TEXT("ShapeColor"), ShapeColor, [](const FString& Str) { FLinearColor Color; Color.InitFromString(Str); return Color; });
}

bool UShapeComponent::CheckOverlapComponent(UShapeComponent* Other, FHitResult& OutHitResult)
{
    return true;
}
