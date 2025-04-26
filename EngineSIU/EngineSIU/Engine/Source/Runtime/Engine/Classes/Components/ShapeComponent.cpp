#include "ShapeComponent.h"

UShapeComponent::UShapeComponent()
{
}

UShapeComponent::~UShapeComponent()
{
}

void UShapeComponent::Serialize(FArchive& Ar)
{
    UPrimitiveComponent::Serialize(Ar);
}

void UShapeComponent::UninitializeComponent()
{
    UPrimitiveComponent::UninitializeComponent();
}

void UShapeComponent::InitializeComponent()
{
    UPrimitiveComponent::InitializeComponent();
}

void UShapeComponent::BeginPlay()
{
    UPrimitiveComponent::BeginPlay();
}

void UShapeComponent::TickComponent(float DeltaTime)
{
    UPrimitiveComponent::TickComponent(DeltaTime);
}

void UShapeComponent::OnComponentDestroyed()
{
    UPrimitiveComponent::OnComponentDestroyed();
}

void UShapeComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    UPrimitiveComponent::EndPlay(EndPlayReason);
}

void UShapeComponent::DestroyComponent()
{
    UPrimitiveComponent::DestroyComponent();
}

UObject* UShapeComponent::Duplicate(UObject* InOuter)
{
    return UPrimitiveComponent::Duplicate(InOuter);
}

int UShapeComponent::CheckRayIntersection(FVector& rayOrigin, FVector& rayDirection, float& pfNearHitDistance)
{
    return UPrimitiveComponent::CheckRayIntersection(rayOrigin, rayDirection, pfNearHitDistance);
}

void UShapeComponent::GetProperties(TMap<FString, FString>& OutProperties) const
{
    UPrimitiveComponent::GetProperties(OutProperties);
}

void UShapeComponent::SetProperties(const TMap<FString, FString>& InProperties)
{
    UPrimitiveComponent::SetProperties(InProperties);
}

bool UShapeComponent::CheckOverlapComponent(UShapeComponent* Other, FHitResult& OutHitResult)
{
    return true;
}
