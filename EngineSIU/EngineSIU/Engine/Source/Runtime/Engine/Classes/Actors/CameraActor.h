#pragma once
#include "Components/StaticMeshComponent.h"
#include "GameFramework/Actor.h"

class UCameraComponent;

class ACameraActor : public AActor
{
    DECLARE_CLASS(ACameraActor, AActor)
public:
    ACameraActor();
    ~ACameraActor() override;

    UObject* Duplicate(UObject* InOuter) override;
    void BeginPlay() override;
    void Tick(float DeltaTime) override;
    void Destroyed() override;
    void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
    bool Destroy() override;

    FMatrix GetViewMatrix() const;
    FMatrix GetProjectionMatrix() const;

    float GetCameraNearClip() const;
    float GetCameraFarClip() const;

    TArray<FVector> GetFrustumCorners() const;
    class UCameraComponent* GetCameraComponent() const { return CameraComponent; }

private:
    UCameraComponent* CameraComponent;
    UStaticMeshComponent* StaticMeshComponent;
};
