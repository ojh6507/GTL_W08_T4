#include "PlayerCameraManager.h"

#include "CameraComponent.h"
#include "CameraModifier.h"
#include "Actors/CameraActor.h"
#include "Math/JungleMath.h"
#include "World/World.h"
#include "CameraShakeModifier.h"

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

	SetActiveCamera(TEXT("MainCamera"));

    UCameraShakeModifier* modifier = FObjectFactory::ConstructObject<UCameraShakeModifier>(this);
    AddModifier(modifier);
    modifier->EnableModifier();
    modifier->StartShake(.5f, 2.0f);
}

void APlayerCameraManager::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    ModifierList.Empty();
    //// clean up the temp camera actor
    //if (AnimCameraActor != nullptr)
    //{
    //    AnimCameraActor->SetOwner(nullptr);
    //    AnimCameraActor = nullptr;
    //}
    
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

void APlayerCameraManager::Tick(float DeltaTime)
{
    AActor::Tick(DeltaTime);

    DoUpdateCamera(DeltaTime);
    CurCameraComp->GetOwner()->SetActorLocation(CameraCachePrivate.POV.Location);
    CurCameraComp->GetOwner()->SetActorRotation(CameraCachePrivate.POV.Rotation);
}

void APlayerCameraManager::SetActiveCamera(const FName& name)
{
    ActiveCameraName = name;
    ViewTarget = GetWorld()->GetViewTarget(ActiveCameraName);
    CurCameraComp = GetWorld()->GetCameraComponent(ActiveCameraName);
}

void APlayerCameraManager::AddModifier(UCameraModifier* modifier)
{
    ModifierList.Add(modifier);
}

void APlayerCameraManager::RemoveModifier(UCameraModifier* modifier)
{
    ModifierList.Remove(modifier);
}

void APlayerCameraManager::ApplyCameraModifiers(float DeltaTime, FMinimalViewInfo& InOutPOV)
{
    // Loop through each camera modifier
    ForEachCameraModifier([DeltaTime, &InOutPOV](UCameraModifier* CameraModifier)
    {
        bool bContinue = true;

        // Apply camera modification and output into DesiredCameraOffset/DesiredCameraRotation
        if ((CameraModifier != nullptr) && !CameraModifier->IsDisabled())
        {
            // If ModifyCamera returns true, exit loop
            // Allows high priority things to dictate if they are
            // the last modifier to be applied
            bContinue = !CameraModifier->ModifyCamera(DeltaTime, InOutPOV);
        }

        return bContinue;
    });
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
    //// if color scaling is not enabled
    //if (!bEnableColorScaling)
    //{
    //    // set the default color scale
    //    bEnableColorScaling = true;
    //    ColorScale.X = 1.f;
    //    ColorScale.Y = 1.f;
    //    ColorScale.Z = 1.f;
    //}

    //// Don't bother interpolating if we're already scaling at the desired color
    //if( NewColorScale != ColorScale )
    //{
    //    // save the current as original
    //    OriginalColorScale = ColorScale;
    //    // set the new desired scale
    //    DesiredColorScale = NewColorScale;
    //    // set the interpolation duration/time
    //    ColorScaleInterpStartTime = GetWorld()->TimeSeconds;
    //    ColorScaleInterpDuration = InterpTime;
    //    // and enable color scale interpolation
    //    bEnableColorScaleInterp = true;
    //}
}

