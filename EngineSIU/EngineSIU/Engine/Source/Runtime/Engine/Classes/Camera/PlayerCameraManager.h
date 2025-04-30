#pragma once
#include "CameraTypes.h"
#include "GameFramework/Actor.h"
#include "ViewTarget.h"

class APlayerController;
class UCameraModifier;
class UCameraComponent;

enum EViewTargetBlendFunction : int
{
    /** Camera does a simple linear interpolation. */
    VTBlend_Linear,
    /** Camera has a slight ease in and ease out, but amount of ease cannot be tweaked. */
    VTBlend_Cubic,
    /** Camera immediately accelerates, but smoothly decelerates into the target.  Ease amount controlled by BlendExp. */
    VTBlend_EaseIn,
    /** Camera smoothly accelerates, but does not decelerate into the target.  Ease amount controlled by BlendExp. */
    VTBlend_EaseOut,
    /** Camera smoothly accelerates and decelerates.  Ease amount controlled by BlendExp. */
    VTBlend_EaseInOut,
    /** The game's camera system has already performed the blending. Engine should not blend at all */
    VTBlend_PreBlended,
    VTBlend_MAX,
};

struct FCameraCacheEntry
{
public:

    /** World time this entry was created. */
    float TimeStamp;

    /** Camera POV to cache. */
    FMinimalViewInfo POV;

    FCameraCacheEntry()
        : TimeStamp(0.f)
    {}
};


struct FViewTargetTransitionParams
{
public:

    /** Total duration of blend to pending view target. 0 means no blending. */
    float BlendTime;

    /** Function to apply to the blend parameter. */
    EViewTargetBlendFunction BlendFunction;

    /** Exponent, used by certain blend functions to control the shape of the curve. */
    float BlendExp;

    /** 
     * If true, lock outgoing viewtarget to last frame's camera POV for the remainder of the blend.
     * This is useful if you plan to teleport the old viewtarget, but don't want to affect the blend. 
     */
    uint32 bLockOutgoing:1;

    FViewTargetTransitionParams()
        : BlendTime(0.f)
        , BlendFunction(VTBlend_Cubic)
        , BlendExp(2.f)
        , bLockOutgoing(false)
    {}

    /** For a given linear blend value (blend percentage), return the final blend alpha with the requested function applied */
    float GetBlendAlpha(const float& TimePct) const
    {
        switch (BlendFunction)
        {
        case VTBlend_Linear: return FMath::Lerp(0.f, 1.f, TimePct); 
        //case VTBlend_Cubic:	return FMath::CubicInterp(0.f, 0.f, 1.f, 0.f, TimePct); 
        case VTBlend_EaseInOut: return FMath::InterpEaseInOut(0.f, 1.f, TimePct, BlendExp); 
        case VTBlend_EaseIn: return FMath::Lerp(0.f, 1.f, FMath::Pow(TimePct, BlendExp)); 
        case VTBlend_EaseOut: return FMath::Lerp(0.f, 1.f, FMath::Pow(TimePct, (FMath::IsNearlyZero(BlendExp) ? 1.f : (1.f / BlendExp))));
        default:
            break;
        }

        return 1.f;
    }
};

class APlayerCameraManager : public AActor
{
    DECLARE_CLASS(APlayerCameraManager, AActor)
public:
    APlayerCameraManager();
private:
    USceneComponent* TransformComponent;

public:
    UObject* Duplicate(UObject* InOuter) override;
    void BeginPlay() override;
    void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
    
    FName CameraStyle;

    // Radian인지 Degree인지 확인 필수
    float DefaultFOV;

    float GetLockedFOV() const;
protected:
    float LockedFOV;
    
protected:
    /** > 0일 경우 OrthoWidth를 고정시키는 값. <= 0이면 무시됨 */
    float LockedOrthoWidth;

public:
    /** 기본 종횡비. 대부분의 경우 카메라 컴포넌트에서 값을 사용함 */
    float DefaultAspectRatio;
    
    /** 화면이 점점 사라질 때 사용할 색상 (bEnableFading이 true일 때 적용) */
    FLinearColor FadeColor;

    /** 화면 사라짐 효과의 강도 (bEnableFading이 true일 때 적용) */
    float FadeAmount;

    /** 카메라 페이드 알파 범위, X = 시작 알파, Y = 최종 알파 (bEnableFading이 true일 때 적용) */
    FVector2D FadeAlpha;

    /** 카메라 페이드의 전체 지속 시간 (bEnableFading이 true일 때 적용) */
    float FadeTime;

    /** 카메라 페이드에 남은 시간 (bEnableFading이 true일 때 적용) */
    float FadeTimeRemaining = 0.f;

    /** 현재 뷰 타겟 */
    FViewTarget ViewTarget;

    UCameraComponent* CurCameraComp;

    /** 블렌딩 중인 뷰 타겟 */
    FViewTarget PendingViewTarget;

    /** 뷰 타겟 블렌딩에 남은 시간 */
    float BlendTimeToGo = 0;

    /** 현재 뷰 타겟 전환 시의 블렌드 파라미터 */
    struct FViewTargetTransitionParams BlendParams;

