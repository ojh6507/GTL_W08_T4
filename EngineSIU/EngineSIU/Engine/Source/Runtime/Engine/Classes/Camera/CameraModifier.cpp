#include "CameraModifier.h"

#include "CameraTypes.h"
#include "PlayerCameraManager.h"

UCameraModifier::UCameraModifier()
{
}

UCameraModifier::~UCameraModifier()
{
}

UObject* UCameraModifier::Duplicate(UObject* InOuter)
{
    return UObject::Duplicate(InOuter);
}

float UCameraModifier::GetTargetAlpha()
{
    return bPendingDisable ? 0.0f : 1.0f;
}

void UCameraModifier::AddedToCamera(APlayerCameraManager* Camera)
{
    CameraOwner = Camera;
}

bool UCameraModifier::ModifyCamera(float DeltaTime, struct FMinimalViewInfo& InOutPOV)
{
    UpdateAlpha(DeltaTime);

    ModifyCamera(DeltaTime, InOutPOV.Location, InOutPOV.Rotation, InOutPOV.FOV, InOutPOV.Location, InOutPOV.Rotation, InOutPOV.FOV);

    LuaModifyCamera(DeltaTime, InOutPOV.Location, InOutPOV.Rotation, InOutPOV.FOV, InOutPOV.Location, InOutPOV.Rotation, InOutPOV.FOV);

    // If pending disable and fully alpha'd out, truly disable this modifier
    if (bPendingDisable && (Alpha <= 0.f))
    {
        DisableModifier(true);
    }

    // allow subsequent modifiers to update
    return false;
}

void UCameraModifier::LuaModifyCamera(float DeltaTime, FVector ViewLocation, FRotator ViewRotation, float FOV, FVector& NewViewLocation,
    FRotator& NewViewRotation, float& NewFOV)
{
}

bool UCameraModifier::IsDisabled() const
{
    return bDisabled;
}

bool UCameraModifier::IsPendingDisable() const
{
    return bPendingDisable;
}

AActor* UCameraModifier::GetViewTarget() const
{
    return CameraOwner ? CameraOwner->GetViewTarget() : nullptr;
}

void UCameraModifier::DisableModifier(bool bImmediate)
{
    if (bImmediate)
    {
        bDisabled = true;
        bPendingDisable = false;
    }
    else if (!bDisabled)
    {
        bPendingDisable = true;
    }
}

void UCameraModifier::EnableModifier()
{
    bDisabled = false;
    bPendingDisable = false;
}

void UCameraModifier::ToggleModifier()
{
    if( bDisabled )
    {
        EnableModifier();
    }
    else
    {
        DisableModifier();
    }
}

bool UCameraModifier::ProcessViewRotation(class AActor* ViewTarget, float DeltaTime, FRotator& OutViewRotation, FRotator& OutDeltaRot)
{
    return false;
}

void UCameraModifier::UpdateAlpha(float DeltaTime)
{
    float const TargetAlpha = GetTargetAlpha();
    float const BlendTime = (TargetAlpha == 0.f) ? AlphaOutTime : AlphaInTime;

    // interpolate!
    if (BlendTime <= 0.f)
    {
        // no blendtime means no blending, just go directly to target alpha
        Alpha = TargetAlpha;
    }
    else if (Alpha > TargetAlpha)
    {
        // interpolate downward to target, while protecting against overshooting
        Alpha = FMath::Max<float>(Alpha - DeltaTime / BlendTime, TargetAlpha);
    }
    else
    {
        // interpolate upward to target, while protecting against overshooting
        Alpha = FMath::Min<float>(Alpha + DeltaTime / BlendTime, TargetAlpha);
    }
}

UWorld* UCameraModifier::GetWorld() const
{
    return CameraOwner ? CameraOwner->GetWorld() : nullptr;
}

void UCameraModifier::ModifyCamera(float DeltaTime, FVector ViewLocation, FRotator ViewRotation, float FOV, FVector& NewViewLocation,
    FRotator& NewViewRotation, float& NewFOV)
{
}
