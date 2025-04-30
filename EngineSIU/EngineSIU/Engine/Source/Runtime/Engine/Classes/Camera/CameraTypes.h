#pragma once
#include <optional>

#include "EngineBaseTypes.h"
#include "Math/Vector.h"

namespace ECameraProjectionMode
{
    enum Type : int
    {
        Perspective,
        Orthographic
    };
}

enum class ECameraShakePlaySpace
{
    /** 카메라 로컬 공간에서 적용되는 흔들림 애니메이션 */
    CameraLocal,
    /** 월드 공간에서 적용되는 흔들림 애니메이션 */
    World,
    /** 사용자 정의 공간(UserPlaySpaceMatrix)에서 적용되는 흔들림 애니메이션 */
    UserDefined,
};

struct FMinimalViewInfo
{
    FVector Location;

    /** 회전(회전각) */
    FRotator Rotation;

    /** 원근 모드에서 수평 시야각(FOV, 도 단위). 직교 모드에서는 무시됨 */
    float FOV;

    /** 화면 비율 조정을 반영하기 전의 원래 시야각 */
    float DesiredFOV;

    /** “IsFirstPerson” 태그가 붙은 프리미티브에 사용할 수평 시야각(도 단위) */
    float FirstPersonFOV;

    /** 
     * “IsFirstPerson” 태그가 붙은 프리미티브에 적용할 스케일.
     * 씬과 겹치지 않도록 카메라 쪽으로 축소하여 렌더링할 때 사용.
     */
    float FirstPersonScale;

    /** 직교 모드에서 보이는 뷰 너비(월드 단위). 원근 모드에서는 무시됨 */
    float OrthoWidth;

    /** 직교 카메라 Near/Far 평면을 자동 계산할지 여부 */
    bool bAutoCalculateOrthoPlanes;

    /** Near/Far 평면을 수동으로 조정. 양수→FarPlane 쪽 확장, 음수→NearPlane 쪽 이동 */
    float AutoPlaneShift;

    /** 클리핑 및 라이트 아티팩트를 방지하기 위해 Near/Far 평면 및 뷰 원점을 자동 보정할지 여부 */
    bool bUpdateOrthoPlanes;

    /** bUpdateOrthoPlanes가 true일 때, 카메라 높이를 뷰타겟 거리로 대신 사용할지 여부 */
    bool bUseCameraHeightAsViewTarget;

    /** 직교 모드에서 Near 평면 거리(월드 단위) */
    float OrthoNearClipPlane;

    /** 직교 모드에서 Far 평면 거리(월드 단위) */
    float OrthoFarClipPlane;

    /** 원근 모드에서 Near 평면 거리(월드 단위). 음수→GNearClippingPlane 전역값 사용 */
    float PerspectiveNearClipPlane;

    // 화면 비율(가로/세로)
    float AspectRatio;

    // 화면 비율 축(axis) 제약 방식
    std::optional<EAspectRatioAxisConstraint> AspectRatioConstraint;

    /** 화면 비율이 다를 때 검은 여백(레터박스)을 추가할지 여부 */
    uint32 bConstrainAspectRatio; 

    /** true면 “IsFirstPerson” 태그 프리미티브에 FirstPersonFOV와 FirstPersonScale 적용 */
    uint32 bUseFirstPersonParameters;

    /** true면 LOD 선택 시 시야각을 고려 */
    uint32 bUseFieldOfViewForLOD;

    /** 카메라 타입(원근/직교) */
    ECameraProjectionMode::Type ProjectionMode;

    /** 후처리 설정(PostProcessSettings)을 적용할지 여부 */
    float PostProcessBlendWeight;
    
    /** 화면 중심에서 벗어난 투영(off-axis/off-center) 오프셋(화면 비율 기준) */
    FVector2D OffCenterProjectionOffset;

    /** 이전 프레임의 변환(위치/회전/스케일)을 선택적으로 저장 */
    std::optional<FVector> PreviousViewLocation;
    std::optional<FRotator> PreviousViewRotation;
    std::optional<FVector> PreviousViewScale;

