#include "CollisionDispatcher.h"

#include "Components/BoxComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/SphereComponent.h"
#include "Math/JungleMath.h"
#include "UObject/Casts.h"

bool FCollisionDispatcher::OverlapBoxToBox(const UBoxComponent* LHSBox, UBoxComponent* RHSBox, FHitResult& OutHitResult)
{
    // --- 1) OBB 파라미터 추출 ---
    // 로컬 half-extents 에 월드 스케일을 곱한 값
    const FVector LHSScale     = LHSBox->GetWorldScale3D();
    const FVector LHSBoxExtent = LHSBox->GetBoxExtent();
    const FVector LHSExtents   = FVector(LHSBoxExtent.X * LHSScale.X, LHSBoxExtent.Y * LHSScale.Y, LHSBoxExtent.Z * LHSScale.Z);

    const FVector RHSScale  = RHSBox->GetWorldScale3D();
    const FVector RHSBoxExtent = RHSBox->GetBoxExtent();
    const FVector RHSExtents= FVector(RHSBoxExtent.X * RHSScale.X, RHSBoxExtent.Y * RHSScale.Y, RHSBoxExtent.Z * RHSScale.Z);
    
    // 월드 공간에서의 OBB 중심
    const FVector C1 = LHSBox->GetWorldLocation();
    const FVector C2 = RHSBox->GetWorldLocation();

    // 순수 회전 행렬만 가져와서, 축 벡터 계산 (정규화)
    const FMatrix R1 = LHSBox->GetRotationMatrix();
    TArray<FVector> A1;
    A1.Add(FMatrix::TransformVector(FVector(1,0,0), R1).GetSafeNormal());
    A1.Add( FMatrix::TransformVector(FVector(0,1,0), R1).GetSafeNormal());
    A1.Add( FMatrix::TransformVector(FVector(0, 0, 1), R1).GetSafeNormal());

    const FMatrix R2 = RHSBox->GetRotationMatrix();
    TArray<FVector> A2;
    A2.Add(FMatrix::TransformVector(FVector(1,0,0), R2).GetSafeNormal());
    A2.Add( FMatrix::TransformVector(FVector(0,1,0), R2).GetSafeNormal());
    A2.Add( FMatrix::TransformVector(FVector(0, 0, 1), R2).GetSafeNormal());

    // --- 2) SAT 분리축 리스트 (총 15) ---
    TArray<FVector> Axes;
    Axes.SetNum(15);
    int idx = 0;
    // A 축 3개
    for (int i = 0; i < 3; ++i)
    {
        Axes[idx++] = A1[i];
    }
    // B 축 3개
    for (int i = 0; i < 3; ++i)
    {
        Axes[idx++] = A2[i];
    }
    
    // 크로스 9개
    for (int i = 0; i < 3; ++i)
    {
        for (int j = 0; j < 3; ++j)
        {
            FVector cross = FVector::CrossProduct(A1[i], A2[j]); // cross product
            if (!cross.IsNearlyZero())
            {
                Axes[idx++] = cross.GetSafeNormal();
            }
        }
    }


    // --- 3) 축마다 투영(overlap) 검사 ---
    float minOverlap = FLT_MAX;
    FVector smallestAxis = FVector::ZeroVector;
    float dC = 0.f;

    for (int a = 0; a < idx; ++a)
    {
        const FVector& L = Axes[a];

        // 투영 반경 계산: r = Σ e_i * |u_i·L|
        const float r1 = LHSExtents.X * FMath::Abs(A1[0] | L) + LHSExtents.Y * FMath::Abs(A1[1] | L) + LHSExtents.Z * FMath::Abs(A1[2] | L);

        const float r2 = RHSExtents.X * FMath::Abs(A2[0] | L) + RHSExtents.Y * FMath::Abs(A2[1] | L) + RHSExtents.Z * FMath::Abs(A2[2] | L);

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

    OutHitResult.bBlockingHit   = true;
    OutHitResult.Location       = contactPoint;
    OutHitResult.ImpactPoint    = contactPoint;
    OutHitResult.Normal         = smallestAxis;
    OutHitResult.ImpactNormal   = -smallestAxis;
    OutHitResult.HitComponent   = Cast<UPrimitiveComponent>(RHSBox);
    OutHitResult.HitActor       = RHSBox->GetOwner();
    return true;
}

bool FCollisionDispatcher::OverlapBoxToSphere(const UBoxComponent* LHSBox, USphereComponent* RHSSphere, FHitResult& OutHitResult)
{
    const FMatrix BoxTM    = LHSBox->GetWorldMatrix();
    const FMatrix WorldToBox = FMatrix::Inverse(BoxTM);

    // 1) Sphere 중심을 Box 로컬 공간으로 변환
    const FVector WorldCenter = RHSSphere->GetWorldLocation();
    const FVector LocalCenter = WorldToBox.TransformPosition(WorldCenter);

    // 2) Box half-extent 내부로 클램프
    const FVector LHSScale = LHSBox->GetWorldScale3D();
    const FVector BoxExtent = LHSBox->GetBoxExtent(); 

    FVector Clamped = LocalCenter;
    Clamped.X = FMath::Clamp(Clamped.X, -BoxExtent.X * LHSScale.X, BoxExtent.X * LHSScale.X);
    Clamped.Y = FMath::Clamp(Clamped.Y, -BoxExtent.Y * LHSScale.Y, BoxExtent.Y * LHSScale.Y);
    Clamped.Z = FMath::Clamp(Clamped.Z, -BoxExtent.Z * LHSScale.Z, BoxExtent.Z * LHSScale.Z);

    // 3) 클램프된 지점을 다시 월드 공간으로 변환
    const FVector ClosestPoint = BoxTM.TransformPosition(Clamped);

    // 4) 구 중심과 가장 가까운 점 사이 거리 비교
    const float  SphereRadius = RHSSphere->GetRadius();
    const FVector RHSScale = RHSSphere->GetWorldScale3D();
    const float ScaledRadius = SphereRadius * FMath::Min(RHSScale.X, FMath::Min(RHSScale.Y, RHSScale.Z));
    
    const FVector Delta        = WorldCenter - ClosestPoint;
    const float  Dist2         = Delta.LengthSquared();
    if (Dist2 <= ScaledRadius * ScaledRadius)
    {
        const float Dist = FMath::Sqrt(Dist2);
        OutHitResult.bBlockingHit = true;
        OutHitResult.ImpactPoint  = ClosestPoint;
        OutHitResult.Location     = ClosestPoint;
        OutHitResult.Normal       = Dist > 0 ? Delta / Dist : FVector::UpVector;
        OutHitResult.ImpactNormal = OutHitResult.Normal;
        OutHitResult.HitComponent = Cast<UPrimitiveComponent>(RHSSphere);
        OutHitResult.HitActor     = RHSSphere->GetOwner();
        return true;
    }
    
    return false;
}

bool FCollisionDispatcher::OverlapBoxToCapsule(const UBoxComponent* LHSBox, UCapsuleComponent* RHSCapsule, FHitResult& OutHitResult)
{
    // --- 1) OBB 파라미터 추출 ---
    // 박스 half-extents 에 월드 스케일 적용
    const FVector boxScale = LHSBox->GetWorldScale3D();
    const FVector BoxExtent = LHSBox->GetBoxExtent();
    const FVector halfExtents = FVector(BoxExtent.X * boxScale.X, BoxExtent.Y * boxScale.Y, BoxExtent.Z * boxScale.Z);

    // 박스 변환 행렬
    const FMatrix boxToWorld  = LHSBox->GetWorldMatrix();
    const FMatrix worldToBox  = FMatrix::Inverse(boxToWorld);

    // --- 2) 캡슐 세그먼트 계산 ---
    // 캡슐 중심
    const FVector capCenter  = RHSCapsule->GetWorldLocation();
    // 캡슐 방향(로컬 Z축)과 스케일
    const FMatrix capRotMat  = RHSCapsule->GetRotationMatrix();
    const FVector capAxis    = FMatrix::TransformVector(FVector(0,0,1), capRotMat).GetSafeNormal();
    const FVector capScale   = RHSCapsule->GetWorldScale3D();
    const float   halfHeight = RHSCapsule->GetCapsuleHalfHeight() * capScale.Z;
    const float   radius     = RHSCapsule->GetCapsuleRadius() * FMath::Min(capScale.X,capScale.Y);

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
    OutHitResult.HitComponent   = Cast<UPrimitiveComponent>(RHSCapsule);
    OutHitResult.HitActor       = RHSCapsule->GetOwner();
    return true;
}

bool FCollisionDispatcher::OverlapSphereToBox(const USphereComponent* LHSSphere, UBoxComponent* RHSBox, FHitResult& OutHitResult)
{
    const FMatrix BoxTM    = RHSBox->GetWorldMatrix();
    const FMatrix WorldToBox = FMatrix::Inverse(BoxTM);

    // 1) Sphere 중심을 Box 로컬 공간으로 변환
    const FVector WorldCenter = LHSSphere->GetWorldLocation();
    const FVector LocalCenter = WorldToBox.TransformPosition(WorldCenter);

    // 2) Box half-extent 내부로 클램프
    FVector Clamped = LocalCenter;
    const FVector BoxExtent = RHSBox->GetBoxExtent();
    Clamped.X = FMath::Clamp(LocalCenter.X, -BoxExtent.X, BoxExtent.X);
    Clamped.Y = FMath::Clamp(LocalCenter.Y, -BoxExtent.Y, BoxExtent.Y);
    Clamped.Z = FMath::Clamp(LocalCenter.Z, -BoxExtent.Z, BoxExtent.Z);

    // 3) 다시 월드 공간 ClosestPoint 계산
    const FVector ClosestPoint = BoxTM.TransformPosition(Clamped);

    // 4) Sphere 반지름과 비교
    const FVector Delta = WorldCenter - ClosestPoint;
    const float Dist2 = Delta.LengthSquared();
    const float R = LHSSphere->GetRadius();
    if (Dist2 <= R * R)
    {
        const float Dist = FMath::Sqrt(Dist2);
        OutHitResult.bBlockingHit = true;
        OutHitResult.ImpactPoint  = ClosestPoint;
        OutHitResult.Location     = ClosestPoint;
        OutHitResult.Normal       = Dist > 0 ? Delta / Dist : FVector::UpVector;
        OutHitResult.ImpactNormal = OutHitResult.Normal;
        OutHitResult.HitComponent = Cast<UPrimitiveComponent>(RHSBox);
        OutHitResult.HitActor     = RHSBox->GetOwner();
        return true;
    }

    return false;
}

bool FCollisionDispatcher::OverlapSphereToSphere(const USphereComponent* LHSSphere, USphereComponent* RHSSphere, FHitResult& OutHitResult)
{
    const FVector C1 = LHSSphere->GetWorldLocation();
    const FVector C2 = RHSSphere->GetWorldLocation();
    
    const FVector C1Scale = LHSSphere->GetWorldScale3D();
    const FVector C2Scale = RHSSphere->GetWorldScale3D();
    
    const float R1 = LHSSphere->GetRadius() * FMath::Min(C1Scale.X, FMath::Min(C1Scale.Y, C1Scale.Z));
    const float R2 = RHSSphere->GetRadius() * FMath::Min(C2Scale.X, FMath::Min(C2Scale.Y, C2Scale.Z));

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
        OutHitResult.HitComponent = Cast<UPrimitiveComponent>(RHSSphere);
        OutHitResult.HitActor     = RHSSphere->GetOwner();
        return true;
    }
    return false;
}