void APlayerCameraManager::DoUpdateCamera(float DeltaTime)
{
	FMinimalViewInfo NewPOV = ViewTarget.POV;

    if (PendingViewTarget.Target == nullptr || !BlendParams.bLockOutgoing)
    {
        UpdateViewTarget(ViewTarget, DeltaTime);
    }

    NewPOV = ViewTarget.POV;

    if (PendingViewTarget.Target != nullptr)
    {
        BlendTimeToGo -= DeltaTime;
        UpdateViewTarget(ViewTarget, DeltaTime);

        // blend....
        if (BlendTimeToGo > 0)
        {
            float DurationPct = (BlendParams.BlendTime - BlendTimeToGo) / BlendParams.BlendTime;

            float BlendPct = 0.f;
            switch (BlendParams.BlendFunction)
            {
            case VTBlend_Linear:
                BlendPct = FMath::Lerp(0.f, 1.f, DurationPct);
                break;
            case VTBlend_Cubic:
                //BlendPct = FMath::CubicInterp(0.f, 0.f, 1.f, 0.f, DurationPct);
                break;
            case VTBlend_EaseIn:
                BlendPct = FMath::Lerp(0.f, 1.f, FMath::Pow(DurationPct, BlendParams.BlendExp));
                break;
            case VTBlend_EaseOut:
                BlendPct = FMath::Lerp(0.f, 1.f, FMath::Pow(DurationPct, 1.f / BlendParams.BlendExp));
                break;
            case VTBlend_EaseInOut:
                BlendPct = FMath::InterpEaseInOut(0.f, 1.f, DurationPct, BlendParams.BlendExp);
                break;
            case VTBlend_PreBlended:
                BlendPct = 1.0f;
                break;
            default:
                break;
            }


            NewPOV = ViewTarget.POV;
            NewPOV.BlendViewInfo(PendingViewTarget.POV, BlendPct);//@TODO: CAMERA: Make sure the sense is correct!  
            const float PendingViewTargetPPWeight = PendingViewTarget.POV.PostProcessBlendWeight * BlendPct;
        }
        else
        {
            // we're done blending, set new view target
            ViewTarget = PendingViewTarget;

            // clear pending view target
            PendingViewTarget.Target = NULL;

            BlendTimeToGo = 0;

            // our camera is now viewing there
            NewPOV = PendingViewTarget.POV;

            OnBlendComplete().Broadcast();
        }
    }

    if (bEnableFading)
    {
        if (bAutoAnimateFade)
        {
            FadeTimeRemaining = FMath::Max(FadeTimeRemaining - DeltaTime, 0.0f);
            if (FadeTime > 0.0f)
            {
                FadeAmount = FadeAlpha.X + ((1.f - FadeTimeRemaining / FadeTime) * (FadeAlpha.Y - FadeAlpha.X));
            }

            if ((bHoldFadeWhenFinished == false) && (FadeTimeRemaining <= 0.f))
            {
                // done
                StopCameraFade();
            }
        }
    }

    // Cache results
    FillCameraCache(NewPOV);
}

bool APlayerCameraManager::LuaUpdateCamera(AActor* CameraTarget, FVector& NewCameraLocation, FRotator& NewCameraRotation, float& NewCameraFOV)
{
    return true;
}

void APlayerCameraManager::AssignViewTarget(AActor* NewTarget, FViewTarget& VT, struct FViewTargetTransitionParams TransitionParams)
{
    if (!NewTarget)
    {
        return;
    }

    // Skip assigning to the same target unless we have a pending view target that's bLockOutgoing
    if (NewTarget == VT.Target && !(PendingViewTarget.Target && BlendParams.bLockOutgoing))
    {
        return;
    }

    AActor* OldViewTarget = VT.Target;
    VT.Target = NewTarget;

    // Use default FOV and aspect ratio.
    VT.POV.AspectRatio = DefaultAspectRatio;
    VT.POV.bConstrainAspectRatio = bDefaultConstrainAspectRatio;
    VT.POV.FOV = DefaultFOV;

    //PCOwner->ClientSetViewTarget(VT.Target, TransitionParams);
}

FPOV APlayerCameraManager::BlendViewTargets(const FViewTarget& A, const FViewTarget& B, float Alpha)
{
    FPOV POV;
    POV.Location = FMath::Lerp(A.POV.Location, B.POV.Location, Alpha);
    POV.FOV = (A.POV.FOV +  Alpha * ( B.POV.FOV - A.POV.FOV));

    FRotator DeltaAng = (B.POV.Rotation - A.POV.Rotation).GetNormalized();
    POV.Rotation = A.POV.Rotation + DeltaAng *  Alpha;

    return POV;
}

void APlayerCameraManager::FillCameraCache(const FMinimalViewInfo& NewInfo)
{
    // Backup last frame results.
    const float CurrentCacheTime = GetCameraCacheTime();
    const float CurrentGameTime = GetWorld()->TimeSeconds;
    if (CurrentCacheTime != CurrentGameTime)
    {
        SetLastFrameCameraCachePOV(GetCameraCacheView());
        SetLastFrameCameraCacheTime(CurrentCacheTime);
    }

    SetCameraCachePOV(NewInfo);
    SetCameraCacheTime(CurrentGameTime);
}

