#include "PhysicsSystem.h"

#include "HitResult.h"
#include "Components/ShapeComponent.h"

TArray<UShapeComponent*> FPhysicsSystem::ShapeComponents = {};

void FPhysicsSystem::RegisterComponent(UShapeComponent* InShapeComponent)
{
    ShapeComponents.Add(InShapeComponent);
}

void FPhysicsSystem::UnRegisterComponent(UShapeComponent* InShapeComponent)
{
    ShapeComponents.Remove(InShapeComponent);
}

void FPhysicsSystem::UpdateCollisions()
{
    // 1) Broad-phase
    std::vector<std::pair<UShapeComponent*,UShapeComponent*>> Pairs;
    BroadPhase(Pairs);

    // 2) Narrow-phase
    NarrowPhase(Pairs);

    // 3) 이벤트 발송
    DispatchEvents();
}

void FPhysicsSystem::BroadPhase(std::vector<std::pair<UShapeComponent*, UShapeComponent*>>& OutPairs)
{
    
}

void FPhysicsSystem::NarrowPhase(const std::vector<std::pair<UShapeComponent*, UShapeComponent*>>& Pairs)
{
}

void FPhysicsSystem::DispatchEvents()
{
}
