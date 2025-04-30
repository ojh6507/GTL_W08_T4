#pragma once

#include "UObject/Object.h"
#include "UObject/ObjectMacros.h"

class AActor;
class APlayerCameraManager;

enum class EModifierType 
{
    Shake,
};

class UCameraModifier : public UObject
{
    DECLARE_CLASS(UCameraModifier, UObject)

private:
    EModifierType MType;

public:
    UCameraModifier();
    ~UCameraModifier() override;
    UObject* Duplicate(UObject* InOuter) override;

    EModifierType GetModifierType();
    void SetModifierType(EModifierType type);
protected:
    /** true일 경우, 이 모디파이어는 카메라에 적용되지 않습니다. */
    uint32 bDisabled = true;

    /** true일 경우, 보간이 끝나면 이 모디파이어는 자동으로 비활성화됩니다. */
    uint32 bPendingDisable = true;
    
public:
    /** 모디파이어가 적용되는 순서를 결정하는 우선순위 값. 0 = 가장 높은 우선순위, 255 = 가장 낮은 우선순위 */
    uint8 Priority;

protected:
    /** 이 객체가 연결된 카메라 */
    APlayerCameraManager* CameraOwner;

    /** 블렌딩 인 시, 알파값이 0에서 1로 변화하는 데 걸리는 시간 */
    float AlphaInTime;

    /** 블렌딩 아웃 시, 알파값이 1에서 0으로 변화하는 데 걸리는 시간 */
    float AlphaOutTime;

    /** 현재 블렌딩 알파 값 */
    float Alpha;

    /** 목표 알파값을 반환하는 함수 */
    virtual float GetTargetAlpha();

public:
    /** 
     * 사용자 정의 초기화 처리를 위한 함수. 생성 직후 호출됨.
     * @param Camera - 이 모디파이어가 연결될 카메라
     */
    virtual void AddedToCamera(APlayerCameraManager* Camera);

    /**
     * 연결된 카메라의 변수를 직접 수정합니다.
     * @param DeltaTime - 마지막 업데이트 이후 경과 시간
     * @param InOutPOV - 수정 대상이 되는 현재 시점(Point of View)
     * @return bool - true 반환 시 이후 모디파이어 체인을 중단하고, false면 계속 실행
     */
    virtual bool ModifyCamera(float DeltaTime, struct FMinimalViewInfo& InOutPOV);

    /** 
     * 활성 상태인 모디파이어가 매 프레임마다 호출되어 블루프린트에서 카메라 변형을 적용할 수 있게 합니다.
     * 알파값 스케일링은 이 함수 이후 코드에서 처리되므로, 블루프린트에서는 처리할 필요 없습니다.
     * @param DeltaTime - 마지막 업데이트 이후 경과 시간
     * @param ViewLocation - 현재 카메라 위치
     * @param ViewRotation - 현재 카메라 회전
     * @param FOV - 현재 카메라 시야각
     * @param NewViewLocation - (출력) 수정된 카메라 위치
     * @param NewViewRotation - (출력) 수정된 카메라 회전
     * @param NewFOV - (출력) 수정된 시야각
     */
    void LuaModifyCamera(float DeltaTime, FVector ViewLocation, FRotator ViewRotation, float FOV, FVector& NewViewLocation, FRotator& NewViewRotation, float& NewFOV);
    
    /** @return 모디파이어가 비활성 상태인지 여부 반환 */
    virtual bool IsDisabled() const;

    /** @return 모디파이어가 비활성화 대기 중인지 여부 반환 */
    virtual bool IsPendingDisable() const;

    /** @return 현재 카메라가 바라보는 액터 반환 */
    virtual AActor* GetViewTarget() const;

    /** 
     * 이 모디파이어를 비활성화합니다.
     * @param bImmediate - true이면 바로 비활성화, false(기본값)이면 블렌드 아웃 과정을 거침
     */
    virtual void DisableModifier(bool bImmediate = false);

    /** 이 모디파이어를 활성화합니다. */
    virtual void EnableModifier();

    /** 이 모디파이어의 활성/비활성 상태를 토글합니다. */
    virtual void ToggleModifier();

    /**
     * 뷰 회전 값이 적용되기 전에 모디파이어가 회전 값을 조정할 수 있도록 호출됩니다.
     *
     * 기본 구현은 ViewRotation을 그대로 반환합니다.
     * @param ViewTarget - 현재 뷰 타겟
     * @param DeltaTime - 프레임 시간 (초)
     * @param OutViewRotation - 입력/출력: 수정 대상 회전 값
     * @param OutDeltaRot - 입력/출력: 해당 프레임 동안 회전한 값
     * @return true 반환 시 이후 우선순위가 낮은 모디파이어의 회전 수정 차단
     */
    virtual bool ProcessViewRotation(class AActor* ViewTarget, float DeltaTime, FRotator& OutViewRotation, FRotator& OutDeltaRot);

    /**
     * 알파 블렌드 값을 갱신하는 역할을 합니다.
     *
     * @param Camera - 업데이트 대상 카메라
     * @param DeltaTime - 마지막 업데이트 이후 경과 시간
     */
    virtual void UpdateAlpha(float DeltaTime);

    /** @return 이 객체에 적합한 월드 컨텍스트 반환 */
    UWorld* GetWorld() const;

    /** 네이티브 코드에서 카메라를 수정할 수 있게 해주는 함수 */
    virtual void ModifyCamera(float DeltaTime, FVector ViewLocation, FRotator ViewRotation, float FOV, FVector& NewViewLocation, FRotator& NewViewRotation, float& NewFOV);

};