bool FCollisionDispatcher::OverlapSphereToCapsule(const USphereComponent* LHSSphere, UCapsuleComponent* RHSCapsule, FHitResult& OutHitResult)
{
    // 캡슐 중심선(segment) 두 끝점 계산
    const FVector Dir     = RHSCapsule->GetLocalUpVector();
    const FVector C       = RHSCapsule->GetWorldLocation();
    const FVector CapScale = RHSCapsule->GetWorldScale3D();
    const float H         = RHSCapsule->GetCapsuleHalfHeight() * CapScale.Z;
    const FVector P1 = C + Dir * H;
    const FVector P2 = C - Dir * H;

    // 구 중심과 segment의 최단 거리점 계산
    const FVector SC = LHSSphere->GetWorldLocation();
    const FVector Closest = JungleMath::ClosestPointOnSegment(SC, P1, P2);

    // Sphere 반지름과 Capsule 반지름 합으로 비교
    const FVector Delta = SC - Closest;
    const float Dist2   = Delta.LengthSquared();

    const FVector capScale   = RHSCapsule->GetWorldScale3D();
    const float Rsum    = LHSSphere->GetRadius() + RHSCapsule->GetCapsuleRadius() * FMath::Min(capScale.X, capScale.Y);
    if (Dist2 <= Rsum*Rsum)
    {
        const float Dist       = FMath::Sqrt(Dist2);
        OutHitResult.bBlockingHit = true;
        OutHitResult.ImpactPoint  = Closest;
        OutHitResult.Location     = Closest;
        OutHitResult.Normal       = Dist > 0 ? Delta / Dist : FVector::UpVector;
        OutHitResult.ImpactNormal = OutHitResult.Normal;
        OutHitResult.HitComponent = Cast<UPrimitiveComponent>(RHSCapsule);
        OutHitResult.HitActor     = RHSCapsule->GetOwner();
        return true;
    }

    return false;
}

