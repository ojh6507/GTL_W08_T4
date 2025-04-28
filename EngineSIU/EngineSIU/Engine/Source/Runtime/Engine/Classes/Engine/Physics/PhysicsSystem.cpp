#include "PhysicsSystem.h"

#include "HitResult.h"
#include "Components/ShapeComponent.h"
#include "UObject/Casts.h"

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
    TArray<TPair<UShapeComponent*, UShapeComponent*>> Pairs;
    BroadPhase(Pairs);

    // 2) Narrow-phase
    NarrowPhase(Pairs);

    // 3) 이벤트 발송
    DispatchEvents();
}

void FPhysicsSystem::BroadPhase(TArray<TPair<UShapeComponent*, UShapeComponent*>>& OutPairs)
{
    OutPairs.Empty();
    const int Num = ShapeComponents.Num();
    TArray<FBoundingBox> WorldAABBs;
    WorldAABBs.SetNum(Num);
    for (int32 i = 0; i < Num; ++i)
    {
        UShapeComponent* Comp = ShapeComponents[i];
        FBoundingBox LocalBox = Comp->GetBoundingBox();
        WorldAABBs[i] = LocalBox.TransformWorld(Comp->GetWorldMatrix());
    }

    // 2) O(n²) 검사
    TArray<TPair<UShapeComponent*, UShapeComponent*>> CurrPairs;
    for (int32 i = 0; i < Num; ++i)
    {
        for (int32 j = i + 1; j < Num; ++j)
        {
            if (WorldAABBs[i].Intersect(WorldAABBs[j]))
            {
                TPair<UShapeComponent*, UShapeComponent*> P = { ShapeComponents[i], ShapeComponents[j] };
                CurrPairs.Add(P);
                OutPairs.Add(P);
            }
        }
    }

    // 3) 이전 프레임에 겹쳤지만, 이제 걸러지지 않은(=떨어진) 쌍에 EndOverlap 예약
    for (auto PrevP : PrevBroadPhasePairs)
    {
        if (!CurrPairs.Contains(PrevP))
        {
            UPrimitiveComponent* A = Cast<UPrimitiveComponent>(PrevP.Key);
            UPrimitiveComponent* B = Cast<UPrimitiveComponent>(PrevP.Value);
            // EndOverlap 예약
            PendingEndOverlap.Add({ A, FOverlapInfo(B) });
            PendingEndOverlap.Add({ B, FOverlapInfo(A) });
        }
    }

    PrevBroadPhasePairs = CurrPairs;
}

void FPhysicsSystem::NarrowPhase(TArray<TPair<UShapeComponent*, UShapeComponent*>>& Pairs)
{
    for (const TPair<UShapeComponent*, UShapeComponent*>& Pair : Pairs)
    {
        UShapeComponent* A = Pair.Key;
        UShapeComponent* B = Pair.Value;
        UPrimitiveComponent* PrimA = Cast<UPrimitiveComponent>(A);
        UPrimitiveComponent* PrimB = Cast<UPrimitiveComponent>(B);

        const bool bPrev = A->IsOverlappingComponent(B);
        FHitResult HitAB;
        const bool bCurr = A->CheckOverlapComponent(B, HitAB);
        
        FHitResult HitBA = FHitResult::GetReversedHit(HitAB);
        HitBA.HitComponent = PrimA;
        HitBA.HitActor = PrimA->GetOwner();
        
        if (bCurr)
        {
            // 지금 겹치는데, 이전에는 겹치지 않았다 → BeginOverlap
            if (!bPrev)
            {
                PendingBeginOverlap.Add({ PrimA, FOverlapInfo(HitAB) });
                PendingBeginOverlap.Add({ PrimB, FOverlapInfo(HitBA) });

            }
            // 충돌 히트 이벤트(매 프레임마다 발생해도 된다면)
            PendingHitEvents.Add({ PrimA, HitAB });
            PendingHitEvents.Add({ PrimB, HitBA });
        }
        else if (!bCurr && bPrev)
        {
            PendingEndOverlap.Add({ PrimA, FOverlapInfo(B) });
            PendingEndOverlap.Add({ PrimB, FOverlapInfo(A) });
        }
    }
}

void FPhysicsSystem::DispatchEvents()
{
    // --- Hit Events ---
    for (auto& Pair : PendingHitEvents)
    {
        UPrimitiveComponent* Comp = Pair.Key;
        const FHitResult& Hit     = Pair.Value;
        Comp->OnComponentHit.Broadcast(Comp, Hit.GetActor(), Hit.GetComponent(), Hit.Normal * Hit.PenetrationDepth, Hit);
    }
    PendingHitEvents.Empty();

    // --- Begin Overlap Events ---
    for (auto& Pair : PendingBeginOverlap)
    {
        UPrimitiveComponent* Comp      = Pair.Key;
        const FOverlapInfo& OverlapInfo = Pair.Value;
        Comp->BeginComponentOverlap(OverlapInfo, true);
    }
    PendingBeginOverlap.Empty();

    // --- End Overlap Events ---
    for (auto& Pair : PendingEndOverlap)
    {
        UPrimitiveComponent* Comp       = Pair.Key;
        const FOverlapInfo& OverlapInfo = Pair.Value;
        Comp->EndComponentOverlap(OverlapInfo, true);
    }
    PendingEndOverlap.Empty();
}
