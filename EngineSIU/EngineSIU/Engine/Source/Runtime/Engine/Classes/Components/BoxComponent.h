#pragma once
#include "ShapeComponent.h"

class UCapsuleComponent;
class USphereComponent;

class UBoxComponent : public UShapeComponent
{
    DECLARE_CLASS(UBoxComponent, UShapeComponent)

public:
    UBoxComponent();

    virtual void InitializeComponent() override;
    virtual void TickComponent(float DeltaTime) override;
    ~UBoxComponent() override;
    void Serialize(FArchive& Ar) override;
    void UninitializeComponent() override;
    void BeginPlay() override;
    void OnComponentDestroyed() override;
    void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
    void DestroyComponent() override;
    UObject* Duplicate(UObject* InOuter) override;
    int CheckRayIntersection(FVector& rayOrigin, FVector& rayDirection, float& pfNearHitDistance) override;
    void GetProperties(TMap<FString, FString>& OutProperties) const override;
    void SetProperties(const TMap<FString, FString>& InProperties) override;
    bool CheckOverlapComponent(UShapeComponent* Other, FHitResult& OutHitResult) override;

public:
    FORCEINLINE FVector GetBoxExtent() const { return BoxExtent; }
    FORCEINLINE void SetBoxExtent(const FVector InExtent) { BoxExtent = InExtent; }

private:
    FVector BoxExtent;
};
