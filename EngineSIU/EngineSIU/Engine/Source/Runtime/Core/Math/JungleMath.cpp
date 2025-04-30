#include "Math/JungleMath.h"
#include <DirectXMath.h>
#include "MathUtility.h"

#include "Rotator.h"

using namespace DirectX;


FVector4 JungleMath::ConvertV3ToV4(const FVector& vec3)
{
	FVector4 newVec4;
	newVec4.X = vec3.X;
	newVec4.Y = vec3.Y;
	newVec4.Z = vec3.Z;
	return newVec4;
}

FMatrix JungleMath::CreateModelMatrix(const FVector& translation, const FVector& rotation, const FVector& scale)
{
    FMatrix Translation = FMatrix::CreateTranslationMatrix(translation);

    FMatrix Rotation = FMatrix::CreateRotationMatrix(rotation.X, rotation.Y, rotation.Z);
    //FMatrix Rotation = JungleMath::EulerToQuaternion(rotation).ToMatrix();

    FMatrix Scale = FMatrix::CreateScaleMatrix(scale.X, scale.Y, scale.Z);
    return Scale * Rotation * Translation;
}

FMatrix JungleMath::CreateModelMatrix(const FVector& translation, FQuat rotation, const FVector& scale)
{
    FMatrix Translation = FMatrix::CreateTranslationMatrix(translation);
    FMatrix Rotation = rotation.ToMatrix();
    FMatrix Scale = FMatrix::CreateScaleMatrix(scale.X, scale.Y, scale.Z);
    return Scale * Rotation * Translation;
}
FMatrix JungleMath::CreateViewMatrix(const FVector& eye, const FVector& target, const FVector& up)
{
    FVector zAxis = (target - eye).GetSafeNormal();  // DirectX는 LH이므로 -z가 아니라 +z 사용
    FVector xAxis = (up.Cross(zAxis)).GetSafeNormal();
    FVector yAxis = zAxis.Cross(xAxis);

    FMatrix View;
    View.M[0][0] = xAxis.X; View.M[0][1] = yAxis.X; View.M[0][2] = zAxis.X; View.M[0][3] = 0;
    View.M[1][0] = xAxis.Y; View.M[1][1] = yAxis.Y; View.M[1][2] = zAxis.Y; View.M[1][3] = 0;
    View.M[2][0] = xAxis.Z; View.M[2][1] = yAxis.Z; View.M[2][2] = zAxis.Z; View.M[2][3] = 0;
    View.M[3][0] = -xAxis.Dot(eye);
    View.M[3][1] = -yAxis.Dot(eye);
    View.M[3][2] = -zAxis.Dot(eye);
    View.M[3][3] = 1;

    return View;
}

FMatrix JungleMath::CreateProjectionMatrix(float fov, float aspect, float nearPlane, float farPlane)
{
    float tanHalfFOV = tan(fov / 2.0f);
    float depth = farPlane - nearPlane;

    FMatrix Projection = {};
    Projection.M[0][0] = 1.0f / (aspect * tanHalfFOV);
    Projection.M[1][1] = 1.0f / tanHalfFOV;
    Projection.M[2][2] = farPlane / depth;
    Projection.M[2][3] = 1.0f;
    Projection.M[3][2] = -(nearPlane * farPlane) / depth;
    Projection.M[3][3] = 0.0f;  

    return Projection;
}

FMatrix JungleMath::CreateOrthoProjectionMatrix(float width, float height, float nearPlane, float farPlane)
{
    float r = width * 0.5f;
    float t = height * 0.5f;
    float invDepth = 1.0f / (farPlane - nearPlane);

    FMatrix Projection = {};
    Projection.M[0][0] = 1.0f / r;
    Projection.M[1][1] = 1.0f / t;
    Projection.M[2][2] = invDepth;
    Projection.M[3][2] = -nearPlane * invDepth;
    Projection.M[3][3] = 1.0f;

    return Projection;
}

