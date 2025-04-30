#pragma once
#include "Components/SceneComponent.h"
#include "UObject/ObjectMacros.h" // DECLARE_CLASS

class USpringArmComponent : public USceneComponent
{
    DECLARE_CLASS(USpringArmComponent, USceneComponent)

public:
    USpringArmComponent();

    UPROPERTY(float, TargetArmLength, = 30.0f)  // 기본 팔 길이

    // --- 핵심 로직 ---
    virtual void TickComponent(float DeltaTime) override;

protected:
    // 자식 컴포넌트의 최종 트랜스폼을 업데이트하는 함수
    void UpdateChildTransforms();
};
