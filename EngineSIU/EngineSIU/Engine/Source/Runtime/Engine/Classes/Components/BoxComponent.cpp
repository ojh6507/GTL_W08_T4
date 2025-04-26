#include "BoxComponent.h"

#include <set>

#include "CapsuleComponent.h"
#include "SphereComponent.h"
#include "Math/JungleMath.h"
#include "UObject/Casts.h"

UBoxComponent::UBoxComponent()
{
    SetType(StaticClass()->GetName());

}

void UBoxComponent::InitializeComponent()
{
    Super::InitializeComponent();
}

void UBoxComponent::TickComponent(float DeltaTime)
{
    Super::TickComponent(DeltaTime);

}

UBoxComponent::~UBoxComponent()
{
}

void UBoxComponent::Serialize(FArchive& Ar)
{
    UShapeComponent::Serialize(Ar);
}

void UBoxComponent::UninitializeComponent()
{
    UShapeComponent::UninitializeComponent();
}

void UBoxComponent::BeginPlay()
{
    UShapeComponent::BeginPlay();
}

void UBoxComponent::OnComponentDestroyed()
{
    UShapeComponent::OnComponentDestroyed();
}

void UBoxComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    UShapeComponent::EndPlay(EndPlayReason);
}

void UBoxComponent::DestroyComponent()
{
    UShapeComponent::DestroyComponent();
}

UObject* UBoxComponent::Duplicate(UObject* InOuter)
{
    return UShapeComponent::Duplicate(InOuter);
}

int UBoxComponent::CheckRayIntersection(FVector& rayOrigin, FVector& rayDirection, float& pfNearHitDistance)
{
    return UShapeComponent::CheckRayIntersection(rayOrigin, rayDirection, pfNearHitDistance);
}

void UBoxComponent::GetProperties(TMap<FString, FString>& OutProperties) const
{
    UShapeComponent::GetProperties(OutProperties);
}

void UBoxComponent::SetProperties(const TMap<FString, FString>& InProperties)
{
    UShapeComponent::SetProperties(InProperties);
}

bool UBoxComponent::CheckOverlapComponent(UShapeComponent* Other, FHitResult& OutHitResult)
{
    if (UBoxComponent* OtherBox = Cast<UBoxComponent>(Other))
    {
        return OverlapBoxToBox(OtherBox, OutHitResult);
    }

    if (USphereComponent* OtherSphere = Cast<USphereComponent>(Other))
    {
        return OverlapBoxToSphere(OtherSphere, OutHitResult);
    }

    if (UCapsuleComponent* OtherCapsule = Cast<UCapsuleComponent>(Other))
    {
        return OverlapBoxToCapsule(OtherCapsule, OutHitResult);
    }

    return false;
}

