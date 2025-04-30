#include "SpringArmComponent.h"
#include "GameFramework/Actor.h" // GetOwner()
#include "Math/JungleMath.h" // 수학 함수

USpringArmComponent::USpringArmComponent()
{
    //SocketOffset = FVector::ZeroVector;
    TargetOffset = FVector::ZeroVector;

    bUsePawnControlRotation = false;
    bDoCollisionTest = false;

    bUsePawnControlRotation = false;
    bInheritPitch = true;
    bInheritYaw = true;
    bInheritRoll = true;

    TargetArmLength = 30.0f;
    ProbeSize = 12.0f;
    //ProbeChannel = ECC_Camera;

    //RelativeSocketRotation = FQuat::Identity;

    bUseCameraLagSubstepping = true;
    CameraLagSpeed = 1.f;
    CameraRotationLagSpeed = 1.f;
    CameraLagMaxTimeStep = 1.f / 60.f;
    CameraLagMaxDistance = 0.f;
    //bClampToMaxPhysicsDeltaTime = false;

    UnfixedCameraPosition = FVector::ZeroVector;
}

FRotator USpringArmComponent::GetDesiredRotation() const
{
    return GetComponentRotation();
}

FRotator USpringArmComponent::GetTargetRotation() const
{
    FRotator DesiredRot = GetDesiredRotation();

    if (bUsePawnControlRotation)
    {
        // @todo Pawn의 ControlRotation을 사용해야 함
        if (AActor* OwnerActor = GetOwner())
        {
            // 임시로 액터의 월드 회전을 사용
            const FRotator ActorRotation = OwnerActor->GetRootComponent()->GetWorldRotation();
            if (DesiredRot != ActorRotation)
            {
                DesiredRot = ActorRotation;
            }
        }
    }

    // @todo SceneComponent의 bAbsoluteRotation 플래그 추가
    //if (!IsUsingAbsoluteRotation())
    {
        const FRotator LocalRelativeRotation = GetRelativeRotation();
        if (!bInheritPitch)
        {
            DesiredRot.Pitch = LocalRelativeRotation.Pitch;
        }

        if (!bInheritYaw)
        {
            DesiredRot.Yaw = LocalRelativeRotation.Yaw;
        }

        if (!bInheritRoll)
        {
            DesiredRot.Roll = LocalRelativeRotation.Roll;
        }
    }

    return DesiredRot;
}

