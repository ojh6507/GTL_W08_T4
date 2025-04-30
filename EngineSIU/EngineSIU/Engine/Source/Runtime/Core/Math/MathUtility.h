#pragma once
#include <cmath>
#include <numbers>
#include "Core/HAL/PlatformType.h"


#define PI                   (3.1415926535897932f)
#define SMALL_NUMBER         (1.e-8f)
#define KINDA_SMALL_NUMBER   (1.e-4f)

#define PI_DOUBLE            (3.141592653589793238462643383279502884197169399)

struct FVector;
struct FQuat;
struct FMath
{
	/** A와 B중에 더 작은 값을 반환합니다. */
	template <typename T>
	[[nodiscard]] static FORCEINLINE constexpr T Min(const T A, const T B)
	{
		return A < B ? A : B;
	}

	/** A와 B중에 더 큰 값을 반환합니다. */
	template <typename T>
	[[nodiscard]] static FORCEINLINE constexpr T Max(const T A, const T B)
	{
		return B < A ? A : B;
	}

	/** X를 Min과 Max의 사이의 값으로 클램핑 합니다. */
	template <typename T>
	[[nodiscard]] static FORCEINLINE constexpr T Clamp(const T X, const T MinValue, const T MaxValue)
	{
		return Max(Min(X, MaxValue), MinValue);
	}

	/** A의 절댓값을 구합니다. */
	template <typename T>
	[[nodiscard]] static FORCEINLINE constexpr T Abs(const T A)
	{
		return A < T(0) ? -A : A;
	}

    /** Returns 1, 0, or -1 depending on relation of T to 0 */
    template< class T > 
    static constexpr FORCEINLINE T Sign( const T A )
	{
        return (A > (T)0) ? (T)1 : ((A < (T)0) ? (T)-1 : (T)0);
	}

	/** A의 제곱을 구합니다. */
	template <typename T>
	[[nodiscard]] static FORCEINLINE constexpr T Pow(const T A)
	{
		return A * A;
	}

    static FORCEINLINE float Pow(const float A, const float B ) { return powf(A,B); }
    static FORCEINLINE double Pow(const double A, const double B ) { return pow(A,B); }

	// A의 제곱근을 구합니다.
	[[nodiscard]] static FORCEINLINE float Sqrt(const float A) { return sqrtf(A); }
	[[nodiscard]] static FORCEINLINE double Sqrt(const double A) { return sqrt(A); }

	/** A의 역제곱근을 구합니다. */
	[[nodiscard]] static FORCEINLINE float InvSqrt(const float A) { return 1.0f / sqrtf(A); }
	[[nodiscard]] static FORCEINLINE double InvSqrt(const double A) { return 1.0 / sqrt(A); }

	/** A와 B를 Alpha값에 따라 선형으로 보간합니다. */
	template <typename T>
	[[nodiscard]] static FORCEINLINE constexpr T Lerp(const T& A, const T& B, float Alpha)
	{
		return static_cast<T>((A * (1.0f - Alpha)) + (B * Alpha));
	}

	/** A와 B를 Alpha값에 따라 선형으로 보간합니다. */
	template <typename T>
	[[nodiscard]] static FORCEINLINE constexpr T Lerp(const T& A, const T& B, double Alpha)
	{
		return static_cast<T>((A * (1.0 - Alpha)) + (B * Alpha));
	}

	template <typename T>
	[[nodiscard]] static FORCEINLINE constexpr auto RadiansToDegrees(const T& RadVal) -> decltype(RadVal * (180.0f / PI))
	{
		return RadVal * (180.0f / PI);
	}

	[[nodiscard]] static FORCEINLINE constexpr float RadiansToDegrees(const float RadVal)
	{
		return RadVal * (180.0f / PI);
	}

	[[nodiscard]] static FORCEINLINE constexpr double RadiansToDegrees(const double RadVal)
	{
		return RadVal * (180.0 / PI_DOUBLE);
	}

	template <typename T>
	[[nodiscard]] static FORCEINLINE constexpr auto DegreesToRadians(const T& DegVal) -> decltype(DegVal * (PI / 180.0f))
	{
		return DegVal * (PI / 180.0f);
	}

