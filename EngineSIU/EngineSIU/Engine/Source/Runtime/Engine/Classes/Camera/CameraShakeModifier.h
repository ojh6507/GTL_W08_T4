#pragma once

#include "Camera/CameraModifier.h"


class UCameraShakeModifier : public UCameraModifier
{
    DECLARE_CLASS(UCameraShakeModifier, UCameraModifier)
public:
    UCameraShakeModifier();

    // 셰이크 속성
    // 셰이크 관련 속성
    float ShakeIntensity;       // 셰이크 강도
    float ShakeDuration;        // 총 지속 시간
    float ShakeTimeRemaining;   // 남은 셰이크 시간
    bool bIsShaking;            // 셰이크 활성화 상태
    bool bIsStart;

    FVector OriginLocation;
    
    // 셰이크 시작 함수
    void StartShake(float Intensity, float Duration);

    // 셰이크 중지 함수
    void StopShake();

    bool ModifyCamera(float DeltaTime, struct FMinimalViewInfo& InOutPOV) override;

    //// CameraModifier 인터페이스 구현
    //virtual void ModifyCamera(float DeltaTime, FVector ViewLocation, FRotator ViewRotation, float FOV,
    //    FVector& NewLocation, FRotator& NewRotation, float& NewFOV) override;
};
