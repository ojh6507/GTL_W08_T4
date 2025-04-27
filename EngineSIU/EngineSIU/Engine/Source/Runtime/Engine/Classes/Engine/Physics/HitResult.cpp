#include "HitResult.h"

#include "Components/PrimitiveComponent.h"
#include "GameFramework/Actor.h"

FHitResult::FHitResult(AActor* InActor, UPrimitiveComponent* InComponent, FVector const& HitLoc, FVector const& HitNorm)
{
    Location = HitLoc;
    ImpactPoint = HitLoc;
    Normal = HitNorm;
    ImpactNormal = HitNorm;
    HitActor = InActor;
    HitComponent = InComponent;
}

FString FHitResult::ToString() const
{
    return FString::Printf(TEXT("bBlockingHit:%s bStartPenetrating:%s Time:%f Location:%s ImpactPoint:%s Normal:%s ImpactNormal:%s TraceStart:%s TraceEnd:%s PenetrationDepth:%f Item:%s PhysMaterial:%s Actor:%s Component:%s BoneName:%s FaceIndex:%d"),
    bBlockingHit == true ? TEXT("True") : TEXT("False"),
    bStartPenetrating == true ? TEXT("True") : TEXT("False"),
    Time,
    *Location.ToString(),
    *ImpactPoint.ToString(),
    *Normal.ToString(),
    *ImpactNormal.ToString(),
    *TraceStart.ToString(),
    *TraceEnd.ToString(),
    PenetrationDepth,
    HitActor ? *HitActor->GetName() : TEXT("None"),
    HitComponent ? *HitComponent->GetName() : TEXT("None"));
}
