#include "OverlapInfo.h"

#include "Components/PrimitiveComponent.h"


FOverlapInfo::FOverlapInfo(UPrimitiveComponent* InComponent)
    : bFromSweep(false)
{
    if (InComponent)
    {
        OverlapInfo.HitActor = InComponent->GetOwner();
    }
    OverlapInfo.HitComponent = InComponent;
}