bool FCollisionDispatcher::OverlapCapsuleToBox(const UCapsuleComponent* LHSCapsule, UBoxComponent* RHSBBox, FHitResult& OutHitResult)
{
    // --- 1) OBB 파라미터 추출 ---
    // 박스 half-extents 에 월드 스케일 적용
    const FVector boxScale    = RHSBBox->GetWorldScale3D();
    const FVector BoxExtent = RHSBBox->GetBoxExtent();
    const FVector halfExtents = FVector(BoxExtent.X * boxScale.X, BoxExtent.Y * boxScale.Y, BoxExtent.Z * boxScale.Z);

    // 박스 변환 행렬
    const FMatrix boxToWorld  = RHSBBox->GetWorldMatrix();
    const FMatrix worldToBox  = FMatrix::Inverse(boxToWorld);

    // --- 2) 캡슐 세그먼트 계산 ---
    // 캡슐 중심
    const FVector capCenter  = LHSCapsule->GetWorldLocation();
    // 캡슐 방향(로컬 Z축)과 스케일
    const FMatrix capRotMat  = LHSCapsule->GetRotationMatrix();
    const FVector capAxis    = FMatrix::TransformVector(FVector(0,0,1), capRotMat).GetSafeNormal();
    const FVector capScale   = LHSCapsule->GetWorldScale3D();
    const float   halfHeight = LHSCapsule->GetCapsuleHalfHeight() * capScale.Z;
    const float   radius     = LHSCapsule->GetCapsuleRadius() * FMath::Min(capScale.X, capScale.Y);

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
    OutHitResult.HitComponent   = Cast<UPrimitiveComponent>(RHSBBox);
    OutHitResult.HitActor       = RHSBBox->GetOwner();
    return true;
}

