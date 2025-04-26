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

void UCapsuleComponent::UninitializeComponent()
{
    Super::UninitializeComponent();
}

void UCapsuleComponent::InitializeComponent()
{
    Super::InitializeComponent();
}

void UCapsuleComponent::BeginPlay()
{
    Super::BeginPlay();
}

void UCapsuleComponent::TickComponent(float DeltaTime)
{
    Super::TickComponent(DeltaTime);
}

void UCapsuleComponent::OnComponentDestroyed()
{
    Super::OnComponentDestroyed();
}

void UCapsuleComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    Super::EndPlay(EndPlayReason);
}

void UCapsuleComponent::DestroyComponent()
{
    Super::DestroyComponent();
}

UObject* UCapsuleComponent::Duplicate(UObject* InOuter)
{
    ThisClass* NewComponent = Cast<ThisClass>(Super::Duplicate(InOuter));
    CapsuleHalfHeight = NewComponent->CapsuleHalfHeight;
    CapsuleRadius = NewComponent->CapsuleRadius;
    
    return NewComponent;
}

void UCapsuleComponent::GetProperties(TMap<FString, FString>& OutProperties) const
{
    Super::GetProperties(OutProperties);
    OutProperties.Add(TEXT("CapsuleHalfHeight"), FString::Printf(TEXT("%f"), CapsuleHalfHeight));
    OutProperties.Add(TEXT("CapsuleRadius"), FString::Printf(TEXT("%f"), CapsuleRadius));
}

void UCapsuleComponent::SetProperties(const TMap<FString, FString>& InProperties)
{
    Super::SetProperties(InProperties);
    const FString* TempStr = nullptr;
    TempStr = InProperties.Find(TEXT("CapsuleHalfHeight"));
    if (TempStr)
    {
        CapsuleHalfHeight = FString::ToFloat(*TempStr);
    }
    TempStr = InProperties.Find(TEXT("CapsuleRadius"));
    if (TempStr)
    {
        CapsuleRadius = FString::ToFloat(*TempStr);
    }
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
