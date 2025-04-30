#include "SpringArmComponent.h"
#include "GameFramework/Actor.h" // GetOwner()
#include "Math/JungleMath.h" // 수학 함수

USpringArmComponent::USpringArmComponent()
{
    // 기본값 설정
    TargetArmLength = 30.0f;
}

void USpringArmComponent::TickComponent(const float DeltaTime)
{
    Super::TickComponent(DeltaTime);

    UpdateChildTransforms();
}

void USpringArmComponent::UpdateChildTransforms()
{
    const FVector ParentWorldLocation = GetWorldLocation(); // 부모(또는 액터)의 위치
    const FRotator ParentWorldRotation = GetWorldRotation(); // 부모(또는 액터)의 회전

    // --- 목표 회전 계산 (1단계: 단순 상속) ---
    FRotator TargetWorldRotation = ParentWorldRotation; // 우선 부모 회전 그대로 사용

    // --- 목표 위치 계산 (1단계: 충돌 없음) ---
    const FVector ArmOrigin = ParentWorldLocation; // TargetOffset 미적용
    const FVector ArmRotationVector = TargetWorldRotation.ToVector().GetSafeNormal(); // 회전 방향 벡터
    const FVector DesiredWorldEndPoint = ArmOrigin - ArmRotationVector * TargetArmLength;

    // --- 자식 트랜스폼 업데이트 ---
    for (USceneComponent* ChildComp : GetAttachChildren())
    {
        if (ChildComp)
        {
            // 자식의 월드 위치와 회전을 직접 설정
            ChildComp->SetWorldLocation(DesiredWorldEndPoint); // SocketOffset 미적용
            ChildComp->SetWorldRotation(TargetWorldRotation);
        }
    }
}
