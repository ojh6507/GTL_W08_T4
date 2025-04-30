#include "CameraModifier_Interpolation.h"

#include "ViewTarget.h"

UCameraModifier_Interpolation::UCameraModifier_Interpolation()
{
}

UCameraModifier_Interpolation::~UCameraModifier_Interpolation()
{
}

UObject* UCameraModifier_Interpolation::Duplicate(UObject* InOuter)
{
    return Super::Duplicate(InOuter);
}

void UCameraModifier_Interpolation::Initialize(const FViewTarget& InFrom, const FViewTarget& InTo, float InDuration)
{
    StartPOV = InFrom.POV;
    EndPOV = InTo.POV;
    TotalTime = FMath::Max(InDuration, KINDA_SMALL_NUMBER);
    Elapsed = 0.f;
    bDisabled = false;
    bPendingDisable = false;
}

float UCameraModifier_Interpolation::GetTargetAlpha()
{
    return Super::GetTargetAlpha();
}

void UCameraModifier_Interpolation::AddedToCamera(APlayerCameraManager* Camera)
{
    Super::AddedToCamera(Camera);
}

bool UCameraModifier_Interpolation::ModifyCamera(float DeltaTime, FMinimalViewInfo& InOutPOV)
{
    UpdateAlpha(DeltaTime);

    ModifyCamera(DeltaTime, InOutPOV.Location, InOutPOV.Rotation, InOutPOV.FOV, InOutPOV.Location, InOutPOV.Rotation, InOutPOV.FOV);

    LuaModifyCamera(DeltaTime, InOutPOV.Location, InOutPOV.Rotation, InOutPOV.FOV, InOutPOV.Location, InOutPOV.Rotation, InOutPOV.FOV);
    
    // 경과 시간 누적
    Elapsed += DeltaTime;
    float t = FMath::Clamp(Elapsed / TotalTime, 0.f, 1.f);

    // 위치 선형 보간
    InOutPOV.Location = FMath::Lerp(StartPOV.Location, EndPOV.Location, t);
    
    // 회전 선형 보간 (간단화를 위해 각각의 컴포넌트 보간)
    InOutPOV.Rotation.Pitch = FMath::Lerp(StartPOV.Rotation.Pitch, EndPOV.Rotation.Pitch, t);
    InOutPOV.Rotation.Yaw   = FMath::Lerp(StartPOV.Rotation.Yaw,   EndPOV.Rotation.Yaw,   t);
    InOutPOV.Rotation.Roll  = FMath::Lerp(StartPOV.Rotation.Roll,  EndPOV.Rotation.Roll,  t);

    // FOV 보간
    InOutPOV.FOV = FMath::Lerp(StartPOV.FOV, EndPOV.FOV, t);

    // 보간 완료 시 모디파이어 비활성화
    if (Elapsed >= TotalTime)
    {
        DisableModifier(true);
    }

    // 계속 체인 실행
    return false;
}

bool UCameraModifier_Interpolation::IsDisabled() const
{
    return Super::IsDisabled();
}

bool UCameraModifier_Interpolation::IsPendingDisable() const
{
    return Super::IsPendingDisable();
}

AActor* UCameraModifier_Interpolation::GetViewTarget() const
{
    return Super::GetViewTarget();
}

void UCameraModifier_Interpolation::DisableModifier(bool bImmediate)
{
    Super::DisableModifier(bImmediate);
}

void UCameraModifier_Interpolation::EnableModifier()
{
    Super::EnableModifier();
}

void UCameraModifier_Interpolation::ToggleModifier()
{
    Super::ToggleModifier();
}

bool UCameraModifier_Interpolation::ProcessViewRotation(AActor* ViewTarget, float DeltaTime, FRotator& OutViewRotation, FRotator& OutDeltaRot)
{
    return Super::ProcessViewRotation(ViewTarget, DeltaTime, OutViewRotation, OutDeltaRot);
}

void UCameraModifier_Interpolation::UpdateAlpha(float DeltaTime)
{
    Super::UpdateAlpha(DeltaTime);
}

void UCameraModifier_Interpolation::ModifyCamera(float DeltaTime, FVector ViewLocation, FRotator ViewRotation, float FOV, FVector& NewViewLocation,
    FRotator& NewViewRotation, float& NewFOV)
{
    Super::ModifyCamera(DeltaTime, ViewLocation, ViewRotation, FOV, NewViewLocation, NewViewRotation, NewFOV);
}
