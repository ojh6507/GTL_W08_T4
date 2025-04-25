#pragma once
#include"GameFramework/Actor.h"

class UProjectileMovementComponent;
class UPointLightComponent;
class UStaticMeshComponent;

class AFireballActor : public AActor
{
    DECLARE_CLASS(AFireballActor, AActor)

public:
    AFireballActor();

    virtual void BeginPlay() override;

protected:
    UPROPERTY
    (UProjectileMovementComponent*, ProjectileMovementComponent, = nullptr);

    UPROPERTY
    (UPointLightComponent*, PointLightComponent, = nullptr);

    UPROPERTY
    (UStaticMeshComponent*, SphereComp, = nullptr);
};
