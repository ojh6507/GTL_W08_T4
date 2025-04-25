#pragma once

#include "Container/Array.h"
#include "World/World.h"

class UShapeComponent;

class FPhysicsSystem
{
public:
    static void RegisterComponent(UShapeComponent* InShapeComponent);
    static void UnRegisterComponent(UShapeComponent* InShapeComponent);

    static void UpdateCollisions();
private:
    static TArray<UShapeComponent*> ShapeComponents;
};
