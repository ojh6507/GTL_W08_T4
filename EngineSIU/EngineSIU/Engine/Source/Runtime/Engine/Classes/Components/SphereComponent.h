#pragma once
#include "ShapeComponent.h"

class UCapsuleComponent;
class UBoxComponent;

class USphereComponent : public UShapeComponent
{
    DECLARE_CLASS(USphereComponent, UShapeComponent)

public:
    USphereComponent();
    ~USphereComponent() override;
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

    virtual void InitializeComponent() override;
    virtual void TickComponent(float DeltaTime) override;

    void HandleBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, bool bFromSweep, const FHitResult& SweepResult);
    void HandleEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp);
public:
    FORCEINLINE float GetRadius() const { return SphereRadius; }
    FORCEINLINE void SetRadius(const float InRadius) { SphereRadius = InRadius; }

private:
    float SphereRadius;
};
