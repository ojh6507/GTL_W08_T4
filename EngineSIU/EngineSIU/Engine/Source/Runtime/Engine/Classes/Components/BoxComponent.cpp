#include "BoxComponent.h"

#include "CapsuleComponent.h"
#include "SphereComponent.h"
#include "Engine/Physics/CollisionDispatcher.h"
#include "Engine/Physics/PhysicsSystem.h"
#include "Math/JungleMath.h"
#include "UObject/Casts.h"

UBoxComponent::UBoxComponent()
{
    SetType(StaticClass()->GetName());

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

void UBoxComponent::Serialize(FArchive& Ar)
{
    Super::Serialize(Ar);
}

void UBoxComponent::UninitializeComponent()
{
    Super::UninitializeComponent();
}

void UBoxComponent::BeginPlay()
{
    Super::BeginPlay();
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
    return Super::Duplicate(InOuter);
}

int UBoxComponent::CheckRayIntersection(FVector& rayOrigin, FVector& rayDirection, float& pfNearHitDistance)
{
    return Super::CheckRayIntersection(rayOrigin, rayDirection, pfNearHitDistance);
}

void UBoxComponent::GetProperties(TMap<FString, FString>& OutProperties) const
{
    Super::GetProperties(OutProperties);
}

void UBoxComponent::SetProperties(const TMap<FString, FString>& InProperties)
{
    Super::SetProperties(InProperties);
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
