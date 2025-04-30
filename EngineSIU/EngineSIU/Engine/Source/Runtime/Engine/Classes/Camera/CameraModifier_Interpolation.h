#pragma once
#include "CameraModifier.h"
#include "CameraTypes.h"

struct FViewTarget;

class UCameraModifier_Interpolation : public UCameraModifier
{
    DECLARE_CLASS(UCameraModifier_Interpolation, UCameraModifier)
public:
    UCameraModifier_Interpolation();
    ~UCameraModifier_Interpolation() override;
    UObject* Duplicate(UObject* InOuter) override;

    /**
    * 보간 초기화
    * @param InFrom     보간 시작 뷰 타겟
    *  @param InTo       보간 종료 뷰 타겟
    * @param InDuration 보간 소요 시간(초)
    */
    void Initialize(const FViewTarget& InFrom, const FViewTarget& InTo, float InDuration);

protected:
    /** 보간 시작 시점 POV */
    FMinimalViewInfo StartPOV;
    /** 보간 목표 시점 POV */
    FMinimalViewInfo EndPOV;
    /** 전체 보간 시간 */
    float TotalTime;
    /** 누적 경과 시간 */
    float Elapsed;

protected:
    float GetTargetAlpha() override;

public:
    void AddedToCamera(APlayerCameraManager* Camera) override;
    /**
     * 매 프레임 보간 수행
     * @param DeltaTime   마지막 업데이트 이후 경과 시간
     * @param InOutPOV    수정할 카메라 POV 정보
     * @return             true 반환 시 이후 모디파이어 체인 중단, false면 계속
     */
    bool ModifyCamera(float DeltaTime, FMinimalViewInfo& InOutPOV) override;
    bool IsDisabled() const override;
    bool IsPendingDisable() const override;
    AActor* GetViewTarget() const override;
    void DisableModifier(bool bImmediate) override;
    void EnableModifier() override;
    void ToggleModifier() override;
    bool ProcessViewRotation(AActor* ViewTarget, float DeltaTime, FRotator& OutViewRotation, FRotator& OutDeltaRot) override;
    void UpdateAlpha(float DeltaTime) override;
    void ModifyCamera(float DeltaTime, FVector ViewLocation, FRotator ViewRotation, float FOV, FVector& NewViewLocation, FRotator& NewViewRotation,
        float& NewFOV) override;
};