	[[nodiscard]] static FORCEINLINE constexpr float DegreesToRadians(const float DegVal)
	{
		return DegVal * (PI / 180.0f);
	}

	[[nodiscard]] static FORCEINLINE constexpr double DegreesToRadians(const double DegVal)
	{
		return DegVal * (PI_DOUBLE / 180.0);
	}

    // Returns e^Value
    static FORCEINLINE float Exp(const float Value ) { return expf(Value); }
    static FORCEINLINE double Exp(const double Value) { return exp(Value); }

    // Returns 2^Value
    static FORCEINLINE float Exp2(const float Value ) { return powf(2.f, Value); /*exp2f(Value);*/ }
    static FORCEINLINE double Exp2(const double Value) { return pow(2.0, Value); /*exp2(Value);*/ }

    static FORCEINLINE float Loge(const float Value ) {	return logf(Value); }
    static FORCEINLINE double Loge(const double Value) { return log(Value); }

    static FORCEINLINE float LogX(const float Base, const float Value ) { return Loge(Value) / Loge(Base); }
    static FORCEINLINE double LogX(const double Base, const double Value) { return Loge(Value) / Loge(Base); }

    // 1.0 / Loge(2) = 1.4426950f
    static FORCEINLINE float Log2(const float Value ) { return Loge(Value) * std::numbers::log2e_v<float>; }	
    // 1.0 / Loge(2) = 1.442695040888963387
    static FORCEINLINE double Log2(const double Value) { return Loge(Value) * std::numbers::log2e; }


	[[nodiscard]] static FORCEINLINE double Cos(const double RadVal) { return cos(RadVal); }
	[[nodiscard]] static FORCEINLINE float Cos(const float RadVal) { return cosf(RadVal); }

	[[nodiscard]] static FORCEINLINE double Sin(const double RadVal) { return sin(RadVal); }
	[[nodiscard]] static FORCEINLINE float Sin(const float RadVal) { return sinf(RadVal); }

	[[nodiscard]] static FORCEINLINE double Tan(const double RadVal) { return tan(RadVal); }
	[[nodiscard]] static FORCEINLINE float Tan(const float RadVal) { return tanf(RadVal); }

	[[nodiscard]] static FORCEINLINE double Acos(const double Value) { return acos(Value); }
	[[nodiscard]] static FORCEINLINE float Acos(const float Value) { return acosf(Value); }

	[[nodiscard]] static FORCEINLINE double Asin(const double Value) { return asin(Value); }
	[[nodiscard]] static FORCEINLINE float Asin(const float Value) { return asinf(Value); }

	[[nodiscard]] static FORCEINLINE double Atan(const double Value) { return atan(Value); }
	[[nodiscard]] static FORCEINLINE float Atan(const float Value) { return atanf(Value); }

	[[nodiscard]] static FORCEINLINE double Atan2(const double Y, const double X) { return atan2(Y, X); }
	[[nodiscard]] static FORCEINLINE float Atan2(const float Y, const float X) { return atan2f(Y, X); }

	static FORCEINLINE void SinCos(float* ScalarSin, float* ScalarCos, const float Value)
	{
		*ScalarSin = sinf(Value);
		*ScalarCos = cosf(Value);
	}

	static FORCEINLINE void SinCos(double* ScalarSin, double* ScalarCos, const double Value)
	{
		*ScalarSin = sin(Value);
		*ScalarCos = cos(Value);
	}

    template <typename T>
	[[nodiscard]] static FORCEINLINE T Square(T Value) { return Value * Value; }


	[[nodiscard]] static FORCEINLINE int32 CeilToInt(const float Value) { return static_cast<int32>(ceilf(Value)); }
	[[nodiscard]] static FORCEINLINE int32 CeilToInt(const double Value) { return static_cast<int32>(ceil(Value)); }

    template <typename T>
    [[nodiscard]] static FORCEINLINE int32 CeilToInt(T Value) { return static_cast<int32>(ceil(Value)); }


