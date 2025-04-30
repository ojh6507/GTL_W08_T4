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
        OriginRotation = InOutPOV.Rotation;
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
        InOutPOV.Rotation = OriginRotation;
        // 셰이크 완전 중지
        StopShake();
        return false;
    }

    // 현재 강도 계산 (시간에 따른 감쇠)
    float currentIntensity = ShakeIntensity;
    if (ShakeDuration > 0.0f)
    {
        float timeRatio = ShakeTimeRemaining / ShakeDuration;
        currentIntensity *= FMath::Pow(timeRatio, 0.3f);
    }

    // 달리는 방향 고려한 흔들림
    float frequencyMultiplier = 12.0f;
    float timeElapsed = ShakeDuration - ShakeTimeRemaining;

    // 주행 방향(X축) 강조 흔들림
    float smoothX = FMath::Sin(timeElapsed * frequencyMultiplier * 1.5f) * currentIntensity * 25.0f;
    float smoothY = FMath::Sin(timeElapsed * frequencyMultiplier) * currentIntensity * 10.0f;
    float smoothZ = FMath::Sin(timeElapsed * frequencyMultiplier * 0.8f) * currentIntensity * 5.0f;

    // 셰이크 오프셋 계산
    FVector shakeOffset;
    shakeOffset.X = smoothX;  // 주행 방향 강조
    shakeOffset.Y = smoothY;  // 약간의 좌우 흔들림
    shakeOffset.Z = smoothZ;  // 미세한 상하 흔들림

    // 회전 오프셋 (충격감 강조)
    FRotator rotOffset;
    rotOffset.Pitch = smoothY * 0.1f;   // 앞뒤 흔들림
    rotOffset.Yaw = smoothX * 0.05f;    // 회전 방향 미세 조정
    rotOffset.Roll = smoothZ * 0.03f;   // 미세한 기울임

    // 원래 위치/회전을 기준으로 오프셋 적용
    InOutPOV.Location = OriginLocation + shakeOffset;
    InOutPOV.Rotation = OriginRotation + rotOffset;

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
