#pragma once
#include "UObject/Object.h"
#include "UObject/ObjectMacros.h"
#include <Camera/ViewTarget.h>

class AActor;
class UWorld;

class ULevel : public UObject
{
    DECLARE_CLASS(ULevel, UObject)

public:
    ULevel() = default;

    void InitLevel(UWorld* InOwningWorld);
    void Release();

    virtual UObject* Duplicate(UObject* InOuter) override;

    void RegisterCamera(FName InName, const FViewTarget& View);

    FViewTarget GetViewTarget(FName InName);

    uint32 RemoveCamera(FName InName);

    TArray<AActor*> Actors;
    TMap<FName, FViewTarget> Cameras;
    UWorld* OwningWorld;
};
