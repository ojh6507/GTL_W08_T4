#include "Define.h"

bool FBoundingBox::Intersect(const FVector& rayOrigin, const FVector& rayDir, float& outDistance) const
{
    float tmin = -FLT_MAX;
    float tmax = FLT_MAX;
    constexpr float epsilon = 1e-6f;

    // X축 처리
    if (fabs(rayDir.X) < epsilon)
    {
        // 레이가 X축 방향으로 거의 평행한 경우,
        // 원점의 x가 박스 [min.X, max.X] 범위 밖이면 교차 없음
        if (rayOrigin.X < min.X || rayOrigin.X > max.X)
            return false;
    }
    else
    {
        float t1 = (min.X - rayOrigin.X) / rayDir.X;
        float t2 = (max.X - rayOrigin.X) / rayDir.X;
        if (t1 > t2)  std::swap(t1, t2);

        // tmin은 "현재까지의 교차 구간 중 가장 큰 min"
        tmin = (t1 > tmin) ? t1 : tmin;
        // tmax는 "현재까지의 교차 구간 중 가장 작은 max"
        tmax = (t2 < tmax) ? t2 : tmax;
        if (tmin > tmax)
            return false;
    }

    // Y축 처리
    if (fabs(rayDir.Y) < epsilon)
    {
        if (rayOrigin.Y < min.Y || rayOrigin.Y > max.Y)
            return false;
    }
    else
    {
        float t1 = (min.Y - rayOrigin.Y) / rayDir.Y;
        float t2 = (max.Y - rayOrigin.Y) / rayDir.Y;
        if (t1 > t2)  std::swap(t1, t2);

        tmin = (t1 > tmin) ? t1 : tmin;
        tmax = (t2 < tmax) ? t2 : tmax;
        if (tmin > tmax)
            return false;
    }

    // Z축 처리
    if (fabs(rayDir.Z) < epsilon)
    {
        if (rayOrigin.Z < min.Z || rayOrigin.Z > max.Z)
            return false;
    }
    else
    {
        float t1 = (min.Z - rayOrigin.Z) / rayDir.Z;
        float t2 = (max.Z - rayOrigin.Z) / rayDir.Z;
        if (t1 > t2)  std::swap(t1, t2);

        tmin = (t1 > tmin) ? t1 : tmin;
        tmax = (t2 < tmax) ? t2 : tmax;
        if (tmin > tmax)
            return false;
    }

    // 여기까지 왔으면 교차 구간 [tmin, tmax]가 유효하다.
    // tmax < 0 이면, 레이가 박스 뒤쪽에서 교차하므로 화면상 보기엔 교차 안 한다고 볼 수 있음
    if (tmax < 0.0f)
        return false;

    // outDistance = tmin이 0보다 크면 그게 레이가 처음으로 박스를 만나는 지점
    // 만약 tmin < 0 이면, 레이의 시작점이 박스 내부에 있다는 의미이므로, 거리를 0으로 처리해도 됨.
    outDistance = (tmin >= 0.0f) ? tmin : 0.0f;

    return true;
}

FBoundingBox FBoundingBox::TransformWorld(const FMatrix& worldMatrix) const
{
    // 로컬 공간에서 중심과 half-extents 계산
    FVector center = (min + max) * 0.5f;
    FVector extents = (max - min) * 0.5f;

    // 중심을 월드 공간으로 변환
    FVector worldCenter = worldMatrix.TransformPosition(center);

    // 행렬의 상위 3x3 부분의 절대값 벡터를 사용하여 각 축에 대한 확장량 계산
    // (FVector::GetAbs()와 DotProduct() 함수가 사용됨)
    FVector row0(worldMatrix.M[0][0], worldMatrix.M[0][1], worldMatrix.M[0][2]);
    FVector row1(worldMatrix.M[1][0], worldMatrix.M[1][1], worldMatrix.M[1][2]);
    FVector row2(worldMatrix.M[2][0], worldMatrix.M[2][1], worldMatrix.M[2][2]);

    row0 = FVector::GetAbs(row0);
    row1 = FVector::GetAbs(row1);
    row2 = FVector::GetAbs(row2);

    float worldExtentX = row0.Dot(extents);
    float worldExtentY = row1.Dot(extents);
    float worldExtentZ = row2.Dot(extents);

    FVector worldExtents(worldExtentX, worldExtentY, worldExtentZ);

    return FBoundingBox(worldCenter - worldExtents, worldCenter + worldExtents);
}

void FBoundingBox::Expand(const FVector& point)
{
    min.X = std::min(min.X, point.X);
    min.Y = std::min(min.Y, point.Y);
    min.Z = std::min(min.Z, point.Z);

    max.X = std::max(max.X, point.X);
    max.Y = std::max(max.Y, point.Y);
    max.Z = std::max(max.Z, point.Z);
}

FVector FBoundingBox::GetCorner(int index) const
{
    return FVector(
        (index & 1) ? max.X : min.X,
        (index & 2) ? max.Y : min.Y,
        (index & 4) ? max.Z : min.Z
    );
}

bool FBoundingBox::Intersect(const FBoundingBox& Other) const
{
    return (min.X <= Other.max.X && max.X >= Other.min.X)
        && (min.Y <= Other.max.Y && max.Y >= Other.min.Y)
        && (min.Z <= Other.max.Z && max.Z >= Other.min.Z);
}
