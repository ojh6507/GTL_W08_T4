#pragma once
#include "Define.h"
#include "Rotator.h"
#include "Quat.h"

class JungleMath
{
public:
    static FVector4 ConvertV3ToV4(const FVector& vec3);
    static FMatrix CreateModelMatrix(const FVector& translation, const FVector& rotation, const FVector& scale);
    static FMatrix CreateModelMatrix(const FVector& translation, FQuat rotation, const FVector& scale);
    static FMatrix CreateViewMatrix(const FVector& eye, const FVector& target, const FVector& up);
    static FMatrix CreateProjectionMatrix(float fov, float aspect, float nearPlane, float farPlane);
    static FMatrix CreateOrthoProjectionMatrix(float width, float height, float nearPlane, float farPlane);
    
    static FMatrix CreateOrthoOffCenterProjectionMatrix(float left, float right, float bottom, float top, float nearPlane, float farPlane);
    static FMatrix CreateLookAtMatrix(const FVector& eye, const FVector& target, const FVector& up);
    static FVector FVectorRotate(FVector& origin, const FVector& InRotation);
    static FVector FVectorRotate(FVector& origin, const FRotator& InRotation);
    static FMatrix CreateRotationMatrix(const FVector& rotation);
    static FQuat EulerToQuaternion(const FVector& eulerDegrees);
    static FVector QuaternionToEuler(const FQuat& quat);

    static FVector ClosestPointOnSegment(const FVector& Point, const FVector& A, const FVector& B);
    static float SegmentDistToSegment(const FVector& P1, const FVector& P2, const FVector& Q1, const FVector& Q2, FVector& ClosestPoint1, FVector& ClosestPoint2);
};
