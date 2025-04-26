#include "ShapeComponent.h"

#include "Engine/Physics/PhysicsSystem.h"

UShapeComponent::UShapeComponent()
{
}

UShapeComponent::~UShapeComponent()
{
}

void UShapeComponent::Serialize(FArchive& Ar)
{
    Super::Serialize(Ar);
}

void UShapeComponent::UninitializeComponent()
{
    Super::UninitializeComponent();
}

void UShapeComponent::InitializeComponent()
{
    Super::InitializeComponent();
    // TODO : 일단 테스트용 나중에 빼기
    FPhysicsSystem::Get().RegisterComponent(this);
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
    return Super::Duplicate(InOuter);
}

int UShapeComponent::CheckRayIntersection(FVector& rayOrigin, FVector& rayDirection, float& pfNearHitDistance)
{
    return Super::CheckRayIntersection(rayOrigin, rayDirection, pfNearHitDistance);
}

void UShapeComponent::GetProperties(TMap<FString, FString>& OutProperties) const
{
    Super::GetProperties(OutProperties);
}

void UShapeComponent::SetProperties(const TMap<FString, FString>& InProperties)
{
    Super::SetProperties(InProperties);
}

bool UShapeComponent::CheckOverlapComponent(UShapeComponent* Other, FHitResult& OutHitResult)
{
    return true;
}