bool FCollisionDispatcher::OverlapCapsuleToSphere(UCapsuleComponent* LHSCapsule, USphereComponent* RHSSphere, FHitResult& OutHitResult)
{
    // 캡슐 중심선(segment) 두 끝점 계산
    const FVector Dir     = LHSCapsule->GetLocalUpVector();
    const FVector C       = LHSCapsule->GetWorldLocation();
    const FVector capScale   = LHSCapsule->GetWorldScale3D();
    const float H         = LHSCapsule->GetCapsuleHalfHeight() * capScale.Z;
    const FVector P1 = C + Dir * H;
    const FVector P2 = C - Dir * H;

    // 구 중심과 segment의 최단 거리점 계산
    const FVector SC = RHSSphere->GetWorldLocation();
    const FVector Closest = JungleMath::ClosestPointOnSegment(SC, P1, P2);

    // Sphere 반지름과 Capsule 반지름 합으로 비교
    const FVector Delta = SC - Closest;
    const float Dist2   = Delta.LengthSquared();
    const float Rsum    = RHSSphere->GetRadius() + LHSCapsule->GetCapsuleRadius() * FMath::Min(capScale.X, capScale.Y);
    if (Dist2 <= Rsum * Rsum)
    {
        const float Dist       = FMath::Sqrt(Dist2);
        OutHitResult.bBlockingHit = true;
        OutHitResult.ImpactPoint  = Closest;
        OutHitResult.Location     = Closest;
        OutHitResult.Normal       = Dist > 0 ? Delta / Dist : FVector::UpVector;
        OutHitResult.ImpactNormal = OutHitResult.Normal;
        OutHitResult.HitComponent = Cast<UPrimitiveComponent>(RHSSphere);
        OutHitResult.HitActor     = RHSSphere->GetOwner();
        return true;
    }

    return false;
}