bool UBoxComponent::OverlapBoxToBox(UBoxComponent* OtherBox, FHitResult& OutHitResult)
{
    // --- 1) OBB 파라미터 추출 ---
    // 로컬 half-extents 에 월드 스케일을 곱한 값
    const FVector MyScale     = GetWorldScale3D();
    const FVector MyExtents   = FVector(BoxExtent.X * MyScale.X,
                                  BoxExtent.Y * MyScale.Y,
                                  BoxExtent.Z * MyScale.Z);

    const FVector OtherScale  = OtherBox->GetWorldScale3D();
    const FVector OtherExtents= FVector(OtherBox->BoxExtent.X * OtherScale.X,
                                  OtherBox->BoxExtent.Y * OtherScale.Y,
                                  OtherBox->BoxExtent.Z * OtherScale.Z);

    // 월드 공간에서의 OBB 중심
    const FVector C1 = GetWorldLocation();
    const FVector C2 = OtherBox->GetWorldLocation();

    // 순수 회전 행렬만 가져와서, 축 벡터 계산 (정규화)
    const FMatrix R1 = GetRotationMatrix();
    TArray<FVector> A1;
    A1.Add(FMatrix::TransformVector(FVector(1,0,0), R1).GetSafeNormal());
    A1.Add( FMatrix::TransformVector(FVector(0,1,0), R1).GetSafeNormal());
    A1.Add( FMatrix::TransformVector(FVector(0, 0, 1), R1).GetSafeNormal());

    const FMatrix R2 = OtherBox->GetRotationMatrix();
    TArray<FVector> A2;
    A2.Add(FMatrix::TransformVector(FVector(1,0,0), R2).GetSafeNormal());
    A2.Add( FMatrix::TransformVector(FVector(0,1,0), R2).GetSafeNormal());
    A2.Add( FMatrix::TransformVector(FVector(0, 0, 1), R2).GetSafeNormal());

    // --- 2) SAT 분리축 리스트 (총 15) ---
    TArray<FVector> Axes;
    Axes.SetNum(15);
    int idx = 0;
    // A 축 3개
    for (int i = 0; i < 3; ++i) Axes[idx++] = A1[i];
    // B 축 3개
    for (int i = 0; i < 3; ++i) Axes[idx++] = A2[i];
    // 크로스 9개
    for (int i = 0; i < 3; ++i)
        for (int j = 0; j < 3; ++j)
        {
            FVector cross = A1[i] ^ A2[j]; // cross product
            if (!cross.IsNearlyZero())
                Axes[idx++] = cross.GetSafeNormal();
        }

    // --- 3) 축마다 투영(overlap) 검사 ---
    float minOverlap = FLT_MAX;
    FVector smallestAxis = FVector::ZeroVector;
    float dC = 0.f;

    for (int a = 0; a < idx; ++a)
    {
        const FVector& L = Axes[a];

        // 투영 반경 계산: r = Σ e_i * |u_i·L|
        const float r1 = MyExtents.X * FMath::Abs(A1[0] | L)
                 + MyExtents.Y * FMath::Abs(A1[1] | L)
                 + MyExtents.Z * FMath::Abs(A1[2] | L);

        const float r2 = OtherExtents.X * FMath::Abs(A2[0] | L)
                 + OtherExtents.Y * FMath::Abs(A2[1] | L)
                 + OtherExtents.Z * FMath::Abs(A2[2] | L);

        // 중심 간 거리 투영
        const float dist = (C2 - C1) | L;
        const float overlap = (r1 + r2) - FMath::Abs(dist);
        if (overlap <= 0.f)
        {
            // 분리축 발견 → 충돌 없음
            return false;
        }
        // 최소 침투 깊이 저장
        if (overlap < minOverlap)
        {
            minOverlap = overlap;
            // 법선 방향: dist < 0 이면 반대로
            smallestAxis = (dist < 0.f ? -L : L);
            dC = dist;
        }
    }

    // --- 4) 충돌 보정 및 HitResult 채우기 ---
    // 충돌 지점: 두 중심의 중간에서 법선 방향으로 약간 보정
    const FVector contactPoint = (C1 + C2) * 0.5f;
    // 또는 C1 + smallestAxis * (r1 - minOverlap * 0.5f);

    OutHitResult.bBlockingHit   = true;
    OutHitResult.Location       = contactPoint;
    OutHitResult.ImpactPoint    = contactPoint;
    OutHitResult.Normal         = smallestAxis;
    OutHitResult.ImpactNormal   = -smallestAxis;
    OutHitResult.HitComponent   = Cast<UPrimitiveComponent>(OtherBox);
    OutHitResult.HitActor       = OtherBox->GetOwner();
    return true;
}

bool UBoxComponent::OverlapBoxToSphere(USphereComponent* OtherSphere, FHitResult& OutHitResult)
{
    const FMatrix BoxTM    = GetWorldMatrix();
    const FMatrix WorldToBox = FMatrix::Inverse(BoxTM);

    // 1) Sphere 중심을 Box 로컬 공간으로 변환
    const FVector WorldCenter = OtherSphere->GetWorldLocation();
    const FVector LocalCenter = WorldToBox.TransformPosition(WorldCenter);

    // 2) Box half-extent 내부로 클램프
    FVector Clamped = LocalCenter;
    Clamped.X = FMath::Clamp(LocalCenter.X, -BoxExtent.X, BoxExtent.X);
    Clamped.Y = FMath::Clamp(LocalCenter.Y, -BoxExtent.Y, BoxExtent.Y);
    Clamped.Z = FMath::Clamp(LocalCenter.Z, -BoxExtent.Z, BoxExtent.Z);

    // 3) 다시 월드 공간 ClosestPoint 계산
    const FVector ClosestPoint = BoxTM.TransformPosition(Clamped);

    // 4) Sphere 반지름과 비교
    const FVector Delta = WorldCenter - ClosestPoint;
    const float Dist2 = Delta.LengthSquared();
    const float R = OtherSphere->GetRadius();
    if (Dist2 <= R * R)
    {
        const float Dist = FMath::Sqrt(Dist2);
        OutHitResult.bBlockingHit = true;
        OutHitResult.ImpactPoint  = ClosestPoint;
        OutHitResult.Location     = ClosestPoint;
        OutHitResult.Normal       = Dist > 0 ? Delta / Dist : FVector::UpVector;
        OutHitResult.ImpactNormal = OutHitResult.Normal;
        OutHitResult.HitComponent = Cast<UPrimitiveComponent>(OtherSphere);
        OutHitResult.HitActor     = OtherSphere->GetOwner();
        return true;
    }

    return false;
}

