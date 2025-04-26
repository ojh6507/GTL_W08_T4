#pragma once

#include "Classes/Engine/StaticMeshActor.h"
#include "Components/BoxComponent.h"

class ACube : public AStaticMeshActor
{
    DECLARE_CLASS(ACube, AStaticMeshActor)

public:
    ACube();

    virtual void Tick(float DeltaTime) override;
private:
    UBoxComponent* BoxComponent;
};

