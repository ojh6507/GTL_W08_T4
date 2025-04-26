#include "SphereComponent.h"

#include "BoxComponent.h"
#include "CapsuleComponent.h"
#include "Runtime/Core/Math/JungleMath.h"
#include "UnrealEd/EditorViewportClient.h"
#include "UObject/Casts.h"

USphereComponent::USphereComponent()
{
    SetType(StaticClass()->GetName());
}

USphereComponent::~USphereComponent()
{
}

void USphereComponent::Serialize(FArchive& Ar)
{
    UShapeComponent::Serialize(Ar);
}

void USphereComponent::UninitializeComponent()
{
    UShapeComponent::UninitializeComponent();
}

void USphereComponent::BeginPlay()
{
    UShapeComponent::BeginPlay();
}

void USphereComponent::OnComponentDestroyed()
{
    UShapeComponent::OnComponentDestroyed();
}

void USphereComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    UShapeComponent::EndPlay(EndPlayReason);
}

void USphereComponent::DestroyComponent()
{
    UShapeComponent::DestroyComponent();
}

UObject* USphereComponent::Duplicate(UObject* InOuter)
{
    return UShapeComponent::Duplicate(InOuter);
}

int USphereComponent::CheckRayIntersection(FVector& rayOrigin, FVector& rayDirection, float& pfNearHitDistance)
{
    return UShapeComponent::CheckRayIntersection(rayOrigin, rayDirection, pfNearHitDistance);
}

void USphereComponent::GetProperties(TMap<FString, FString>& OutProperties) const
{
    UShapeComponent::GetProperties(OutProperties);
}

void USphereComponent::SetProperties(const TMap<FString, FString>& InProperties)
{
    UShapeComponent::SetProperties(InProperties);
}

bool USphereComponent::CheckOverlapComponent(UShapeComponent* Other, FHitResult& OutHitResult)
{
    if (UBoxComponent* OtherBox = Cast<UBoxComponent>(Other))
    {
        return OverlapSphereToBox(OtherBox, OutHitResult);
    }

    if (USphereComponent* OtherSphere = Cast<USphereComponent>(Other))
    {
        return OverlapSphereToSphere(OtherSphere, OutHitResult);
    }

    if (UCapsuleComponent* OtherCapsule = Cast<UCapsuleComponent>(Other))
    {
        return OverlapSphereToCapsule(OtherCapsule, OutHitResult);
    }

    return false;
}

void USphereComponent::InitializeComponent()
{
    Super::InitializeComponent();
}

void USphereComponent::TickComponent(float DeltaTime)
{
    Super::TickComponent(DeltaTime);
}

bool USphereComponent::OverlapSphereToBox(UBoxComponent* OtherBox, FHitResult& OutHitResult) const
{
    const FMatrix BoxTM    = OtherBox->GetWorldMatrix();
    const FMatrix WorldToBox = FMatrix::Inverse(BoxTM);

    // 1) Sphere 중심을 Box 로컬 공간으로 변환
    const FVector WorldCenter = GetWorldLocation();
    const FVector LocalCenter = WorldToBox.TransformPosition(WorldCenter);

    // 2) Box half-extent 내부로 클램프
    FVector Clamped = LocalCenter;
    const FVector BoxExtent = OtherBox->GetBoxExtent();
    Clamped.X = FMath::Clamp(LocalCenter.X, -BoxExtent.X, BoxExtent.X);
    Clamped.Y = FMath::Clamp(LocalCenter.Y, -BoxExtent.Y, BoxExtent.Y);
    Clamped.Z = FMath::Clamp(LocalCenter.Z, -BoxExtent.Z, BoxExtent.Z);

    // 3) 다시 월드 공간 ClosestPoint 계산
    const FVector ClosestPoint = BoxTM.TransformPosition(Clamped);

    // 4) Sphere 반지름과 비교
    const FVector Delta = WorldCenter - ClosestPoint;
    const float Dist2 = Delta.LengthSquared();
    const float R = GetRadius();
    if (Dist2 <= R * R)
    {
        const float Dist = FMath::Sqrt(Dist2);
        OutHitResult.bBlockingHit = true;
        OutHitResult.ImpactPoint  = ClosestPoint;
        OutHitResult.Location     = ClosestPoint;
        OutHitResult.Normal       = Dist > 0 ? Delta / Dist : FVector::UpVector;
        OutHitResult.ImpactNormal = OutHitResult.Normal;
        OutHitResult.HitComponent = Cast<UPrimitiveComponent>(OtherBox);
        OutHitResult.HitActor     = OtherBox->GetOwner();
        return true;
    }

    return false;
}

bool USphereComponent::OverlapSphereToSphere(USphereComponent* OtherSphere, FHitResult& OutHitResult) const
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
        const float Dist = FMath::Sqrt(Dist2);
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

bool USphereComponent::OverlapSphereToCapsule(UCapsuleComponent* OtherCapsule, FHitResult& OutHitResult) const
{
    // 캡슐 중심선(segment) 두 끝점 계산
    const FVector Dir     = OtherCapsule->GetUpVector();
    const FVector C       = OtherCapsule->GetWorldLocation();
    const float H         = OtherCapsule->GetCapsuleHalfHeight();
    const FVector P1 = C + Dir * H;
    const FVector P2 = C - Dir * H;

    // 구 중심과 segment의 최단 거리점 계산
    const FVector SC = GetWorldLocation();
    const FVector Closest = JungleMath::ClosestPointOnSegment(SC, P1, P2);

    // Sphere 반지름과 Capsule 반지름 합으로 비교
    const FVector Delta = SC - Closest;
    const float Dist2   = Delta.LengthSquared();
    const float Rsum    = SphereRadius + OtherCapsule->GetCapsuleRadius();
    if (Dist2 <= Rsum*Rsum)
    {
        const float Dist       = FMath::Sqrt(Dist2);
        OutHitResult.bBlockingHit = true;
        OutHitResult.ImpactPoint  = Closest;
        OutHitResult.Location     = Closest;
        OutHitResult.Normal       = Dist > 0 ? Delta / Dist : FVector::UpVector;
        OutHitResult.ImpactNormal = OutHitResult.Normal;
        OutHitResult.HitComponent = Cast<UPrimitiveComponent>(OtherCapsule);
        OutHitResult.HitActor     = OtherCapsule->GetOwner();
        return true;
    }

    return false;
}
