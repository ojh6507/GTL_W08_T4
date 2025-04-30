#pragma once
#include "PlayerCameraManager.h"
#include "Components/SceneComponent.h"
#include "LevelEditor/SLevelEditor.h"
#include "Math/JungleMath.h"
#include "UnrealEd/EditorViewportClient.h"

class UCameraComponent : public USceneComponent
{
    DECLARE_CLASS(UCameraComponent, USceneComponent)

public:
    UCameraComponent() = default;
    ~UCameraComponent() override;

    void UninitializeComponent() override;
    void BeginPlay() override;
    void OnComponentDestroyed() override;
    
    void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
    UObject* Duplicate(UObject* InOuter) override;
    void GetProperties(TMap<FString, FString>& OutProperties) const override;
    void SetProperties(const TMap<FString, FString>& InProperties) override;
    
    void InitializeComponent() override;
    void TickComponent(float DeltaTime) override;
    void DestroyComponent() override;

    float GetViewFovRadians() const { return FMath::DegreesToRadians(ViewFOV); }
    void SetViewFovRadians(const float InViewFOV) { ViewFOV = FMath::RadiansToDegrees(InViewFOV) ; }
    
    float GetVeiwFovDegrees() const { return ViewFOV; }
    void SetViewFovDegrees(const float InViewFOV) { ViewFOV = InViewFOV; }

    float GetNearClip() const { return NearClip; }
    void SetNearClip(const float InNearClip) { NearClip = InNearClip; }
    
    float GetFarClip() const { return FarClip; }
    void SetFarClip(const float InFarClip) { FarClip = InFarClip; }

    FMatrix GetViewMatrix() const  { return View; }
    FMatrix GetProjectionMatrix() const { return Projection; }

private:
    void UpdateViewMatrix();
    void UpdateProjectionMatrix();

private:
    // 카메라 정보 
    float ViewFOV = 90.0f;
    float NearClip = 0.1f;
    float FarClip = 1000.0f;

    FMatrix View = FMatrix::Identity;
    FMatrix Projection = FMatrix::Identity;
};
