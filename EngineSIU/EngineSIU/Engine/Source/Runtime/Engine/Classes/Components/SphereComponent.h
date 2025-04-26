#pragma once
#include "ShapeComponent.h"

class UCapsuleComponent;
class UBoxComponent;

class USphereComponent : public UShapeComponent
{
    DECLARE_CLASS(USphereComponent, UShapeComponent)

public:
    USphereComponent();

    virtual void InitializeComponent() override;
    virtual void TickComponent(float DeltaTime) override;

private:
    bool OverlapSphereToBox(UBoxComponent* Other, FHitResult& OutHitResult);
    bool OverlapSphereToSphere(USphereComponent* Other, FHitResult& OutHitResult);
    bool OverlapSphereToCapsule(UCapsuleComponent* Other, FHitResult& OutHitResult);

public:
    FORCEINLINE float GetRadius() const { return SphereRadius; }
    FORCEINLINE void SetRadius(const float InRadius) { SphereRadius = InRadius; }

private:
    float SphereRadius;
};
