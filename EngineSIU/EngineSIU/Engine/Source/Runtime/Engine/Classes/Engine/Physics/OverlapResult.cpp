#include "OverlapResult.h"

AActor* FOverlapResult::GetActor() const
{
    return OverlapObject;
}

UPrimitiveComponent* FOverlapResult::GetComponent() const
{
    return HitComponent;
}
