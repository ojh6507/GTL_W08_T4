#pragma once
#include "Vector.h"

struct FMatrix;
struct FTransform;

struct FBox
{
public:
	FVector Min;
	FVector Max;

	FBox()
		: Min(FVector::ZeroVector)
		, Max(FVector::ZeroVector)
	{
	}

	FBox(const FVector InMin, const FVector InMax)
		: Min(InMin)
		, Max(InMax)
	{
	}

public:
	FORCEINLINE FBox& operator+=(const FBox& Other)
	{
		Min.X = FMath::Min(Min.X, Other.Min.X);
		Min.Y = FMath::Min(Min.Y, Other.Min.Y);
		Min.Z = FMath::Min(Min.Z, Other.Min.Z);

		Max.X = FMath::Max(Max.X, Other.Max.X);
		Max.Y = FMath::Max(Max.Y, Other.Max.Y);
		Max.Z = FMath::Max(Max.Z, Other.Max.Z);

		return *this;
	}

	FORCEINLINE FBox operator+(const FBox& Other) const
	{
		return FBox(*this) += Other;
	}

	FORCEINLINE FBox operator+=(const FVector& Other)
	{
		Min.X = FMath::Min(Min.X, Other.X);
		Min.Y = FMath::Min(Min.Y, Other.Y);
		Min.Z = FMath::Min(Min.Z, Other.Z);

		Max.X = FMath::Max(Max.X, Other.X);
		Max.Y = FMath::Max(Max.Y, Other.Y);
		Max.Z = FMath::Max(Max.Z, Other.Z);

		return *this;
	}

	FORCEINLINE FBox operator+(const FVector& Other) const
	{
		return FBox(*this) += Other;
	}

	FORCEINLINE float GetWidth() const { return Max.X - Min.X; }
	FORCEINLINE float GetHeight() const { return Max.Y - Min.Y; }
	FORCEINLINE float GetDepth() const { return Max.Z - Min.Z; }

	FORCEINLINE float ComputeSquaredDIstanceToPoint(const FVector Point) const
	{
		float DistSqaured = 0.0f;

		if (Point.X < Min.X)
		{
			DistSqaured += FMath::Square(Point.X - Min.X);
		}
		else if (Point.X > Max.X)
		{
			DistSqaured += FMath::Square(Point.X - Max.X);
		}

		if (Point.Y < Min.Y)
		{
			DistSqaured += FMath::Square(Point.Y - Min.Y);
		}
		else if (Point.Y > Max.Y)
		{
			DistSqaured += FMath::Square(Point.Y - Max.Y);
		}

		if (Point.Z < Min.Z)
		{
			DistSqaured += FMath::Square(Point.Z - Min.Z);
		}
		else if (Point.Z > Max.Z)
		{
			DistSqaured += FMath::Square(Point.Z - Max.Z);
		}

		return DistSqaured;
	}

	FORCEINLINE float ComputeSquaredDistanceToBox(const FBox Other) const
	{
		FVector AxisDist = FVector(FMath::Abs(GetCenter().X - Other.GetCenter().X), FMath::Abs(GetCenter().Y - Other.GetCenter().Y), FMath::Abs(GetCenter().Z - Other.GetCenter().Z)) - (GetExtent() + Other.GetExtent());
		AxisDist = (AxisDist > FVector::ZeroVector) ? AxisDist : FVector::ZeroVector;
		return AxisDist.Dot(AxisDist);
	}

	FORCEINLINE FBox ExpandBy(const float W) const
	{
		return FBox(Min - FVector(W, W, W), Max + FVector(W, W, W));
	}

	FORCEINLINE FBox ExpandBy(const FVector V) const
	{
		return FBox(Min - V, Max + V);
	}

	FORCEINLINE FBox ShiftBy(const FVector Offet) const
	{
		return FBox(Min + Offet, Max + Offet);
	}

	FORCEINLINE FBox MoveTo(const FVector Position) const
	{
		const FVector Offset = Position - GetCenter();
		return ShiftBy(Offset);
	}

	FORCEINLINE FVector GetCenter() const
	{
		return (Min + Max) * 0.5f;
	}

	FORCEINLINE FVector GetExtent() const
	{
		return (Max - Min) * 0.5f;
	}

	FORCEINLINE FVector GetSize() const
	{
		return Max - Min;
	}

	FORCEINLINE float GetVolume() const
	{
		return  GetSize().X * GetSize().Y * GetSize().Z;
	}
};

struct FSphere
{
public:
	FVector Center;
	float Radius;
	FSphere()
		: Center(FVector::ZeroVector)
		, Radius(0.0f)
	{
	}
	FSphere(const FVector InCenter, const float InRadius)
		: Center(InCenter)
		, Radius(InRadius)
	{
	};

	FORCEINLINE float GetVolume() const
	{
		return 4.0f / 3.0f * PI * (Radius * Radius * Radius);
	}

};

struct FBoxSphereBounds
{
public:
	FVector Origin;
    float padding = 0.0f;
	FVector BoxExtent;
	float SphereRadius;

	FBoxSphereBounds()
		: Origin(FVector::ZeroVector)
		, BoxExtent(FVector::ZeroVector)
		, SphereRadius(0.0f)
	{
	}

	FBoxSphereBounds(const FVector InOrigin, const FVector InBoxExtent, const float InSphereRadius)
		: Origin(InOrigin)
		, BoxExtent(InBoxExtent)
		, SphereRadius(InSphereRadius)
	{
	}

	FBoxSphereBounds(const FVector InOrigin, const float InSphereRadius)
		: Origin(InOrigin)
		, BoxExtent(FVector::ZeroVector)
		, SphereRadius(InSphereRadius)
	{
	}

	FBoxSphereBounds(const FVector InOrigin, const FVector InBoxExtent)
		: Origin(InOrigin)
		, BoxExtent(InBoxExtent)
		, SphereRadius(0.0f)
	{
	}

	FBoxSphereBounds(const FBox& InBox)
	{
		FVector LocalExtent;
		Origin = InBox.GetCenter();
		BoxExtent = InBox.GetExtent();
		SphereRadius = BoxExtent.Length();
	}

	FORCEINLINE FBox GetBox() const
	{
		return FBox(Origin - BoxExtent, Origin + BoxExtent);
	}

	FORCEINLINE FVector GetCenter() const
	{
		return Origin;
	}

	FORCEINLINE FSphere GetSphere() const
	{
		return FSphere(Origin, SphereRadius);
	}

	FORCEINLINE FBoxSphereBounds ExpandBy(const float V) const
	{
		return FBoxSphereBounds(Origin, { BoxExtent.X + V, BoxExtent.Y + V, BoxExtent.Z + V }, SphereRadius + V);
	}

	FBoxSphereBounds TransformBy(const FMatrix& InMatrix) const;

	//FBoxSphereBounds TransformBy(const FTransform& InTransform) const;
};

