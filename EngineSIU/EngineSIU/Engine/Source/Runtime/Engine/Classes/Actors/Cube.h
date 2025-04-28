#pragma once

#include "Classes/Engine/StaticMeshActor.h"

class UBoxComponent;
class UCapsuleComponent;

class ACube : public AStaticMeshActor
{
    DECLARE_CLASS(ACube, AStaticMeshActor)

public:
    ACube();

    virtual void Tick(float DeltaTime) override;
    UObject* Duplicate(UObject* InOuter) override;

private:
    UBoxComponent* BoxComponent;
    //UCapsuleComponent* CapsuleComponent;
};