FMatrix JungleMath::CreateOrthoOffCenterProjectionMatrix( float left, float right, float bottom, float top, float nearPlane, float farPlane)
{
    FMatrix Projection = {};
    Projection.M[0][0] = 2.0f / (right - left);
    Projection.M[1][1] = 2.0f / (top - bottom);
    Projection.M[2][2] = 1.0f / (farPlane - nearPlane);
    Projection.M[3][0] = (left + right) / (left - right);
    Projection.M[3][1] = (top + bottom) / (bottom - top);
    Projection.M[3][2] = nearPlane / (nearPlane - farPlane);
    Projection.M[3][3] = 1.0f;
    return Projection;
}
FVector JungleMath::FVectorRotate(FVector& origin, const FVector& InRotation)
{
    FQuat quaternion = JungleMath::EulerToQuaternion(InRotation);
    // 쿼터니언을 이용해 벡터 회전 적용
    return quaternion.RotateVector(origin);
}
FQuat JungleMath::EulerToQuaternion(const FVector& eulerDegrees)
{
    float yaw = FMath::DegreesToRadians(eulerDegrees.Z);   // Z축 Yaw
    float pitch = FMath::DegreesToRadians(eulerDegrees.Y); // Y축 Pitch
    float roll = FMath::DegreesToRadians(eulerDegrees.X);  // X축 Roll

    float halfYaw = yaw * 0.5f;
    float halfPitch = pitch * 0.5f;
    float halfRoll = roll * 0.5f;

    float cosYaw = cos(halfYaw);
    float sinYaw = sin(halfYaw);
    float cosPitch = cos(halfPitch);
    float sinPitch = sin(halfPitch);
    float cosRoll = cos(halfRoll);
    float sinRoll = sin(halfRoll);

    FQuat quat;
    quat.W = cosYaw * cosPitch * cosRoll + sinYaw * sinPitch * sinRoll;
    quat.X = cosYaw * cosPitch * sinRoll - sinYaw * sinPitch * cosRoll;
    quat.Y = cosYaw * sinPitch * cosRoll + sinYaw * cosPitch * sinRoll;
    quat.Z = sinYaw * cosPitch * cosRoll - cosYaw * sinPitch * sinRoll;

    quat.Normalize();
    return quat;
}

FMatrix JungleMath::CreateLookAtMatrix(const FVector& eye, const FVector& target, const FVector& up)
{
    FVector zAxis = (target - eye).GetSafeNormal();
    FVector xAxis = up.Cross(zAxis).GetSafeNormal();
    FVector yAxis = zAxis.Cross(xAxis);

    FMatrix view = {};
    view.M[0][0] = xAxis.X; view.M[0][1] = yAxis.X; view.M[0][2] = zAxis.X; view.M[0][3] = 0.0f;
    view.M[1][0] = xAxis.Y; view.M[1][1] = yAxis.Y; view.M[1][2] = zAxis.Y; view.M[1][3] = 0.0f;
    view.M[2][0] = xAxis.Z; view.M[2][1] = yAxis.Z; view.M[2][2] = zAxis.Z; view.M[2][3] = 0.0f;
    // 5) 위치 보정
    view.M[3][0] = -FVector::DotProduct(xAxis, eye);
    view.M[3][1] = -FVector::DotProduct(yAxis, eye);
    view.M[3][2] = -FVector::DotProduct(zAxis, eye);
    view.M[3][3] = 1.0f;

    return view;
}

FVector JungleMath::QuaternionToEuler(const FQuat& quat)
{
    FVector euler;

    // 쿼터니언 정규화
    FQuat q = quat;
    q.Normalize();

    // Yaw (Z 축 회전)
    float sinYaw = 2.0f * (q.W * q.Z + q.X * q.Y);
    float cosYaw = 1.0f - 2.0f * (q.Y * q.Y + q.Z * q.Z);
    euler.Z = FMath::RadiansToDegrees(atan2(sinYaw, cosYaw));

    // Pitch (Y 축 회전, 짐벌락 방지)
    float sinPitch = 2.0f * (q.W * q.Y - q.Z * q.X);
    if (fabs(sinPitch) >= 1.0f)
    {
        euler.Y = FMath::RadiansToDegrees(copysign(PI / 2, sinPitch)); // 🔥 Gimbal Lock 방지
    }
    else
    {
        euler.Y = FMath::RadiansToDegrees(asin(sinPitch));
    }

    // Roll (X 축 회전)
    float sinRoll = 2.0f * (q.W * q.X + q.Y * q.Z);
    float cosRoll = 1.0f - 2.0f * (q.X * q.X + q.Y * q.Y);
    euler.X = FMath::RadiansToDegrees(atan2(sinRoll, cosRoll));
    return euler;
}