    /** 오버스캔(overscan) 시 프러스텀 확장에 따라 해상도를 스케일할 비율 */
    float OverscanResolutionFraction;
    
    /** 최종 후처리 단계에서 크롭할 뷰 비율(0.0~1.0, 1.0은 크롭 없음) */
    float CropFraction;
    
private:
    /** 적용된 오버스캔 비율(0.0: 없음, 1.0: 100% 확장) */
    float Overscan;
    // 직교 카메라 자동 평면 계산에서만 사용, Near 평면에 추가할 거리
    FVector CameraToViewTarget;

public:

    FMinimalViewInfo()
        : Location()
        , Rotation()
        , FOV(90.0f)
        , DesiredFOV(90.0f)
        , FirstPersonFOV(90.0f)
        , FirstPersonScale(1.0f)
        , OrthoWidth(512.0f)
        , bAutoCalculateOrthoPlanes(true)
        , AutoPlaneShift(0.0f)
        , bUpdateOrthoPlanes(false)
        , bUseCameraHeightAsViewTarget(false)
        , OrthoNearClipPlane(0.0f)
        , OrthoFarClipPlane(2097152.0)
        , PerspectiveNearClipPlane(-1.0f)
        , AspectRatio(1.33333333f)
        , bConstrainAspectRatio(false)
        , bUseFirstPersonParameters(false)
        , bUseFieldOfViewForLOD(true)
        , ProjectionMode(ECameraProjectionMode::Perspective)
        , PostProcessBlendWeight(0.0f)
        , OffCenterProjectionOffset(FVector2D::ZeroVector)
        , OverscanResolutionFraction(1.0f)
        , CropFraction(1.0f)
        , Overscan(0.0f)
        , CameraToViewTarget(FVector::ZeroVector)
    {
    }

    bool Equals(const FMinimalViewInfo& OtherInfo) const;

    // 뷰 정보를 블렌딩(가중치 블렌딩)합니다. 불리언은 OR 연산합니다.
    void BlendViewInfo(FMinimalViewInfo& OtherInfo, float OtherWeight);

    // 이 뷰에 가중치를 적용합니다(이 *= Weight).
    void ApplyBlendWeight(const float& Weight);

    // 다른 뷰를 가중치와 함께 더합니다(이 += OtherView * Weight).
    void AddWeightedViewInfo(const FMinimalViewInfo& OtherView, const float& Weight);

    /** 현재 PerspectiveNearClipPlane 값(양수) 또는 GNearClippingPlane 전역값을 반환 */
    FORCEINLINE float GetFinalPerspectiveNearClipPlane() const
    {
        return PerspectiveNearClipPlane > 0.0f ? PerspectiveNearClipPlane : 10.f;
    }

    /** AutoCalculateOrthoPlanes용 카메라→뷰타겟 거리 설정 */
    FORCEINLINE void SetCameraToViewTarget(const FVector ActorLocation)
    {
        CameraToViewTarget = ActorLocation - Location;
    }

    /**
    * 화면 영역을 확장해 프러스텀(frustrum)을 키우는 기능입니다.
    *
    * @param InOverscan
    *   0.0 → 원래 크기 그대로,  
    *   1.0 → 100% 더 확장된 뷰(실제 FOV·OrthoWidth가 두 배)
    *
    * @param bScaleResolutionWithOverscan
    *   true  → 뷰 해상도도 함께 확장, 원래 프러스텀 해상도를 유지  
    *   false → 해상도는 그대로 두고 프러스텀만 확장
    *
    * @param bCropOverscan
    *   true  → 최종 포스트프로세스 단계에서 확장된 영역을 잘라내 화면에 보이지 않게 함  
    *   false → 확장된 영역까지 모두 렌더링
    */
    void ApplyOverscan(float InOverscan, bool bScaleResolutionWithOverscan = false, bool bCropOverscan = false);

    /** 현재 뷰에 적용된 오버스캔 비율을 반환 (0.0 ~ 1.0) */
    float GetOverscan() const { return Overscan; }

    /** 뷰에 적용된 모든 오버스캔을 제거해 원래 프러스텀으로 되돌림 */
    void ClearOverscan();
};
