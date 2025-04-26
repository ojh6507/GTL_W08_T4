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
    
    // 1) Broad-phase 후보 추출
    static void BroadPhase(std::vector<std::pair<UShapeComponent*,UShapeComponent*>>& OutPairs);

    // 2) Narrow-phase 검사
    static void NarrowPhase(const std::vector<std::pair<UShapeComponent*,UShapeComponent*>>& Pairs);

    // 3) 이벤트 브로드캐스트
    static void DispatchEvents();
private:
    static TArray<UShapeComponent*> ShapeComponents;
};