FVector JungleMath::ClosestPointOnSegment(const FVector& Point, const FVector& A, const FVector& B)
{
    const FVector AB = B - A;
    const float AB2 = AB | AB;                              // Dot(AB, AB)
    if (AB2 <= KINDA_SMALL_NUMBER)                    // A와 B가 거의 동일한 경우
    {
        return A;
    }
    float t = (Point - A) | AB;                       // Dot(Point-A, AB)
    t = t / AB2;                                      // 무한 직선 상의 파라메터
    t = FMath::Clamp(t, 0.0f, 1.0f);                  // 선분 범위에 한정
    return A + AB * t;                                // 선분 위 점
}

float JungleMath::SegmentDistToSegment(const FVector& P1, const FVector& P2, const FVector& Q1, const FVector& Q2, FVector& ClosestPoint1,
    FVector& ClosestPoint2)
{
    const FVector   u = P2 - P1;
    const FVector   v = Q2 - Q1;
    const FVector   w = P1 - Q1;
    const float    a = u | u;         // dot(u,u)
    const float    b = u | v;         // dot(u,v)
    const float    c = v | v;         // dot(v,v)
    const float    d = u | w;         // dot(u,w)
    const float    e = v | w;         // dot(v,w)
    const float    D = a*c - b*b;     // 항상 ≥ 0
    float    sc, sN, sD = D;    // sc = sN / sD
    float    tc, tN, tD = D;    // tc = tN / tD

    // 두 선분이 거의 평행한 경우
    if (D < KINDA_SMALL_NUMBER)
    {
        sN = 0.0f;    // P1 쪽 고정
        sD = 1.0f;
        tN = e;
        tD = c;
    }
    else
    {
        // 무한 직선상 최단점 파라미터
        sN =  (b*e - c*d);
        tN =  (a*e - b*d);
        // s 범위 밖 체크
        if (sN < 0.0f)
        {
            sN = 0.0f;
            tN = e;
            tD = c;
        }
        else if (sN > sD)
        {
            sN = sD;
            tN = e + b;
            tD = c;
        }
    }

    // t 범위 밖 체크
    if (tN < 0.0f)
    {
        tN = 0.0f;
        if (-d < 0.0f)       sN = 0.0f;
        else if (-d > a)     sN = sD;
        else
        {
            sN = -d;
            sD = a;
        }
    }
    else if (tN > tD)
    {
        tN = tD;
        if ((-d + b) < 0.0f)         sN = 0.0f;
        else if ((-d + b) > a)       sN = sD;
        else
        {
            sN = (-d + b);
            sD = a;
        }
    }

    // 파라미터 sc, tc 계산
    sc = (FMath::Abs(sN) < KINDA_SMALL_NUMBER) ? 0.0f : sN / sD;
    tc = (FMath::Abs(tN) < KINDA_SMALL_NUMBER) ? 0.0f : tN / tD;

    // 최단 점들 계산
    ClosestPoint1 = P1 + u * sc;
    ClosestPoint2 = Q1 + v * tc;

    // 두 점 사이 거리 반환
    const FVector dP = ClosestPoint1 - ClosestPoint2;
    return dP.Length();
}

float JungleMath::ClampAngle(float AngleDegrees, float MinAngleDegrees, float MaxAngleDegrees)
{
    const float MaxDelta = FRotator::ClampAxis(MaxAngleDegrees - MinAngleDegrees) * 0.5f;			// 0..180
    const float RangeCenter = FRotator::ClampAxis(MinAngleDegrees + MaxDelta);						// 0..360
    const float DeltaFromCenter = FRotator::NormalizeAxis(AngleDegrees - RangeCenter);				// -180..180

    // maybe clamp to nearest edge
    if (DeltaFromCenter > MaxDelta)
    {
        return FRotator::NormalizeAxis(RangeCenter + MaxDelta);
    }
    else if (DeltaFromCenter < -MaxDelta)
    {
        return FRotator::NormalizeAxis(RangeCenter - MaxDelta);
    }

    // already in range, just return it
    return FRotator::NormalizeAxis(AngleDegrees);
}

