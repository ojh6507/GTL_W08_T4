#pragma once
#include "Components/SceneComponent.h"
#include "CameraTypes.h"

struct FMinimalViewInfo;

class UCameraComponent : public USceneComponent
{
    DECLARE_CLASS(UCameraComponent, USceneComponent)

public:
    UCameraComponent() = default;
    ~UCameraComponent() override;

    void UninitializeComponent() override;
    void BeginPlay() override;
    void OnComponentDestroyed() override;
    
    void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
    UObject* Duplicate(UObject* InOuter) override;
    void GetProperties(TMap<FString, FString>& OutProperties) const override;
    void SetProperties(const TMap<FString, FString>& InProperties) override;
    
    void InitializeComponent() override;
    void TickComponent(float DeltaTime) override;
    void DestroyComponent() override;

    float GetViewFovRadians() const { return FMath::DegreesToRadians(ViewFOV); }
    void SetViewFovRadians(const float InViewFOV) { ViewFOV = FMath::RadiansToDegrees(InViewFOV) ; }
    
    float GetVeiwFovDegrees() const { return ViewFOV; }
    void SetViewFovDegrees(const float InViewFOV) { ViewFOV = InViewFOV; }

    float GetNearClip() const { return NearClip; }
    void SetNearClip(const float InNearClip) { NearClip = InNearClip; }
    
    float GetFarClip() const { return FarClip; }
    void SetFarClip(const float InFarClip) { FarClip = InFarClip; }

    FMatrix GetViewMatrix() const  { return View; }
    FMatrix GetProjectionMatrix() const { return Projection; }

    virtual void GetCameraView(float DeltaTime, FMinimalViewInfo& DesiredView);

    FString CameraName;
private:
    void UpdateViewMatrix();
    void UpdateProjectionMatrix();

private:
    // 카메라 정보 
    float ViewFOV = 90.0f;
    float NearClip = 0.1f;
    float FarClip = 1000.0f;

    /** 뷰 평면(near/far plane)을 수동으로 조정합니다. 양수는 Far plane 방향, 음수는 Near plane 방향으로 이동 */
    float AutoPlaneShift;

    /** 종횡비(가로/세로 비율) */
    float AspectRatio;

    /** 로컬 플레이어에 정의된 기본 종횡비 축 제약을 재정의할지 여부 */
    EAspectRatioAxisConstraint AspectRatioAxisConstraint = EAspectRatioAxisConstraint::AspectRatio_MaintainXFOV;

    /** 
     * bConstrainAspectRatio가 true일 때,
     * 이 카메라가 요청한 종횡비와 다른 해상도일 경우 검은 여백(레터박스)을 추가할지 여부 
     */
    bool bConstrainAspectRatio = false;

    /** 로컬 플레이어에 정의된 기본 종횡비 축 제약을 재정의할지 여부 */
    bool bOverrideAspectRatioAxisConstraint = false;

    /** 뷰 프러스텀을 확대할 비율 (0.0 = 확대 없음, 1.0 = 100% 확대) */
    float Overscan = 0.0f;

    /**
     * 오버스캔 양만큼 해상도를 스케일하여 원래 프러스텀 해상도를 유지할지 여부.
     * 활성화 시 오버스캔이 늘어날수록 렌더링 부하가 증가하여 성능이 저하될 수 있습니다.
     */
    bool bScaleResolutionWithOverscan = false;

    /**
     * 렌더링 파이프라인 마지막 단계에서 오버스캔된 픽셀을 크롭하여 화면에 보이지 않게 할지 여부.
     * (예: 렌즈 왜곡 같은 포스트프로세스 효과에 프러스텀 외 픽셀이 필요할 때 사용)
     * bScaleResolutionWithOverscan이 true일 때는 크롭된 이미지 해상도가 원본과 동일하며,
     * false일 때는 더 낮은 해상도로 출력됩니다.
     */
    bool bCropOverscan = false;

    /** True to enable the additive view offset, for adjusting the view without moving the component. */
    bool bUseAdditiveOffset = false;

    ECameraProjectionMode::Type ProjectionMode;

    bool bUseFieldOfViewForLOD;

    /** An optional extra FOV offset to adjust the final view without modifying the component */
    float AdditiveFOVOffset;

    /** Indicates if PostProcessSettings should be used when using this Camera to view through. */
    float PostProcessBlendWeight;
    
    //FPostProcessSettings PostProcessSettings;

    FMatrix View = FMatrix::Identity;
    FMatrix Projection = FMatrix::Identity;
};