bool FCollisionDispatcher::OverlapCapsuleToCapsule(UCapsuleComponent* LHSCapsule, UCapsuleComponent* RHSCapsule, FHitResult& OutHitResult)
{
    // --- 1) 내 캡슐 P1, P2 계산 ---
    // 내 월드 스케일, 반절 높이, 반지름

    const FVector scaleA      = LHSCapsule->GetWorldScale3D();
    const float   halfHeightA = LHSCapsule->GetCapsuleHalfHeight() * scaleA.Z;
    const float   radiusA     = LHSCapsule->GetCapsuleRadius() * FMath::Min(scaleA.X, scaleA.Y);
    // 내 월드 중심과 Z축 방향

    const FVector centerA     = LHSCapsule->GetWorldLocation();
    const FMatrix rotA        = LHSCapsule->GetRotationMatrix();
    const FVector axisA       = FMatrix::TransformVector(FVector(0,0,1), rotA).GetSafeNormal();
    
    // 세그먼트 끝점
    const FVector P1 = centerA + axisA * halfHeightA;
    const FVector P2 = centerA - axisA * halfHeightA;

    // --- 2) 상대 캡슐 Q1, Q2 계산 ---
    const FVector scaleB      = RHSCapsule->GetWorldScale3D();
    const float   halfHeightB = RHSCapsule->GetCapsuleHalfHeight() * scaleB.Z;
    const float radiusB     = RHSCapsule->GetCapsuleRadius() * FMath::Min(scaleB.X, scaleB.Y);
    const FVector centerB     = RHSCapsule->GetWorldLocation();
    const FMatrix rotB        = RHSCapsule->GetRotationMatrix();
    const FVector axisB       = FMatrix::TransformVector(FVector(0,0,1), rotB).GetSafeNormal();
    const FVector Q1 = centerB + axisB * halfHeightB;
    const FVector Q2 = centerB - axisB * halfHeightB;

    // --- 3) 두 세그먼트 간 최단 거리 & 점 계산 ---
    FVector ClosestA, ClosestB;
    const float Dist = JungleMath::SegmentDistToSegment(P1, P2, Q1, Q2, ClosestA, ClosestB);
    const float Rsum = radiusA + radiusB;
    if (Dist <= Rsum)
    {
        OutHitResult.bBlockingHit = true;
        OutHitResult.ImpactPoint  = (ClosestA + ClosestB) * 0.5f;
        OutHitResult.Location     = OutHitResult.ImpactPoint;
        OutHitResult.Normal       = (ClosestB - ClosestA).GetSafeNormal();
        OutHitResult.ImpactNormal = OutHitResult.Normal;
        OutHitResult.HitComponent = Cast<UPrimitiveComponent>(RHSCapsule);
        OutHitResult.HitActor     = RHSCapsule->GetOwner();
        return true;
    }
    return false;
}
