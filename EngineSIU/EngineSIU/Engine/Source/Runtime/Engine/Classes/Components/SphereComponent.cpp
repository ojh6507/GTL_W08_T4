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
    UShapeComponent::Serialize(Ar);
}

void USphereComponent::UninitializeComponent()
{
    UShapeComponent::UninitializeComponent();
}

void USphereComponent::BeginPlay()
{
    UShapeComponent::BeginPlay();
}

void USphereComponent::OnComponentDestroyed()
{
    UShapeComponent::OnComponentDestroyed();
}

void USphereComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    UShapeComponent::EndPlay(EndPlayReason);
}

void USphereComponent::DestroyComponent()
{
    UShapeComponent::DestroyComponent();
}

UObject* USphereComponent::Duplicate(UObject* InOuter)
{
    return UShapeComponent::Duplicate(InOuter);
}

int USphereComponent::CheckRayIntersection(FVector& rayOrigin, FVector& rayDirection, float& pfNearHitDistance)
{
    return UShapeComponent::CheckRayIntersection(rayOrigin, rayDirection, pfNearHitDistance);
}

void USphereComponent::GetProperties(TMap<FString, FString>& OutProperties) const
{
    UShapeComponent::GetProperties(OutProperties);
}

void USphereComponent::SetProperties(const TMap<FString, FString>& InProperties)
{
    UShapeComponent::SetProperties(InProperties);
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
