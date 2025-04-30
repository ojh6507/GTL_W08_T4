#include "PlayerCameraManager.h"

#include "CameraComponent.h"
#include "CameraModifier.h"
#include "Actors/CameraActor.h"
#include "World/World.h"
#include "CameraShakeModifier.h"
#

bool FViewTarget::Equal(const FViewTarget& OtherTarget) const
{
    return (Target == OtherTarget.Target) && POV.Equals(OtherTarget.POV);

}

APlayerCameraManager::APlayerCameraManager()
{
}

UObject* APlayerCameraManager::Duplicate(UObject* InOuter)
{
    return AActor::Duplicate(InOuter);
}

void APlayerCameraManager::BeginPlay()
{
    AActor::BeginPlay();

    AddModifier(FObjectFactory::ConstructObject<UCameraShakeModifier>(this));
}

void APlayerCameraManager::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    AActor::EndPlay(EndPlayReason);
}

float APlayerCameraManager::GetLockedFOV() const
{
    return LockedFOV;
}

void APlayerCameraManager::ForEachCameraModifier(const std::function<bool(UCameraModifier*)>& Fn) const
{
    // Local copy the modifiers array in case it get when calling the lambda on each modifiers
    TArray<UCameraModifier*> LocalModifierList = ModifierList;

    // Loop through each camera modifier
    for (int32 ModifierIdx = 0; ModifierIdx < LocalModifierList.Num(); ++ModifierIdx)
    {
        if (!Fn(LocalModifierList[ModifierIdx]))
        {
            return;
        }
    }
}

void APlayerCameraManager::SetCameraCachePOV(const FMinimalViewInfo& InPOV)
{
    CameraCachePrivate.POV = InPOV;
}

void APlayerCameraManager::SetLastFrameCameraCachePOV(const FMinimalViewInfo& InPOV)
{
    LastFrameCameraCachePrivate.POV = InPOV;
}

const FMinimalViewInfo& APlayerCameraManager::GetCameraCacheView() const
{
    return CameraCachePrivate.POV;
}

const FMinimalViewInfo& APlayerCameraManager::GetLastFrameCameraCacheView() const
{
    return LastFrameCameraCachePrivate.POV;
}

FMinimalViewInfo APlayerCameraManager::GetCameraCachePOV() const
{
    return GetCameraCacheView();
}

FMinimalViewInfo APlayerCameraManager::GetLastFrameCameraCachePOV() const
{
    return GetLastFrameCameraCacheView();
}

void APlayerCameraManager::Tick(float deltaTime)
{
    AActor::Tick(deltaTime);

    if (ActiveCameraName != TEXT(""))
    {
        
    }
    
    UpdateFade(deltaTime);
    ApplyModifiers(deltaTime);
}

void APlayerCameraManager::SetActiveCamera(const FName& name)
{
    ActiveCameraName = name;
    ViewTarget = GetWorld()->GetCamera(ActiveCameraName);
}

void APlayerCameraManager::AddModifier(UCameraModifier* modifier)
{
    ModifierList.Add(modifier);
}

void APlayerCameraManager::RemoveModifier(UCameraModifier* modifier)
{
    ModifierList.Remove(modifier);
}

void APlayerCameraManager::StartFade(const FLinearColor& color, float duration)
{

}

void APlayerCameraManager::UpdateFade(float deltaTime)
{
}

void APlayerCameraManager::ApplyModifiers(float deltaTime)
{
    std::sort(ModifierList.begin(), ModifierList.end(), [](UCameraModifier* a, UCameraModifier* b) {
    return a->Priority < b->Priority;
    });
    // 각 모디파이어 적용
    for (auto* mod : ModifierList) {
        if (!mod->IsDisabled()) {
            mod->UpdateAlpha(deltaTime);

            FVector NewLocation;
            FRotator NewRotation;
            float NewFOV;
            mod->ModifyCamera(deltaTime, ViewTarget.Target->GetActorLocation(), ViewTarget.Target->GetActorRotation(), ViewTarget.POV.FOV, NewLocation, NewRotation, NewFOV);
        }
    }
}

float APlayerCameraManager::GetFOVAngle() const
{
    return (LockedFOV > 0.f) ? LockedFOV : GetCameraCacheView().FOV;
}

void APlayerCameraManager::SetFOVAngle(float FOVAngle)
{
    LockedFOV = FOVAngle;
}

void APlayerCameraManager::UnLockFOV()
{
    LockedFOV = 0.f;
}

bool APlayerCameraManager::IsOrthographic() const
{
    return bIsOrthographic;
}

float APlayerCameraManager::GetOrthoWidth() const
{
    return (LockedOrthoWidth > 0.f) ? LockedOrthoWidth : DefaultOrthoWidth;
}

void APlayerCameraManager::SetOrthoWidth(float width)
{
    LockedOrthoWidth = width;
}

void APlayerCameraManager::UnLockOrthoWidth()
{
    LockedOrthoWidth = 0.f;
}

void APlayerCameraManager::GetCameraViewPoint(FVector& OutCamLoc, FRotator& OutCamRot) const
{
    const FMinimalViewInfo& CurrentPOV = GetCameraCacheView();
    OutCamLoc = CurrentPOV.Location;
    OutCamRot = CurrentPOV.Rotation;
}

FRotator APlayerCameraManager::GetCameraRotation() const
{
    return GetCameraCacheView().Rotation;
}

FVector APlayerCameraManager::GetCameraLocation() const
{
    return GetCameraCacheView().Location;
}

void APlayerCameraManager::SetDesiredColorScale(FVector NewColorScale, float InterpTime)
{
    
}

void APlayerCameraManager::DoUpdateCamera(float DeltaTime)
{
    FMinimalViewInfo NewPOV = ViewTarget.POV;

}

bool APlayerCameraManager::LuaUpdateCamera(AActor* CameraTarget, FVector& NewCameraLocation, FRotator& NewCameraRotation, float& NewCameraFOV)
{
    return true;
}

void APlayerCameraManager::SetViewTarget(AActor* NewViewTarget, FViewTargetTransitionParams TransitionParams)
{
}

void APlayerCameraManager::ProcessViewRotation(float DeltaTime, FRotator& OutViewRotation, FRotator& OutDeltaRot)
{
}

void APlayerCameraManager::StartCameraFade(float FromAlpha, float ToAlpha, float Duration, FLinearColor Color, bool bShouldFadeAudio,
    bool bHoldWhenFinished)
{
}

void APlayerCameraManager::StopCameraFade()
{
}

AActor* APlayerCameraManager::GetViewTarget() const
{
    if (PendingViewTarget.Target)
    {
        return PendingViewTarget.Target;
    }

    return ViewTarget.Target;
}
