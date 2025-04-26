#pragma once
#include "HitResult.h"

class UCapsuleComponent;
class USphereComponent;
class UBoxComponent;

class FCollisionDispatcher
{
public:
    // LHS가 체크할 Box, RHS가 체크 당할 Box
    static bool OverlapBoxToBox(const UBoxComponent* LHSBox, UBoxComponent* RHSBox, FHitResult& OutHitResult);
    static bool OverlapBoxToSphere(const UBoxComponent* LHSBox, USphereComponent* RHSSphere, FHitResult& OutHitResult);
    static bool OverlapBoxToCapsule(const UBoxComponent* LHSBox, UCapsuleComponent* RHSCapsule, FHitResult& OutHitResult);

    static bool OverlapSphereToBox(const USphereComponent* LHSSphere, UBoxComponent* RHSBox, FHitResult& OutHitResult);
    static bool OverlapSphereToSphere(const USphereComponent* LHSSphere, USphereComponent* RHSSphere, FHitResult& OutHitResult);
    static bool OverlapSphereToCapsule(const USphereComponent* LHSSphere, UCapsuleComponent* RHSCapsule, FHitResult& OutHitResult);

    static bool OverlapCapsuleToBox(const UCapsuleComponent* LHSCapsule, UBoxComponent* RHSBBox, FHitResult& OutHitResult);
    static bool OverlapCapsuleToSphere(UCapsuleComponent* LHSCapsule, USphereComponent* RHSSphere, FHitResult& OutHitResult);
    static bool OverlapCapsuleToCapsule(UCapsuleComponent* LHSCapsule,  UCapsuleComponent* RHSCapsule, FHitResult& OutHitResult);
};