	[[nodiscard]] static FORCEINLINE float UnwindDegrees(float A)
	{
		while (A > 180.0f)
		{
			A -= 360.0f;
		}
		while (A < -180.0f)
		{
			A += 360.0f;
		}
		return A;
	}

    static FORCEINLINE bool IsNearlyZero(const float Value, const float Tolerance = KINDA_SMALL_NUMBER)
	{
	    return Value > - Tolerance && Value <  Tolerance;
	}

    static bool FORCEINLINE IsNearlyEqual(const float A, const float B, const float Tolerance = KINDA_SMALL_NUMBER)
	{
	    return IsNearlyZero(A - B, Tolerance);
	}

    template<typename T1, typename T2 = T1, typename T3 = T2, typename T4 = T3>
    [[nodiscard]] static auto FInterpTo( T1  Current, T2 Target, T3 DeltaTime, T4 InterpSpeed )
	{
	    static_assert(!std::is_same_v<T1, bool> && !std::is_same_v<T2, bool>, "Boolean types may not be interpolated");
	    using RetType = decltype(T1() * T2() * T3() * T4());
	
	    // If no interp speed, jump to target value
	    if( InterpSpeed <= 0.f )
	    {
	        return static_cast<RetType>(Target);
	    }

	    // Distance to reach
	    const RetType Dist = Target - Current;

	    // If distance is too small, just set the desired location
	    if( FMath::Square(Dist) < KINDA_SMALL_NUMBER)
	    {
	        return static_cast<RetType>(Target);
	    }

	    // Delta Move, Clamp so we do not over shoot.
	    const RetType DeltaMove = Dist * FMath::Clamp<RetType>(DeltaTime * InterpSpeed, 0.f, 1.f);

	    return Current + DeltaMove;				
	}

    /**
     * Returns value based on comparand. The main purpose of this function is to avoid
     * branching based on floating point comparison which can be avoided via compiler
     * intrinsics.
     *
     * Please note that we don't define what happens in the case of NaNs as there might
     * be platform specific differences.
     *
     * @param	Comparand		Comparand the results are based on
     * @param	ValueGEZero		Return value if Comparand >= 0
     * @param	ValueLTZero		Return value if Comparand < 0
     *
     * @return	ValueGEZero if Comparand >= 0, ValueLTZero otherwise
     */
    static constexpr FORCEINLINE float FloatSelect(float Comparand, float ValueGEZero, float ValueLTZero)
    {
        return Comparand >= 0.f ? ValueGEZero : ValueLTZero;
    }


    /** Interpolate vector from Current to Target. Scaled by distance to Target, so it has a strong start speed and ease out. */
    static FVector VInterpTo(const FVector& Current, const FVector& Target, float DeltaTime, float InterpSpeed);

    /** Interpolate quaternion from Current to Target. Scaled by angle to Target, so it has a strong start speed and ease out. */
    static FQuat QInterpTo(const FQuat& Current, const FQuat& Target, float DeltaTime, float InterpSpeed);
    
    template< class T >
    [[nodiscard]] static FORCEINLINE T InterpEaseIn(const T& A, const T& B, const float Alpha, const float Exp)
	{
	    float const ModifiedAlpha = Pow(Alpha, Exp);
	    return Lerp<T>(A, B, ModifiedAlpha);
	}

    template< class T >
    [[nodiscard]] static FORCEINLINE T InterpEaseOut(const T& A, const T& B, const float Alpha, const float Exp)
	{
	    float const ModifiedAlpha = 1.f - Pow(1.f - Alpha, Exp);
	    return Lerp<T>(A, B, ModifiedAlpha);
	}

    template< class T > 
    [[nodiscard]] static FORCEINLINE T InterpEaseInOut( const T& A, const T& B, const float Alpha, const float Exp )
	{
	    return Lerp<T>(A, B, (Alpha < 0.5f) ?
            InterpEaseIn(0.f, 1.f, Alpha * 2.f, Exp) * 0.5f :
            InterpEaseOut(0.f, 1.f, Alpha * 2.f - 1.f, Exp) * 0.5f + 0.5f);
	}
};