FVector JungleMath::FVectorRotate(FVector& origin, const FRotator& InRotation)
{
    return InRotation.ToQuaternion().RotateVector(origin);
}

FMatrix JungleMath::CreateRotationMatrix(const FVector& rotation)
{
    XMVECTOR quatX = XMQuaternionRotationAxis(XMVectorSet(1, 0, 0, 0), FMath::DegreesToRadians(rotation.X));
    XMVECTOR quatY = XMQuaternionRotationAxis(XMVectorSet(0, 1, 0, 0), FMath::DegreesToRadians(rotation.Y));
    XMVECTOR quatZ = XMQuaternionRotationAxis(XMVectorSet(0, 0, 1, 0), FMath::DegreesToRadians(rotation.Z));

    XMVECTOR rotationQuat = XMQuaternionMultiply(quatZ, XMQuaternionMultiply(quatY, quatX));
    rotationQuat = XMQuaternionNormalize(rotationQuat);  // 정규화 필수

    XMMATRIX rotationMatrix = XMMatrixRotationQuaternion(rotationQuat);
    FMatrix result = FMatrix::Identity;  // 기본값 설정 (단위 행렬)

    for (int i = 0; i < 4; i++)
    {
        for (int j = 0; j < 4; j++)
        {
            result.M[i][j] = rotationMatrix.r[i].m128_f32[j];  // XMMATRIX에서 FMatrix로 값 복사
        }
    }
    return result;
}

FQuat FQuat::Slerp_NotNormalized(const FQuat& Quat1, const FQuat& Quat2, float Slerp)
{
    // Get cosine of angle between quats.
    const float RawCosom =
        Quat1.X * Quat2.X +
        Quat1.Y * Quat2.Y +
        Quat1.Z * Quat2.Z +
        Quat1.W * Quat2.W;
    // Unaligned quats - compensate, results in taking shorter route.
    const float Cosom = FMath::FloatSelect(RawCosom, RawCosom, -RawCosom);

    float Scale0, Scale1;

    if (Cosom < float(0.9999f))
    {
        const float Omega = FMath::Acos(Cosom);
        const float InvSin = float(1.f) / FMath::Sin(Omega);
        Scale0 = FMath::Sin((float(1.f) - Slerp) * Omega) * InvSin;
        Scale1 = FMath::Sin(Slerp * Omega) * InvSin;
    }
    else
    {
        // Use linear interpolation.
        Scale0 = float(1.0f) - Slerp;
        Scale1 = Slerp;
    }

    // In keeping with our flipped Cosom:
    Scale1 = FMath::FloatSelect(RawCosom, Scale1, -Scale1);

    FQuat Result;

    Result.X = Scale0 * Quat1.X + Scale1 * Quat2.X;
    Result.Y = Scale0 * Quat1.Y + Scale1 * Quat2.Y;
    Result.Z = Scale0 * Quat1.Z + Scale1 * Quat2.Z;
    Result.W = Scale0 * Quat1.W + Scale1 * Quat2.W;

    return Result;
}

FVector FMath::VInterpTo(const FVector& Current, const FVector& Target, float DeltaTime, float InterpSpeed)
{
    // If no interp speed, jump to target value
    if (InterpSpeed <= 0.f)
    {
        return Target;
    }

    // Distance to reach
    const FVector Dist = Target - Current;

    // If distance is too small, just set the desired location
    if (Dist.LengthSquared() < KINDA_SMALL_NUMBER)
    {
        return Target;
    }

    // Delta Move, Clamp so we do not over shoot.
    const FVector DeltaMove = Dist * FMath::Clamp<float>(InterpSpeed * DeltaTime, 0.f, 1.f);

    return Current + DeltaMove;
}

FQuat FMath::QInterpTo(const FQuat& Current, const FQuat& Target, float DeltaTime, float InterpSpeed)
{
    // If no interp speed, jump to target value
    if (InterpSpeed <= 0.f)
    {
        return Target;
    }

    // If the values are nearly equal, just return Target and assume we have reached our destination.
    if (Current.Equals(Target))
    {
        return Target;
    }

    return FQuat::Slerp(Current, Target, FMath::Clamp<float>(InterpSpeed * DeltaTime, 0.f, 1.f));
}
