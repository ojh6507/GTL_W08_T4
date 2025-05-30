#pragma once
#include "ShapeComponent.h"

class USphereComponent;
class UBoxComponent;

class UCapsuleComponent : public UShapeComponent
{
    DECLARE_CLASS(UCapsuleComponent, UShapeComponent)
public:
    UCapsuleComponent();
    ~UCapsuleComponent() override;
    void UninitializeComponent() override;
    void InitializeComponent() override;
    void BeginPlay() override;
    void TickComponent(float DeltaTime) override;
    void OnComponentDestroyed() override;
    void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
    void DestroyComponent() override;
    UObject* Duplicate(UObject* InOuter) override;
    void GetProperties(TMap<FString, FString>& OutProperties) const override;
    void SetProperties(const TMap<FString, FString>& InProperties) override;
    bool CheckOverlapComponent(UShapeComponent* Other, FHitResult& OutHitResult) override;
    
    void HandleBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, bool bFromSweep, const FHitResult& SweepResult);
    void HandleEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp);
public:
    FORCEINLINE float GetCapsuleRadius() const { return CapsuleRadius; }
    FORCEINLINE void SetCapsuleRadius(const float InRadius) { CapsuleRadius = InRadius; }

    FORCEINLINE float GetCapsuleHalfHeight() const { return CapsuleHalfHeight; }
    FORCEINLINE void SetCapsuleHalfHeight(const float InCasuleHalfHeight) { CapsuleHalfHeight = InCasuleHalfHeight; }

private:
    float CapsuleHalfHeight;
    float CapsuleRadius;
};
