#include "SphereComponent.h"
#include "Runtime/Core/Math/JungleMath.h"
#include "UnrealEd/EditorViewportClient.h"
#include "UObject/Casts.h"

USphereComponent::USphereComponent()
{
    SetType(StaticClass()->GetName());
}

void USphereComponent::InitializeComponent()
{
    Super::InitializeComponent();
}

void USphereComponent::TickComponent(float DeltaTime)
{
    Super::TickComponent(DeltaTime);
}

bool USphereComponent::OverlapSphereToBox(UBoxComponent* Other, FHitResult& OutHitResult)
{
    return false;
}

bool USphereComponent::OverlapSphereToSphere(USphereComponent* OtherSphere, FHitResult& OutHitResult)
{
    const FVector C1 = GetWorldLocation();
    const FVector C2 = OtherSphere->GetWorldLocation();
    const float R1 = SphereRadius;
    const float R2 = OtherSphere->SphereRadius;

    const FVector Delta = C2 - C1;
    const float Dist2 = Delta.LengthSquared();
    const float Rsum = R1 + R2;
    if (Dist2 <= Rsum * Rsum)
    {
        float Dist = FMath::Sqrt(Dist2);
        OutHitResult.bBlockingHit = true;
        // 충돌 지점: 두 구의 중심을 잇는 선 위, 각 구 표면이 만나는 점
        OutHitResult.ImpactPoint = C1 + Delta * (R1 / Rsum);
        OutHitResult.Location     = OutHitResult.ImpactPoint;
        OutHitResult.Normal       = Delta / Dist;
        OutHitResult.ImpactNormal = OutHitResult.Normal;
        OutHitResult.HitComponent = Cast<UPrimitiveComponent>(OtherSphere);
        OutHitResult.HitActor     = OtherSphere->GetOwner();
        return true;
    }
    return false;
}

bool USphereComponent::OverlapSphereToCapsule(UCapsuleComponent* Other, FHitResult& OutHitResult)
{
    return false;
}
