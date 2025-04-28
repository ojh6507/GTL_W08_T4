#pragma once
#include "Classes/Engine/StaticMeshActor.h"

class UStaticMesh;
class UBoxComponent;
class UCapsuleComponent;

class AAnimPlayerActor  : public AStaticMeshActor
{
    DECLARE_CLASS(AAnimPlayerActor, AStaticMeshActor)
public:
    AAnimPlayerActor();

    virtual void Tick(float DeltaTime) override;

    void ToggleStaticMesh(UStaticMeshComponent* staticmeshcomp);
    float AccumulatedTime = 0.f;
    float ToggleInterval = 0.25f;
    bool bIsMesh1Active = true;
private:
    UBoxComponent* BoxComponent;
    //UCapsuleComponent* CapsuleComponent;
    UPROPERTY
    (UStaticMeshComponent*, StaticMeshComponent1, = nullptr);

    UStaticMesh* MeshAsset1;
    UStaticMesh* MeshAsset2;
};

