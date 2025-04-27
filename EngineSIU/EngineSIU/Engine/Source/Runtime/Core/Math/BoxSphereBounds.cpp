#include "BoxSphereBounds.h"

#include "Matrix.h"
#include "Vector4.h"

FBoxSphereBounds FBoxSphereBounds::TransformBy(const FMatrix& InMatrix) const
{
	FBoxSphereBounds NewBounds;

	const FVector4 m0 = FVector4(InMatrix.M[0][0], InMatrix.M[0][1], InMatrix.M[0][2], InMatrix.M[0][3]);
	const FVector4 m1 = FVector4(InMatrix.M[1][0], InMatrix.M[1][1], InMatrix.M[1][2], InMatrix.M[1][3]);
	const FVector4 m2 = FVector4(InMatrix.M[2][0], InMatrix.M[2][1], InMatrix.M[2][2], InMatrix.M[2][3]);
	const FVector4 m3 = FVector4(InMatrix.M[3][0], InMatrix.M[3][1], InMatrix.M[3][2], InMatrix.M[3][3]);

	FVector4 NewOrigin = FVector4::MultiplyVector4(FVector4(Origin.X, Origin.X, Origin.X, Origin.X), m0);
	NewOrigin = FVector4::VectorMultiplyAdd(FVector4(Origin.Y, Origin.Y, Origin.Y, Origin.Y), m1, NewOrigin);
	NewOrigin = FVector4::VectorMultiplyAdd(FVector4(Origin.Z, Origin.Z, Origin.Z, Origin.Z), m2, NewOrigin);
	NewOrigin = FVector4(NewOrigin.X + m3.X, NewOrigin.Y + m3.Y, NewOrigin.Z + m3.Z, NewOrigin.W + m3.W);

	const FVector4 MultiplyExXMO = FVector4::MultiplyVector4(FVector4(BoxExtent.X, BoxExtent.X, BoxExtent.X, BoxExtent.X), m0);
	FVector4 NewExtent = FVector4(FMath::Abs(MultiplyExXMO.X), FMath::Abs(MultiplyExXMO.Y), FMath::Abs(MultiplyExXMO.Z), FMath::Abs(MultiplyExXMO.W));
	const FVector4 MultiplyNewExYM1 = FVector4::VectorMultiplyAdd(FVector4(BoxExtent.Y, BoxExtent.Y, BoxExtent.Y, BoxExtent.Y), m1, NewExtent);
	NewExtent = FVector4(FMath::Abs(MultiplyNewExYM1.X), FMath::Abs(MultiplyNewExYM1.Y), FMath::Abs(MultiplyNewExYM1.Z), FMath::Abs(MultiplyNewExYM1.W));
	const FVector4 MultiplyNewExZM2 = FVector4::VectorMultiplyAdd(FVector4(BoxExtent.Z, BoxExtent.Z, BoxExtent.Z, BoxExtent.Z), m2, NewExtent);
	NewExtent = FVector4(FMath::Abs(MultiplyNewExZM2.X), FMath::Abs(MultiplyNewExZM2.Y), FMath::Abs(MultiplyNewExZM2.Z), FMath::Abs(MultiplyNewExZM2.W));

	NewBounds.Origin = FVector(NewOrigin.X, NewOrigin.Y, NewOrigin.Z);
	NewBounds.BoxExtent = FVector(NewExtent.X, NewExtent.Y, NewExtent.Z);

	FVector4 MaxRadius = FVector4::MultiplyVector4(m0, m0);
	MaxRadius = FVector4::VectorMultiplyAdd(m1, m1, MaxRadius);
	MaxRadius = FVector4::VectorMultiplyAdd(m2, m2, MaxRadius);
	MaxRadius = FVector4::VectorMax(MaxRadius, FVector4::VectorMax(FVector4(MaxRadius.Y, MaxRadius.Y, MaxRadius.Y, MaxRadius.Y), FVector4(MaxRadius.Z, MaxRadius.Z, MaxRadius.Z, MaxRadius.Z)));

	NewBounds.SphereRadius = FMath::Sqrt(MaxRadius.X) * SphereRadius;

	float const BoxExtentMagnitude = FMath::Sqrt(BoxExtent.X * BoxExtent.X + BoxExtent.Y * BoxExtent.Y + BoxExtent.Z * BoxExtent.Z);
	NewBounds.SphereRadius += BoxExtentMagnitude * FMath::Sqrt(MaxRadius.W);

	return NewBounds;
}
//
// FBoxSphereBounds FBoxSphereBounds::TransformBy(const FTransform& InTransform) const
// {
// 	return TransformBy(InTransform.ToMatrixWithScale());
// }
