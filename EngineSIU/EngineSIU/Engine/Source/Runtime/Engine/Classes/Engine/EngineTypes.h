#pragma once
#include "Core/HAL/PlatformType.h"
#include "Core/Math/Vector.h"
#include "Core/Math/Rotator.h"

namespace EEndPlayReason
{
enum Type : uint8
{
    /** 명시적인 삭제가 일어났을 때, Destroy()등 */
    Destroyed,
    /** World가 바뀌었을 때 */
    WorldTransition,
    /** 프로그램을 종료했을 때 */
    Quit,
};
}

struct FPOV
{
    /** Location */
    FVector Location;

    /** Rotation */
    FRotator Rotation;

    /** FOV angle */
    float FOV;

    FPOV() 
    : Location(),Rotation(), FOV(90.0f)
    {}

    FPOV(FVector InLocation, FRotator InRotation, float InFOV)
    : Location(InLocation), Rotation(InRotation), FOV(InFOV) 
    {}
    
};
