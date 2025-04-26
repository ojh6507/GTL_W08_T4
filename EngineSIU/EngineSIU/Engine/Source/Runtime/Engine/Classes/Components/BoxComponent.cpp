#include "BoxComponent.h"

#include "SphereComponent.h"
#include "Engine/FLoaderOBJ.h"
#include "UObject/Casts.h"
#include "UObject/ObjectFactory.h"


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
    UShapeComponent::Serialize(Ar);
}

void UBoxComponent::UninitializeComponent()
{
    UShapeComponent::UninitializeComponent();
}

void UBoxComponent::BeginPlay()
{
    UShapeComponent::BeginPlay();
}

void UBoxComponent::OnComponentDestroyed()
{
    UShapeComponent::OnComponentDestroyed();
}

void UBoxComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    UShapeComponent::EndPlay(EndPlayReason);
}

void UBoxComponent::DestroyComponent()
{
    UShapeComponent::DestroyComponent();
}

UObject* UBoxComponent::Duplicate(UObject* InOuter)
{
    return UShapeComponent::Duplicate(InOuter);
}

int UBoxComponent::CheckRayIntersection(FVector& rayOrigin, FVector& rayDirection, float& pfNearHitDistance)
{
    return UShapeComponent::CheckRayIntersection(rayOrigin, rayDirection, pfNearHitDistance);
}

void UBoxComponent::GetProperties(TMap<FString, FString>& OutProperties) const
{
    UShapeComponent::GetProperties(OutProperties);
}

void UBoxComponent::SetProperties(const TMap<FString, FString>& InProperties)
{
    UShapeComponent::SetProperties(InProperties);
}

bool UBoxComponent::CheckOverlapComponent(UShapeComponent* Other, FHitResult& OutHitResult)
{
    if (UBoxComponent* OtherBox = Cast<UBoxComponent>(Other))
    {
        
    }

    if (USphereComponent* OtherSphere = Cast<USphereComponent>(Other))
    {
        
    }

    return true;
}