void APlayerCameraManager::UpdateViewTarget(FViewTarget& OutVT, float DeltaTime)
{
    // Don't update outgoing viewtarget during an interpolation 
    if ((PendingViewTarget.Target != nullptr) && BlendParams.bLockOutgoing && OutVT.Equal(ViewTarget))
    {
        return;
    }

    // Store previous POV, in case we need it later
    FMinimalViewInfo OrigPOV = OutVT.POV;

    // Reset the view target POV fully
    static const FMinimalViewInfo DefaultViewInfo;
    OutVT.POV = DefaultViewInfo;
    OutVT.POV.FOV = DefaultFOV;
    OutVT.POV.AspectRatio = DefaultAspectRatio;
    OutVT.POV.bConstrainAspectRatio = bDefaultConstrainAspectRatio;
    OutVT.POV.ProjectionMode = bIsOrthographic ? ECameraProjectionMode::Orthographic : ECameraProjectionMode::Perspective;
    OutVT.POV.PostProcessBlendWeight = 1.0f;
    OutVT.POV.AutoPlaneShift = AutoPlaneShift;

    bool bDoNotApplyModifiers = false;
    if (ACameraActor* CamActor = Cast<ACameraActor>(OutVT.Target))
    {
        // Viewing through a camera actor.
        CamActor->GetCameraComponent()->GetCameraView(DeltaTime, OutVT.POV);
    }
    else
    {
        static const FName NAME_Fixed = FName(TEXT("Fixed"));
        static const FName NAME_ThirdPerson = FName(TEXT("ThirdPerson"));
        static const FName NAME_FreeCam = FName(TEXT("FreeCam"));
        static const FName NAME_FreeCam_Default = FName(TEXT("FreeCam_Default"));
        static const FName NAME_FirstPerson = FName(TEXT("FirstPerson"));

        if (CameraStyle == NAME_Fixed)
        {
            // do not update, keep previous camera position by restoring
            // saved POV, in case CalcCamera changes it but still returns false
            OutVT.POV = OrigPOV;

            // don't apply modifiers when using this debug camera mode
            bDoNotApplyModifiers = true;
        }
        //else if (CameraStyle == NAME_ThirdPerson || CameraStyle == NAME_FreeCam || CameraStyle == NAME_FreeCam_Default)
        //{
        //    // Simple third person view implementation
        //    FVector Loc = OutVT.Target->GetActorLocation();
        //    FRotator Rotator = OutVT.Target->GetActorRotation();

        //    if (OutVT.Target == PCOwner)
        //    {
        //        Loc = PCOwner->GetFocalLocation();
        //    }

        //    // Take into account Mesh Translation so it takes into account the PostProcessing we do there.
        //    // @fixme, can crash in certain BP cases where default mesh is null
        //    //			APawn* TPawn = Cast<APawn>(OutVT.Target);
        //    // 			if ((TPawn != NULL) && (TPawn->Mesh != NULL))
        //    // 			{
        //    // 				Loc += FQuatRotationMatrix(OutVT.Target->GetActorQuat()).TransformVector(TPawn->Mesh->RelativeLocation - GetDefault<APawn>(TPawn->GetClass())->Mesh->RelativeLocation);
        //    // 			}

        //    //OutVT.Target.GetActorEyesViewPoint(Loc, Rot);
        //    if( CameraStyle == NAME_FreeCam || CameraStyle == NAME_FreeCam_Default )
        //    {
        //        Rotator = PCOwner->GetControlRotation();
        //    }

        //    FVector Pos = Loc + ViewTargetOffset + FRotationMatrix(Rotator).TransformVector(FreeCamOffset) - Rotator.Vector() * FreeCamDistance;
        //    FCollisionQueryParams BoxParams(SCENE_QUERY_STAT(FreeCam), false, this);
        //    BoxParams.AddIgnoredActor(OutVT.Target);
        //    FHitResult Result;

        //    GetWorld()->SweepSingleByChannel(Result, Loc, Pos, FQuat::Identity, ECC_Camera, FCollisionShape::MakeBox(FVector(12.f)), BoxParams);
        //    OutVT.POV.Location = !Result.bBlockingHit ? Pos : Result.Location;
        //    OutVT.POV.Rotation = Rotator;

        //    // don't apply modifiers when using this debug camera mode
        //    bDoNotApplyModifiers = true;
        //}
        else
        {
            UpdateViewTargetInternal(OutVT, DeltaTime);
        }
    }

    if (!bDoNotApplyModifiers || bAlwaysApplyModifiers)
    {
        // Apply camera modifiers at the end (view shakes for example)
        ApplyCameraModifiers(DeltaTime, OutVT.POV);
    }
    
    SetActorLocation(OutVT.POV.Location);
    SetActorRotation(OutVT.POV.Rotation);
}

