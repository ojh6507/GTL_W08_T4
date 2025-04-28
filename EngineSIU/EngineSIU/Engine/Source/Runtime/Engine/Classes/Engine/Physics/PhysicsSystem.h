#pragma once

#include "HitResult.h"
#include "OverlapInfo.h"
#include "Container/Array.h"
#include "World/World.h"

class UPrimitiveComponent;
class UShapeComponent;

class FPhysicsSystem
{
public:
    static FPhysicsSystem& Get()
    {
        static FPhysicsSystem Instance;
        return Instance;
    }
    
    void RegisterComponent(UShapeComponent* InShapeComponent);
    void UnRegisterComponent(UShapeComponent* InShapeComponent);

    void UpdateCollisions();

private:
    // 1) Broad-phase 후보 추출
    void BroadPhase(TArray<TPair<UShapeComponent*, UShapeComponent*>>& OutPairs);

    // 2) Narrow-phase 검사
    void NarrowPhase(TArray<TPair<UShapeComponent*, UShapeComponent*>>& Pairs);

    // 3) 이벤트 브로드캐스트
    void DispatchEvents();
private:
    FPhysicsSystem() = default;
    ~FPhysicsSystem() = default;

    // 복사/대입 금지
    FPhysicsSystem(const FPhysicsSystem&) = delete;
    FPhysicsSystem& operator=(const FPhysicsSystem&) = delete;

    // (원하면 이동 금지도 추가)
    FPhysicsSystem(FPhysicsSystem&&) = delete;
    FPhysicsSystem& operator=(FPhysicsSystem&&) = delete;

    // 충돌 체크할 ShapeComponent들
    TArray<UShapeComponent*> ShapeComponents;
    // 충돌 히트 이벤트
    TArray<TPair<UPrimitiveComponent*, FHitResult>> PendingHitEvents;
    // BeginOverlap 이벤트
    TArray<TPair<UPrimitiveComponent*, FOverlapInfo>> PendingBeginOverlap;
    // EndOverlap 이벤트
    TArray<TPair<UPrimitiveComponent*, FOverlapInfo>> PendingEndOverlap;

    TArray<TPair<UShapeComponent*, UShapeComponent*>> PrevBroadPhasePairs;
};
