#include "BoxComponent.h"

#include "CapsuleComponent.h"
#include "SphereComponent.h"
#include "Engine/Physics/CollisionDispatcher.h"
#include "Engine/Physics/PhysicsSystem.h"
#include "GameFramework/Actor.h"
#include "Math/JungleMath.h"
#include "UObject/Casts.h"

UBoxComponent::UBoxComponent()
{
    SetType(StaticClass()->GetName());
    BoxExtent = FVector(1, 1, 1);
    // Test용 하드 코딩
    FVector LocalMin = -BoxExtent;  // (-X, -Y, -Z)
    FVector LocalMax =  BoxExtent;  // ( +X, +Y, +Z)
    AABB.max = LocalMax;
    AABB.min = LocalMin;

    
}

void UBoxComponent::InitializeComponent()
{
    Super::InitializeComponent();
}

void UBoxComponent::TickComponent(float DeltaTime)
{
    Super::TickComponent(DeltaTime);
}

UBoxComponent::~UBoxComponent()
{
}

void UBoxComponent::UninitializeComponent()
{
    Super::UninitializeComponent();
}

void UBoxComponent::BeginPlay()
{
    Super::BeginPlay();
    OnComponentBeginOverlap.AddDynamic(this, &UBoxComponent::HandleBeginOverlap);
}

void UBoxComponent::OnComponentDestroyed()
{
    Super::OnComponentDestroyed();
}

void UBoxComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    Super::EndPlay(EndPlayReason);
}

void UBoxComponent::DestroyComponent()
{
    Super::DestroyComponent();
}

UObject* UBoxComponent::Duplicate(UObject* InOuter)
{
    ThisClass* NewComponent = Cast<ThisClass>(Super::Duplicate(InOuter));
    BoxExtent = NewComponent->BoxExtent;

    return NewComponent;
}

int UBoxComponent::CheckRayIntersection(FVector& rayOrigin, FVector& rayDirection, float& pfNearHitDistance)
{
    return Super::CheckRayIntersection(rayOrigin, rayDirection, pfNearHitDistance);
}

void UBoxComponent::GetProperties(TMap<FString, FString>& OutProperties) const
{
    Super::GetProperties(OutProperties);
    OutProperties.Add(TEXT("BoxExtent"), BoxExtent.ToString());
}

void UBoxComponent::SetProperties(const TMap<FString, FString>& InProperties)
{
    Super::SetProperties(InProperties);
    const FString* TempStr = nullptr;
    TempStr = InProperties.Find(TEXT("BoxExtent"));
    if (TempStr)
    {
        BoxExtent.InitFromString(*TempStr);
    }
}

bool UBoxComponent::CheckOverlapComponent(UShapeComponent* Other, FHitResult& OutHitResult)
{
    if (UBoxComponent* OtherBox = Cast<UBoxComponent>(Other))
    {
        return FCollisionDispatcher::OverlapBoxToBox(this, OtherBox, OutHitResult);
    }

    if (USphereComponent* OtherSphere = Cast<USphereComponent>(Other))
    {
        return FCollisionDispatcher::OverlapBoxToSphere(this, OtherSphere, OutHitResult);
    }

    if (UCapsuleComponent* OtherCapsule = Cast<UCapsuleComponent>(Other))
    {
        return FCollisionDispatcher::OverlapBoxToCapsule(this, OtherCapsule, OutHitResult);
    }

    return false;
}

void UBoxComponent::HandleBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, bool bFromSweep,
    const FHitResult& SweepResult)
{
    UE_LOG(ELogLevel::Display, TEXT("[Overlap] %s ↔ %s at %s"), *OverlappedComp->GetName(), *OtherActor->GetName(), *SweepResult.ImpactPoint.ToString());
}