void APlayerCameraManager::SetViewTarget(AActor* NewViewTarget, FViewTargetTransitionParams TransitionParams)
{
    if (NewViewTarget != ViewTarget.Target || (PendingViewTarget.Target && BlendParams.bLockOutgoing))
    {
        if (TransitionParams.BlendTime > 0)
        {
            if (PendingViewTarget.Target == nullptr)
            {
                PendingViewTarget.Target = ViewTarget.Target;
            }

            ViewTarget.POV = GetLastFrameCameraCacheView();
            BlendTimeToGo = TransitionParams.BlendTime;

            //AssignViewTarget(NewTarget, PendingViewTarget, TransitionParams);
        }
        else
        {
            //AssignViewTarget(NewTarget, PendingViewTarget, TransitionParams);
            PendingViewTarget.Target = nullptr;
        }
    }
    else
    {
        if (PendingViewTarget.Target != nullptr)
        {
            //if (!PCOwner->IsPendingKillPending() && !PCOwner->IsLocalPlayerController() && GetNetMode() != NM_Client)
            //{
                //PCOwner->ClientSetViewTarget(NewTarget, TransitionParams);
            //}
        }
        PendingViewTarget.Target = nullptr;
    }

    // update the blend params after all the assignment logic so that sub-classes can compare
    // the old vs new parameters if needed.
    BlendParams = TransitionParams;
}

void APlayerCameraManager::ProcessViewRotation(float DeltaTime, FRotator& OutViewRotation, FRotator& OutDeltaRot)
{
    for( int32 ModifierIdx = 0; ModifierIdx < ModifierList.Num(); ModifierIdx++ )
    {
        if( ModifierList[ModifierIdx] != nullptr && 
            !ModifierList[ModifierIdx]->IsDisabled() )
        {
            if( ModifierList[ModifierIdx]->ProcessViewRotation(ViewTarget.Target, DeltaTime, OutViewRotation, OutDeltaRot) )
            {
                break;
            }
        }
    }

    // Limit Player View Axes
    LimitViewPitch( OutViewRotation, ViewPitchMin, ViewPitchMax );
    LimitViewYaw( OutViewRotation, ViewYawMin, ViewYawMax );
    LimitViewRoll( OutViewRotation, ViewRollMin, ViewRollMax );
}

void APlayerCameraManager::StartCameraFade(float FromAlpha, float ToAlpha, float InFadeTime, FLinearColor InFadeColor, bool bShouldFadeAudio,  bool bInHoldWhenFinished)
{
    bEnableFading = true;

    FadeColor = InFadeColor;
    FadeAlpha = FVector2D(FromAlpha, ToAlpha);
    FadeTime = InFadeTime;
    FadeTimeRemaining = InFadeTime;

    bAutoAnimateFade = true;
    bHoldFadeWhenFinished = bInHoldWhenFinished;
}

void APlayerCameraManager::StopCameraFade()
{
    if (bEnableFading == true)
    {
        // Make sure FadeAmount finishes at the desired value
        FadeAmount = FadeAlpha.Y;
        bEnableFading = false;
    }
}

void APlayerCameraManager::LimitViewPitch(FRotator& ViewRotation, float InViewPitchMin, float InViewPitchMax)
{
    ViewRotation.Pitch = JungleMath::ClampAngle(ViewRotation.Pitch, InViewPitchMin, InViewPitchMax);
    ViewRotation.Pitch = FRotator::ClampAxis(ViewRotation.Pitch);
}

void APlayerCameraManager::LimitViewRoll(FRotator& ViewRotation, float InViewRollMin, float InViewRollMax)
{
    ViewRotation.Roll = JungleMath::ClampAngle(ViewRotation.Roll, InViewRollMin, InViewRollMax);
    ViewRotation.Roll = FRotator::ClampAxis(ViewRotation.Roll);
}

void APlayerCameraManager::LimitViewYaw(FRotator& ViewRotation, float InViewYawMin, float InViewYawMax)
{
    ViewRotation.Yaw = JungleMath::ClampAngle(ViewRotation.Yaw, InViewYawMin, InViewYawMax);
    ViewRotation.Yaw = FRotator::ClampAxis(ViewRotation.Yaw);
}

void APlayerCameraManager::UpdateViewTargetInternal(FViewTarget& OutVT, float DeltaTime)
{
    if (OutVT.Target)
    {
        FVector OutLocation;
        FRotator OutRotation;
        float OutFOV;

        if (LuaUpdateCamera(OutVT.Target, OutLocation, OutRotation, OutFOV))
        {
            OutVT.POV.Location = OutLocation;
            OutVT.POV.Rotation = OutRotation;
            OutVT.POV.FOV = OutFOV;
        }
        else
        {
            OutVT.Target->CalcCamera(DeltaTime, OutVT.POV);
        }
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
