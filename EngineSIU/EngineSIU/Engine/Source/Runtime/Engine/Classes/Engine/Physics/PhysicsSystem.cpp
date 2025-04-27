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
    const int num = ShapeComponents.Num();
    for (int i = 0; i < num; ++i)
    {
        UShapeComponent* A = ShapeComponents[i];
        FBoundingBox BoundingBoxA = A->GetBoundingBox();
        for (int j = i + 1; j < num; ++j)
        {
            UShapeComponent* B = ShapeComponents[j];
            FBoundingBox BoundingBoxB = A->GetBoundingBox();

            if (BoundingBoxA.Intersect(BoundingBoxB))
            {
                OutPairs.Add({ A, B });
            }
        }
    }
}

void FPhysicsSystem::NarrowPhase(TArray<TPair<UShapeComponent*, UShapeComponent*>>& Pairs)
{
    for (const TPair<UShapeComponent*, UShapeComponent*>& Pair : Pairs)
    {
        UShapeComponent* A = Pair.Key;
        UShapeComponent* B = Pair.Value;

        const bool bPrevAB = A->IsOverlappingComponent(B);

        FHitResult HitAB;
        const bool bCurrAB = A->CheckOverlapComponent(B, HitAB);
        
        if (bCurrAB)
        {
            // 지금 겹치는데, 이전에는 겹치지 않았다 → BeginOverlap
            if (!bPrevAB)
            {
                PendingBeginOverlap.Add({ Cast<UPrimitiveComponent>(A), FOverlapInfo(HitAB) });
            }
            // 충돌 히트 이벤트(매 프레임마다 발생해도 된다면)
            PendingHitEvents.Add({ Cast<UPrimitiveComponent>(A), HitAB });
        }
        else if (bPrevAB)
        {
            // 이전에는 겹쳤는데, 지금은 안 겹치네 → EndOverlap
            PendingEndOverlap.Add({ Cast<UPrimitiveComponent>(A), FOverlapInfo(B) });
        }
        
        bool bPrevBA = B->IsOverlappingComponent(A);
        FHitResult HitBA;
        bool bCurrBA = B->CheckOverlapComponent(A, HitBA);

        if (bCurrBA)
        {
            if (!bPrevBA)
            {
                PendingBeginOverlap.Add({ Cast<UPrimitiveComponent>(B), FOverlapInfo(HitBA) });
            }
            PendingHitEvents.Add({ Cast<UPrimitiveComponent>(B), HitBA });
        }
        else if (bPrevBA)
        {
            PendingEndOverlap.Add({ Cast<UPrimitiveComponent>(B), FOverlapInfo(A) });
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
