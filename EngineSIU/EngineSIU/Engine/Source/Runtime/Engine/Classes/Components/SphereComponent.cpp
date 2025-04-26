#include "SphereComponent.h"

#include "BoxComponent.h"
#include "CapsuleComponent.h"
#include "Engine/Physics/CollisionDispatcher.h"
#include "Runtime/Core/Math/JungleMath.h"
#include "UnrealEd/EditorViewportClient.h"
#include "UObject/Casts.h"

USphereComponent::USphereComponent()
{
    SetType(StaticClass()->GetName());
}

USphereComponent::~USphereComponent()
{
}

void USphereComponent::Serialize(FArchive& Ar)
{
    Super::Serialize(Ar);
}

void USphereComponent::UninitializeComponent()
{
    Super::UninitializeComponent();
}

void USphereComponent::BeginPlay()
{
    Super::BeginPlay();
}

void USphereComponent::OnComponentDestroyed()
{
    Super::OnComponentDestroyed();
}

void USphereComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    Super::EndPlay(EndPlayReason);
}

void USphereComponent::DestroyComponent()
{
    Super::DestroyComponent();
}

UObject* USphereComponent::Duplicate(UObject* InOuter)
{
    return Super::Duplicate(InOuter);
}

int USphereComponent::CheckRayIntersection(FVector& rayOrigin, FVector& rayDirection, float& pfNearHitDistance)
{
    return Super::CheckRayIntersection(rayOrigin, rayDirection, pfNearHitDistance);
}

void USphereComponent::GetProperties(TMap<FString, FString>& OutProperties) const
{
    Super::GetProperties(OutProperties);
}

void USphereComponent::SetProperties(const TMap<FString, FString>& InProperties)
{
    Super::SetProperties(InProperties);
}

bool USphereComponent::CheckOverlapComponent(UShapeComponent* Other, FHitResult& OutHitResult)
{
    if (UBoxComponent* OtherBox = Cast<UBoxComponent>(Other))
    {
        return FCollisionDispatcher::OverlapSphereToBox(this, OtherBox, OutHitResult);
    }

    if (USphereComponent* OtherSphere = Cast<USphereComponent>(Other))
    {
        return FCollisionDispatcher::OverlapSphereToSphere(this, OtherSphere, OutHitResult);
    }

    if (UCapsuleComponent* OtherCapsule = Cast<UCapsuleComponent>(Other))
    {
        return FCollisionDispatcher::OverlapSphereToCapsule(this, OtherCapsule, OutHitResult);
    }

    return false;
}

void USphereComponent::InitializeComponent()
{
    Super::InitializeComponent();
}

void USphereComponent::TickComponent(float DeltaTime)
{
    Super::TickComponent(DeltaTime);
}
