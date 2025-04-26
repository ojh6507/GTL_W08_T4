#include "CapsuleComponent.h"

#include "BoxComponent.h"
#include "SphereComponent.h"
#include "Engine/Physics/CollisionDispatcher.h"
#include "UObject/Casts.h"

UCapsuleComponent::UCapsuleComponent()
{
}

UCapsuleComponent::~UCapsuleComponent()
{
}

void UCapsuleComponent::Serialize(FArchive& Ar)
{
    UShapeComponent::Serialize(Ar);
}

void UCapsuleComponent::UninitializeComponent()
{
    UShapeComponent::UninitializeComponent();
}

void UCapsuleComponent::InitializeComponent()
{
    UShapeComponent::InitializeComponent();
}

void UCapsuleComponent::BeginPlay()
{
    UShapeComponent::BeginPlay();
}

void UCapsuleComponent::TickComponent(float DeltaTime)
{
    UShapeComponent::TickComponent(DeltaTime);
}

void UCapsuleComponent::OnComponentDestroyed()
{
    UShapeComponent::OnComponentDestroyed();
}

void UCapsuleComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    UShapeComponent::EndPlay(EndPlayReason);
}

void UCapsuleComponent::DestroyComponent()
{
    UShapeComponent::DestroyComponent();
}

UObject* UCapsuleComponent::Duplicate(UObject* InOuter)
{
    return UShapeComponent::Duplicate(InOuter);
}

void UCapsuleComponent::GetProperties(TMap<FString, FString>& OutProperties) const
{
    UShapeComponent::GetProperties(OutProperties);
}

void UCapsuleComponent::SetProperties(const TMap<FString, FString>& InProperties)
{
    UShapeComponent::SetProperties(InProperties);
}

bool UCapsuleComponent::CheckOverlapComponent(UShapeComponent* Other, FHitResult& OutHitResult)
{
    if (UBoxComponent* OtherBox = Cast<UBoxComponent>(Other))
    {
        return FCollisionDispatcher::OverlapCapsuleToBox(this, OtherBox, OutHitResult);
    }

    if (USphereComponent* OtherSphere = Cast<USphereComponent>(Other))
    {
        return FCollisionDispatcher::OverlapCapsuleToSphere(this, OtherSphere, OutHitResult);
    }

    if (UCapsuleComponent* OtherCapsule = Cast<UCapsuleComponent>(Other))
    {
        return FCollisionDispatcher::OverlapCapsuleToCapsule(this, OtherCapsule, OutHitResult);
    }

    return false;
}
