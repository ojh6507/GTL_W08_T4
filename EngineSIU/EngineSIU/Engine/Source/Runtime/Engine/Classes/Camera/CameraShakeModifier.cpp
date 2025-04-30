#include "CameraShakeModifier.h"

#include <cstdlib>
#include "Engine/Classes/Camera/CameraTypes.h"

UCameraShakeModifier::UCameraShakeModifier()
{
    Priority = 10;  // 중간 우선순위

    ShakeIntensity = 0.f;
    ShakeDuration = 0.f;
    ShakeTimeRemaining = 0.f; // 남은 시간을 총 지속 시간으로 초기화
    bIsShaking = false;
}

void UCameraShakeModifier::StartShake(float Intensity, float Duration)
{
    ShakeIntensity = Intensity;
    ShakeDuration = Duration;
    ShakeTimeRemaining = Duration;  // 남은 시간을 총 지속 시간으로 초기화
    bIsShaking = true;
    bIsStart = true;

    // 모디파이어 활성화
    EnableModifier();
}

void UCameraShakeModifier::StopShake()
{
    bIsShaking = false;
    ShakeTimeRemaining = 0.0f;
    ShakeIntensity = 0.0f;
    bIsStart = false;
    DisableModifier(false);  // 부드럽게 페이드 아웃
}

bool UCameraShakeModifier::ModifyCamera(float DeltaTime, FMinimalViewInfo& InOutPOV)
{
    if (bIsStart)
    {
        OriginLocation = InOutPOV.Location;
        bIsStart = false;
    }
    
    if (!bIsShaking)
    {
        return false;
    }

    // 시간 감소
    ShakeTimeRemaining -= DeltaTime;

    // 셰이크 시간 종료 체크
    if (ShakeTimeRemaining <= 0.0f)
    {
        InOutPOV.Location = OriginLocation;
        // 셰이크 완전 중지
        StopShake();
        return false;
    }

    // 시간 감소
    ShakeTimeRemaining -= DeltaTime;

    // 간단한 난수 생성
    float randX = (rand() % 2000 - 1000) / 1000.0f;  // -1.0 ~ 1.0
    float randY = (rand() % 2000 - 1000) / 1000.0f;
    float randZ = (rand() % 2000 - 1000) / 1000.0f;

    // 현재 강도 계산 (시간에 따른 감쇠)
    float currentIntensity = ShakeIntensity;
    if (ShakeDuration > 0.0f)
    {
        float timeRatio = ShakeTimeRemaining / ShakeDuration;
        currentIntensity *= timeRatio; // 시간에 따라 선형 감소
    }

    // 셰이크 오프셋 계산
    FVector shakeOffset;
    shakeOffset.X = randX * currentIntensity;
    shakeOffset.Y = randY * currentIntensity;
    shakeOffset.Z = randZ * currentIntensity * 0.5f;  // Z축은 약하게

    // 회전 오프셋 (선택적)
    FRotator rotOffset;
    rotOffset.Pitch = randX * currentIntensity * 2.0f;
    rotOffset.Yaw = randY * currentIntensity * 2.0f;
    rotOffset.Roll = randZ * currentIntensity * 0.5f;  // Roll은 약하게

    InOutPOV.Location += shakeOffset;
    InOutPOV.Rotation += rotOffset;

    return true;
}

//void UCameraShakeModifier::ModifyCamera(float DeltaTime, FVector ViewLocation, FRotator ViewRotation, float FOV,
//    FVector& NewLocation, FRotator& NewRotation, float& NewFOV)
//{
//    NewLocation = ViewLocation;
//    NewRotation = ViewRotation;
//    NewFOV = FOV;
//
//    // 셰이크 중이 아니거나 알파가 0이면 효과 없음
//    if (!bIsShaking || Alpha <= 0.0f)
//    {
//        if (ShakeTimeRemaining <= 0.0f && bIsShaking)
//        {
//            // 셰이크 종료
//            StopShake();
//        }
//        return;
//    }
//
//    // 시간 감소
//    ShakeTimeRemaining -= DeltaTime;
//
//    // 간단한 난수 생성
//    float randX = (rand() % 2000 - 1000) / 1000.0f;  // -1.0 ~ 1.0
//    float randY = (rand() % 2000 - 1000) / 1000.0f;
//    float randZ = (rand() % 2000 - 1000) / 1000.0f;
//
//    // 현재 강도 계산 (시간에 따른 감쇠)
//    float currentIntensity = ShakeIntensity;
//    if (ShakeDuration > 0.0f)
//    {
//        float timeRatio = ShakeTimeRemaining / ShakeDuration;
//        currentIntensity *= timeRatio; // 시간에 따라 선형 감소
//    }
//
//    // 셰이크 오프셋 계산
//    FVector shakeOffset;
//    shakeOffset.X = randX * currentIntensity;
//    shakeOffset.Y = randY * currentIntensity;
//    shakeOffset.Z = randZ * currentIntensity * 0.5f;  // Z축은 약하게
//
//    // 회전 오프셋 (선택적)
//    FRotator rotOffset;
//    rotOffset.Pitch = randX * currentIntensity * 2.0f;
//    rotOffset.Yaw = randY * currentIntensity * 2.0f;
//    rotOffset.Roll = randZ * currentIntensity * 0.5f;  // Roll은 약하게
//
//    // 결과 적용
//    NewLocation = ViewLocation + shakeOffset;
//    NewRotation = ViewRotation + rotOffset;
//}