    /** 이 카메라가 FOV 대신 정사영(Orthographic) 시점을 사용해야 할 경우 true */
    uint32 bIsOrthographic = false;

    /** 이 카메라가 Near/Far 평면을 자동으로 계산해야 할 경우 true */
    uint32 bAutoCalculateOrthoPlanes = false;

    /** Near/Far 평면 사이의 거리를 유지하면서 평면을 수동으로 조절하는 값. 양수는 far plane 방향으로, 음수는 near plane 방향으로 이동 */
    float AutoPlaneShift;

    /** 목적지 뷰의 종횡비가 다를 경우 검은 막대를 추가할지 여부 (뷰 타겟에서 종횡비 제한 여부를 명시하지 않을 때만 사용됨. 대부분은 카메라 컴포넌트의 설정을 따름) */
    uint32 bDefaultConstrainAspectRatio = false;

    /** 화면에 FadeColor/FadeAmount를 적용할지 여부 */
    uint32 bEnableFading = false;

    /**
     * 이번 프레임에 카메라 컷(뷰 타겟이 갑자기 바뀌는 것)이 발생했는지
     * 컷이 감지되면 한 프레임만 true가 되며, 이 값은 프레임 끝에 자동으로 false로 리셋됩니다.
     * 렌더러 차원에서는 이전 프레임의 옥클루전 쿼리 사용 여부나 모션 블러 처리 등에 영향을 줍니다.
     */
    uint32 bGameCameraCutThisFrame = false;

 public:
     /** ViewTarget이 PendingViewTarget으로 설정될 때 발생하는 이벤트 */
     DECLARE_EVENT(APlayerCameraManager, FOnBlendComplete)
     FOnBlendComplete& OnBlendComplete() const { return OnBlendCompleteEvent; }
 private:
     mutable FOnBlendComplete OnBlendCompleteEvent;

protected:
    /** 페이드가 끝난 후에도 최종 컬러·알파를 유지할지 여부 (hold at final) */
    uint32 bHoldFadeWhenFinished;

    uint32 bAutoAnimateFade;
    
    /**
     * 디버그용 카메라(예: FreeCam)에도 모디파이어를 항상 적용할지 여부.
     * 기본값은 false여서 디버그 카메라에는 모디파이어가 적용되지 않음.
     */
    uint32 bAlwaysApplyModifiers = 1;

public:
    /** Minimum view pitch, in degrees. */
    float ViewPitchMin;

    /** Maximum view pitch, in degrees. */
    float ViewPitchMax;

    /** Minimum view yaw, in degrees. */
    float ViewYawMin;

    /** Maximum view yaw, in degrees. */
    float ViewYawMax;

    /** Minimum view roll, in degrees. */
    float ViewRollMin;

    /** Maximum view roll, in degrees. */
    float ViewRollMax;

 private:
     /** 캐시된 카메라 정보 */
     struct FCameraCacheEntry CameraCachePrivate;

     /** 한 프레임 전의 캐시된 카메라 정보 */
     struct FCameraCacheEntry LastFrameCameraCachePrivate;
protected:
    FName ActiveCameraName;
    /** 최종 카메라 POV(Point of View)를 갱신할 수 있는 활성 카메라 모디파이어 인스턴스 목록 */
    TArray<UCameraModifier*> ModifierList;

public:
    /**
    * ModifierList의 복사본에 대해 주어진 함수를 실행합니다. 조기 종료가 가능함.
    * 람다와 함께 사용하기 쉬움:
    * ForEachCameraModifier([](UCameraModifier* Proxy) -> bool
    * {
    *     return continueLoop ? true : false;
    * });
    */
    void ForEachCameraModifier(const std::function<bool(UCameraModifier*)>& Fn) const;

    /** 카메라에 대해 기본적으로 생성할 모디파이어 목록 */
    TArray<UCameraModifier*> DefaultModifiers;
    
    /** CameraCachePrivate.POV 값을 설정함 */
    virtual void SetCameraCachePOV(const FMinimalViewInfo& InPOV);
	
    /** LastFrameCameraCachePrivate.POV 값을 설정함 */
    virtual void SetLastFrameCameraCachePOV(const FMinimalViewInfo& InPOV);

    /** CameraCachePrivate.POV 값을 반환함 */
    virtual const FMinimalViewInfo& GetCameraCacheView() const;

    /** LastFrameCameraCachePrivate.POV 값을 반환함 */
    virtual const FMinimalViewInfo& GetLastFrameCameraCacheView() const;

    /** CameraCachePrivate.POV 값을 복사 반환함 */
    virtual FMinimalViewInfo GetCameraCachePOV() const;

    /** LastFrameCameraCachePrivate.POV 값을 복사 반환함 */
    virtual FMinimalViewInfo GetLastFrameCameraCachePOV() const;

    /** CameraCachePrivate의 타임스탬프 반환 */
    float GetCameraCacheTime() const { return CameraCachePrivate.TimeStamp; }

