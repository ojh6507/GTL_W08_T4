#pragma once
#include "Components/SceneComponent.h"
#include "UObject/ObjectMacros.h" // DECLARE_CLASS

class USpringArmComponent : public USceneComponent
{
    DECLARE_CLASS(USpringArmComponent, USceneComponent)

public:
    USpringArmComponent();

    // --- 핵심 로직 ---
    virtual void InitializeComponent() override;
    virtual void TickComponent(float DeltaTime) override;

    float GetTargetArmLength() const { return TargetArmLength; }
    void SetTargetArmLength(float InTargetArmLength) { TargetArmLength = InTargetArmLength; }

    UPROPERTY(float, TargetArmLength, = 30.0f)  // 기본 팔 길이
    // @todo Apply SocketOffset
    //UPROPERTY(FVector, SocketOffset, = FVector::ZeroVector) // 로컬 오프셋
    UPROPERTY(FVector, TargetOffset, = FVector::ZeroVector) // 월드 오프셋

    UPROPERTY(float, ProbeSize, = 12.0f)    // 충돌 테스트 크기
    UPROPERTY(bool, bDoCollisionTest, = 1)  // 충돌 테스트 여부

    UPROPERTY(bool, bUsePawnControlRotation, = 1)
    UPROPERTY(bool, bInheritPitch, = 1)
    UPROPERTY(bool, bInheritYaw, = 1)
    UPROPERTY(bool, bInheritRoll, = 1)

    UPROPERTY(bool, bEnableCameraLag, = 1)
    UPROPERTY(bool, bEnableCameraRotationLag, = 1)
    UPROPERTY(bool, bUseCameraLagSubstepping, = 1)
    // @todo Draw Debug Sphere and Arrow
    UPROPERTY(bool, bDrawDebugLagMarkers, = 1)

    UPROPERTY(float, CameraLagSpeed, = 1.0f)
    UPROPERTY(float, CameraRotationLagSpeed, = 1.0f)
    UPROPERTY(float, CameraLagMaxTimeStep, = 0.005f)
    UPROPERTY(float, CameraLagMaxDistance, = 0.0f)

    // 카메라 지연 계산용 임시 변수
    FVector PreviousDesiredLoc;
    FVector PreviousArmOrigin;
    // 카메라 회전 지연 계산용 임시 변수
    FRotator PreviousDesiredRot;

    /** Returns the desired rotation for the spring arm, before the rotation constraints such as bInheritPitch etc are enforced. */
    virtual FRotator GetDesiredRotation() const;

    /**
     * Get the target rotation we inherit, used as the base target for the boom rotation.
     * This is derived from attachment to our parent and considering the UsePawnControlRotation and absolute rotation flags.
     */
    virtual FRotator GetTargetRotation() const;

    // @todo 이 함수는 SceneComponent에 구현되어야 함
    // 자식 컴포넌트의 최종 트랜스폼을 업데이트하는 함수
    void UpdateChildTransforms(const FVector& NewLocation, const FRotator& NewRotation) const;

    // 충돌 처리 계산용 임시 변수
    bool bIsCameraFixed = false;
    FVector UnfixedCameraPosition;

protected:
    /** Cached component-space socket location */
    //FVector RelativeSocketLocation;
    /** Cached component-space socket rotation */
    //FQuat RelativeSocketRotation;

protected:
    // 최종 위치/회전을 계산하고 자식 트랜스폼을 업데이트하는 함수
    virtual void UpdateDesiredArmLocation(bool bDoTrace, bool bDoLocationLag, bool bDoRotationLag, float DeltaTime);

    // 충돌 시 위치 보간 (Phase 2에서 구현)
    virtual FVector BlendLocations(const FVector& DesiredArmLocation, const FVector& TraceHitLocation, bool bHitSomething, float DeltaTime);
};
