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
    FadeTimeRemaining = 3;
    FadeAmount = 2.f;
    FadeAlpha.X = 0;
    FadeAlpha.Y = 1;
    FadeTime = 2.f;
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
    StartCameraFade(FadeAmount, 1.0f, duration, color, false, true);
}

void APlayerCameraManager::UpdateFade(float DeltaTime)
{
    // 페이드 진행 중일 때만 업데이트 (남은 시간 > 0)
    if (FadeTimeRemaining > 0.0f)
    {
        // 남은 시간 감소
        FadeTimeRemaining -= DeltaTime;

        if (FadeTimeRemaining <= 0.0f)
        {
            // 페이드 완료
            FadeAmount = FadeAlpha.Y; // 목표 알파로 정확히 설정
            FadeTimeRemaining = 0.0f; // 남은 시간 0으로 확실히
        }
        else
        {
            // 페이드 진행 중
            if (FadeTime > 0.0f)
            {
                const float ElapsedTime = FadeTime - FadeTimeRemaining;
                const float FadeProgress = FMath::Clamp(ElapsedTime / FadeTime, 0.0f, 1.0f);
                FadeAmount = FMath::Lerp(FadeAlpha.X, FadeAlpha.Y, FadeProgress);

            }
        }
    }
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

void APlayerCameraManager::StartCameraFade(float FromAlpha, float ToAlpha, float Duration, FLinearColor Color, bool bShouldFadeAudio, bool bHoldWhenFinished)
{
    FromAlpha = FMath::Clamp(FromAlpha, 0.0f, 1.0f);
    ToAlpha = FMath::Clamp(ToAlpha, 0.0f, 1.0f);
 
    FadeColor = Color;
    FadeAlpha.X = FromAlpha; // 시작 알파 저장
    FadeAlpha.Y = ToAlpha;   // 목표 알파 저장
    FadeTime = FMath::Max(Duration, 0.0f); // 음수 방지
    FadeTimeRemaining = FadeTime; // 남은 시간 설정 (0이면 즉시 완료됨)
    FadeAmount = FromAlpha; // 현재 알파를 시작 알파로 설정

    if (FadeTimeRemaining <= 0.0f)
    {
        FadeAmount = FadeAlpha.Y; // 즉시 목표 알파로 설정
    }
}

void APlayerCameraManager::StopCameraFade()
{
    if (FadeTimeRemaining > 0.f)
    {
        FadeTimeRemaining = 0.f;
    }
}

AActor* APlayerCameraManager::GetViewTarget() const
{
    if (PendingViewTarget.Target)
    {
        return PendingViewTarget.Target;
    }

    return ViewTarget.Target;
}
