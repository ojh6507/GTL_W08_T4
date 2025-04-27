#pragma once
#include "HAL/PlatformType.h"

struct FOverlapResult
{
    class AActor* OverlapObject;

    /** PrimitiveComponent that the check hit. */
    class UPrimitiveComponent* HitComponent;

    /** This is the index of the overlapping item.
        For DestructibleComponents, this is the ChunkInfo index.
        For SkeletalMeshComponents this is the Body index or INDEX_NONE for single body */
    int32 ItemIndex;

    /** Utility to return the Actor that owns the Component that was hit */
    AActor* GetActor() const;

    /** Utility to return the Component that was hit */
    UPrimitiveComponent* GetComponent() const;

    bool bBlockingHit;

    FOverlapResult()
    {
        memset(this, 0, sizeof(FOverlapResult));
    }
};