    /** LastFrameCameraCachePrivate의 타임스탬프 반환 */
    float GetLastFrameCameraCacheTime() const { return LastFrameCameraCachePrivate.TimeStamp; }

protected:
    /** CameraCachePrivate의 타임스탬프 설정 */
    void SetCameraCacheTime(float InTime) { CameraCachePrivate.TimeStamp = InTime; }

    /** LastFrameCameraCachePrivate의 타임스탬프 설정 */
    void SetLastFrameCameraCacheTime(float InTime) { LastFrameCameraCachePrivate.TimeStamp = InTime; }

    // TODO : UCameraModifier_CameraShake
    /** 코드 기반 화면 흔들림 효과를 위한 캐시된 모디파이어 참조 */
    //class UCameraModifier_CameraShake* CachedCameraShakeMod;

protected:
    /** Internal. Receives the output of individual camera animations. */
    class ACameraActor* AnimCameraActor;

public:
    friend struct FViewTarget;

    AActor* GetViewTarget() const;
    void Tick(float deltaTime) override;

    void SetActiveCamera(const FName& name);

    // 모디파이어 관리
    void AddModifier(UCameraModifier* modifier);
    void RemoveModifier(UCameraModifier* modifier);

private:
    void ApplyCameraModifiers(float DeltaTime, FMinimalViewInfo& InOutPOV);

public:
    virtual float GetFOVAngle() const;
    virtual void SetFOVAngle(float FOVAngle);
    virtual void UnLockFOV();

    virtual void GetCameraViewPoint(FVector& OutCamLoc, FRotator& OutCamRot) const;

    virtual FRotator GetCameraRotation() const;

    virtual FVector GetCameraLocation() const;

    virtual void SetDesiredColorScale(FVector NewColorScale, float InterpTime);

protected:
    virtual void DoUpdateCamera(float DeltaTime);

    bool LuaUpdateCamera(AActor* CameraTarget, FVector& NewCameraLocation, FRotator& NewCameraRotation, float& NewCameraFOV);

    virtual void AssignViewTarget(AActor* NewTarget, FViewTarget& VT, struct FViewTargetTransitionParams TransitionParams=FViewTargetTransitionParams());

    /**
     * 두 개의 ViewTarget을 블렌딩하는 내부 헬퍼 함수입니다.
     *
     * @param A      시작(소스) 뷰 타겟
     * @param B      목표(데스티네이션) 뷰 타겟
     * @param Alpha  A에서 B로 얼만큼(blend 비율, 0.0~1.0) 이동할지 비율
     */
    FPOV BlendViewTargets(const FViewTarget& A, const FViewTarget& B, float Alpha);

    /** 
     * 최종 계산된 POV 정보를 캐시에 저장하여 다른 게임 코드에서 효율적으로 접근할 수 있게 합니다. 
     */
    void FillCameraCache(const FMinimalViewInfo& NewInfo);

    /**
     * 주어진 ViewTarget에 대해 업데이트된 POV를 계산합니다.
     *
     * @param OutVT      업데이트할 ViewTarget 참조
     * @param DeltaTime  마지막 카메라 업데이트 이후 경과된 시간(초 단위)
     */
    virtual void UpdateViewTarget(FViewTarget& OutVT, float DeltaTime);
public:
    void SetViewTarget(class AActor* NewViewTarget, FViewTargetTransitionParams TransitionParams = FViewTargetTransitionParams());

    virtual void ProcessViewRotation(float DeltaTime, FRotator& OutViewRotation, FRotator& OutDeltaRot);

    virtual void StartCameraFade(float FromAlpha, float ToAlpha, float Duration, FLinearColor Color, bool bShouldFadeAudio = false, bool bHoldWhenFinished = false);

    virtual void StopCameraFade();
    /** 
     * 플레이어의 시야 피치(pitch)를 제한합니다.
     * @param ViewRotation       수정할 뷰 회전값(입력 및 출력)
     * @param InViewPitchMin     허용할 최소 피치 각도(도 단위)
     * @param InViewPitchMax     허용할 최대 피치 각도(도 단위)
     */
    virtual void LimitViewPitch(FRotator& ViewRotation, float InViewPitchMin, float InViewPitchMax);

    /** 
     * 플레이어의 시야 롤(roll)을 제한합니다.
     * @param ViewRotation       수정할 뷰 회전값(입력 및 출력)
     * @param InViewRollMin      허용할 최소 롤 각도(도 단위)
     * @param InViewRollMax      허용할 최대 롤 각도(도 단위)
     */
    virtual void LimitViewRoll(FRotator& ViewRotation, float InViewRollMin, float InViewRollMax);

    /** 
     * 플레이어의 시야 요(yaw)를 제한합니다.
     * @param ViewRotation       수정할 뷰 회전값(입력 및 출력)
     * @param InViewYawMin       허용할 최소 요 각도(도 단위)
     * @param InViewYawMax       허용할 최대 요 각도(도 단위)
     */
    virtual void LimitViewYaw(FRotator& ViewRotation, float InViewYawMin, float InViewYawMax);

protected:
    virtual void UpdateViewTargetInternal(FViewTarget& OutVT, float DeltaTime);
};