void USpringArmComponent::UpdateDesiredArmLocation(bool bDoTrace, bool bDoLocationLag, bool bDoRotationLag, float DeltaTime) {
    FRotator DesiredRot = GetTargetRotation();

    // 회전 지연 적용
    if (bDoRotationLag)
    {
        if (bUseCameraLagSubstepping && DeltaTime > CameraLagMaxTimeStep && CameraRotationLagSpeed > 0.f)
        {
            const FRotator ArmRotStep = (DesiredRot - PreviousDesiredRot).GetNormalized() * (1.f / DeltaTime);
            FRotator LerpTarget = PreviousDesiredRot;
            float RemainingTime = DeltaTime;
            while (RemainingTime > KINDA_SMALL_NUMBER)
            {
                const float LerpAmount = FMath::Min(CameraLagMaxTimeStep, RemainingTime);
                LerpTarget += ArmRotStep * LerpAmount;
                RemainingTime -= LerpAmount;

                // @todo Use FQuat(Rot) instead of FRotator.ToQuaternion()
                DesiredRot = FRotator(FMath::QInterpTo(PreviousDesiredRot.ToQuaternion(), LerpTarget.ToQuaternion(), LerpAmount, CameraRotationLagSpeed));
                PreviousDesiredRot = DesiredRot;
            }
        }
        else
        {
            // @todo Use FQuat(Rot) instead of FRotator.ToQuaternion()
            DesiredRot = FRotator(FMath::QInterpTo(PreviousDesiredRot.ToQuaternion(), DesiredRot.ToQuaternion(), DeltaTime, CameraRotationLagSpeed));
        }
    }
    PreviousDesiredRot = DesiredRot;

    // the target we want to look at, SpringArm의 'origin'
    FVector ArmOrigin = GetWorldLocation() + TargetOffset;
    // 실제 카메라 위치가 아닌, 타겟을 지연시킴. 카메라를 돌려도 지연이 없음
    FVector DesiredLoc = ArmOrigin;
    if (bDoLocationLag)
    {
        if (bUseCameraLagSubstepping && DeltaTime > CameraLagMaxTimeStep && CameraLagSpeed > 0.f)
        {
            const FVector ArmMovementStep = (DesiredLoc - PreviousDesiredLoc) * (1.f / DeltaTime);
            FVector LerpTarget = PreviousDesiredLoc;

            float RemainingTime = DeltaTime;
            while (RemainingTime > KINDA_SMALL_NUMBER)
            {
                const float LerpAmount = FMath::Min(CameraLagMaxTimeStep, RemainingTime);
                LerpTarget += ArmMovementStep * LerpAmount;
                RemainingTime -= LerpAmount;

                DesiredLoc = FMath::VInterpTo(PreviousDesiredLoc, LerpTarget, LerpAmount, CameraLagSpeed);
                PreviousDesiredLoc = DesiredLoc;
            }
        }
        else
        {
            DesiredLoc = FMath::VInterpTo(PreviousDesiredLoc, DesiredLoc, DeltaTime, CameraLagSpeed);
        }

        // 거리 제한이 있는 경우 Clamp 적용
        bool bClampedDist = false;  // 디버그 용 (True: Red, False: Green, i.e. 정상이면 초록)
        if (CameraLagMaxDistance > 0.f)
        {
            const FVector FromOrigin = DesiredLoc - ArmOrigin;
            if (FromOrigin.LengthSquared() > FMath::Square(CameraLagMaxDistance))
            {
                DesiredLoc = ArmOrigin + FromOrigin.GetClampedToMaxSize(CameraLagMaxDistance);
                bClampedDist = true;
            }
        }

        // @todo Draw Debug Sphere and Arrow
        if (bDrawDebugLagMarkers)
        {
            //...
        }
    }

    PreviousArmOrigin = ArmOrigin;
    PreviousDesiredLoc = DesiredLoc;

    // 팔 길이 적용 (회전 방향 반대로)
    DesiredLoc -= DesiredRot.ToVector().GetSafeNormal() * TargetArmLength;
    // @todo Apply SocketOffset
    // local space에서 소켓 오프셋 적용
    //DesiredLoc += FRotationMatrix(DesiredRot).TransformVector(SocketOffset);

    // Do a sweep to ensure we are not penetrating the world
    FVector ResultLoc;
    // @todo Collision 구현한 후 적용
    if (false)//bDoTrace && (TargetArmLength != 0.0f))
    {
/*
        bIsCameraFixed = true;
        FCollisionQueryParams QueryParams(SCENE_QUERY_STAT(SpringArm), false, GetOwner());

        FHitResult Result;
        GetWorld()->SweepSingleByChannel(Result, ArmOrigin, DesiredLoc, FQuat::Identity, ProbeChannel, FCollisionShape::MakeSphere(ProbeSize), QueryParams);

        UnfixedCameraPosition = DesiredLoc;

        ResultLoc = BlendLocations(DesiredLoc, Result.Location, Result.bBlockingHit, DeltaTime);

        if (ResultLoc == DesiredLoc)
        {
            bIsCameraFixed = false;
        }
*/
    }
    else
    {
        ResultLoc = DesiredLoc;
        bIsCameraFixed = false;
        UnfixedCameraPosition = ResultLoc;
    }

    // @todo Transform 및 Socket 구현 후 적용
/*
    // Form a transform for new world transform for camera
    FTransform WorldCamTM(DesiredRot, ResultLoc);
    // Convert to relative to component
    FTransform RelCamTM = WorldCamTM.GetRelativeTransform(GetComponentTransform());

    // Update socket location/rotation
    RelativeSocketLocation = RelCamTM.GetLocation();
    RelativeSocketRotation = RelCamTM.GetRotation();
*/

    // @todo 이 함수는 SceneComponent에 구현되어야 함
    // 최종 위치/회전으로 자식 업데이트
    UpdateChildTransforms(ResultLoc, DesiredRot);
}

FVector USpringArmComponent::BlendLocations(const FVector& DesiredArmLocation, const FVector& TraceHitLocation, bool bHitSomething, float DeltaTime)
{
    return bHitSomething ? TraceHitLocation : DesiredArmLocation;
}

void USpringArmComponent::UpdateChildTransforms(const FVector& NewLocation, const FRotator& NewRotation) const
{
    for (USceneComponent* ChildComp : GetAttachChildren())
    {
        if (ChildComp)
        {
            ChildComp->SetWorldLocation(NewLocation);
            ChildComp->SetWorldRotation(NewRotation);
        }
    }
}

void USpringArmComponent::InitializeComponent()
{
    Super::InitializeComponent();

    // 잠재적인 div-by-zero를 피하기 위한 제한
    CameraLagMaxTimeStep = FMath::Max(CameraLagMaxTimeStep, 1.f / 200.f);
    CameraLagSpeed = FMath::Max(CameraLagSpeed, 0.f);

    // 초기화 (지연 없음)
    UpdateDesiredArmLocation(false, false, false, 0.f);
}

void USpringArmComponent::TickComponent(const float DeltaTime)
{
    Super::TickComponent(DeltaTime);

    UpdateDesiredArmLocation(bDoCollisionTest, bEnableCameraLag, bEnableCameraRotationLag, DeltaTime);
}