bool UBoxComponent::OverlapBoxToCapsule(UCapsuleComponent* OtherCaps, FHitResult& OutHitResult)
{
    // --- 1) OBB 파라미터 추출 ---
    // 박스 half-extents 에 월드 스케일 적용
    const FVector boxScale    = GetWorldScale3D();
    const FVector halfExtents = FVector(BoxExtent.X * boxScale.X,
                                  BoxExtent.Y * boxScale.Y,
                                  BoxExtent.Z * boxScale.Z);

    // 박스 변환 행렬
    const FMatrix boxToWorld  = GetWorldMatrix();
    const FMatrix worldToBox  = FMatrix::Inverse(boxToWorld);

    // --- 2) 캡슐 세그먼트 계산 ---
    // 캡슐 중심
    const FVector capCenter  = OtherCaps->GetWorldLocation();
    // 캡슐 방향(로컬 Z축)과 스케일
    const FMatrix capRotMat  = OtherCaps->GetRotationMatrix();
    const FVector capAxis    = FMatrix::TransformVector(FVector(0,0,1), capRotMat).GetSafeNormal();
    const FVector capScale   = OtherCaps->GetWorldScale3D();
    const float   halfHeight = OtherCaps->GetCapsuleHalfHeight() * capScale.Z;
    const float   radius     = OtherCaps->GetCapsuleRadius() * FMath::Max(capScale.X, FMath::Max(capScale.Y, capScale.Z));

    // 세그먼트 양 끝점 (월드 공간)
    const FVector P1 = capCenter + capAxis * halfHeight;
    const FVector P2 = capCenter - capAxis * halfHeight;

    // 로컬 AABB 공간으로 변환
    const FVector L1 = worldToBox.TransformPosition(P1);
    const FVector L2 = worldToBox.TransformPosition(P2);

    // --- 3) AABB 확장 및 세그먼트 vs AABB 슬랩 검사 ---
    // 확장된 half-extents
    const FVector expExtents = halfExtents + FVector(radius);

    const FVector d = L2 - L1;
    float tMin = 0.0f, tMax = 1.0f;

    // 각 축마다 슬랩 검사
    for (int i = 0; i < 3; ++i)
    {
        const float origin = (&L1.X)[i];
        const float dir    = (&d.X)[i];
        const float minB   = -(&expExtents.X)[i];
        const float maxB   =  (&expExtents.X)[i];

        if (FMath::Abs(dir) < KINDA_SMALL_NUMBER)
        {
            // 방향분이 0이면 origin 이 슬랩 내에 있어야 함
            if (origin < minB || origin > maxB)
                return false;
        }
        else
        {
            const float invD = 1.0f / dir;
            float t1 = (minB - origin) * invD;
            float t2 = (maxB - origin) * invD;
            if (t1 > t2) std::swap(t1, t2);
            tMin = FMath::Max(tMin, t1);
            tMax = FMath::Min(tMax, t2);
            if (tMax < tMin)
                return false; // 분리축 발견
        }
    }

    // 충돌 시점 t
    const float tHit = FMath::Clamp(tMin, 0.0f, 1.0f);

    // --- 4) 충돌점 및 법선 계산 ---
    // 로컬에서 충돌 위치
    const FVector localHit = L1 + d * tHit;
    // 로컬 박스 표면에 가장 가까운 점
    FVector localClosest;
    localClosest.X = FMath::Clamp(localHit.X, -halfExtents.X, halfExtents.X);
    localClosest.Y = FMath::Clamp(localHit.Y, -halfExtents.Y, halfExtents.Y);
    localClosest.Z = FMath::Clamp(localHit.Z, -halfExtents.Z, halfExtents.Z);

    // 다시 월드 좌표로
    const FVector worldHit     = boxToWorld.TransformPosition(localHit);
    const FVector worldClosest = boxToWorld.TransformPosition(localClosest);

    const FVector normal = (worldHit - worldClosest).GetSafeNormal();

    // --- 5) FHitResult 채우기 ---
    OutHitResult.bBlockingHit   = true;
    OutHitResult.Location       = worldClosest;
    OutHitResult.ImpactPoint    = worldHit - normal * radius;
    OutHitResult.Normal         = normal;
    OutHitResult.ImpactNormal   = -normal;
    OutHitResult.HitComponent   = Cast<UPrimitiveComponent>(OtherCaps);
    OutHitResult.HitActor       = OtherCaps->GetOwner();
    return true;
}
