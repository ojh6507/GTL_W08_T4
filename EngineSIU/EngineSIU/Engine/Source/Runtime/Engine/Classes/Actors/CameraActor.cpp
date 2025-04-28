#include "CameraActor.h"

#include <sol/sol.hpp>

#include "Camera/CameraComponent.h"
#include "Engine/FLoaderOBJ.h"

ACameraActor::ACameraActor()
{
    StaticMeshComponent = AddComponent<UStaticMeshComponent>(TEXT("StaticMeshComponent"));
    StaticMeshComponent->SetStaticMesh(FManagerOBJ::GetStaticMesh(L"Contents/Camera.obj"));
    CameraComponent = AddComponent<UCameraComponent>(TEXT("CameraComponent"));
}

ACameraActor::~ACameraActor()
{
}

UObject* ACameraActor::Duplicate(UObject* InOuter)
{
    ThisClass* NewCameraActor = Cast<ThisClass>(Super::Duplicate(InOuter));
    for (UActorComponent* component : NewCameraActor->GetComponents())
    {
        UCameraComponent* cameraComponent = Cast<UCameraComponent>(component);
        UStaticMeshComponent* staticMeshComponent = Cast<UStaticMeshComponent>(component);
        if (cameraComponent)
        {
            NewCameraActor->CameraComponent->DestroyComponent();
            NewCameraActor->CameraComponent = cameraComponent;
        }
        else if (staticMeshComponent)
        {
            NewCameraActor->StaticMeshComponent->DestroyComponent();
            NewCameraActor->StaticMeshComponent = staticMeshComponent;
        }
    }

    
    return NewCameraActor;
}

void ACameraActor::BeginPlay()
{
    Super::BeginPlay();
    std::shared_ptr<FEditorViewportClient> ActiveViewporClient = GEngineLoop.GetLevelEditor()->GetActiveViewportClient();

    if (ActiveViewporClient)
    {
        ActiveViewporClient->SetCameraComponent(CameraComponent);
    }
}

void ACameraActor::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

void ACameraActor::Destroyed()
{
    Super::Destroyed();
}

void ACameraActor::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    Super::EndPlay(EndPlayReason);
    std::shared_ptr<FEditorViewportClient> ActiveViewporClient = GEngineLoop.GetLevelEditor()->GetActiveViewportClient();

    if (ActiveViewporClient)
    {
        ActiveViewporClient->SetCameraComponent(nullptr);
    }
}

bool ACameraActor::Destroy()
{
    return Super::Destroy();
}

FMatrix ACameraActor::GetViewMatrix() const
{
    if (CameraComponent != nullptr)
    {
        return CameraComponent->GetViewMatrix();
    }
    
    return FMatrix::Identity;
}

FMatrix ACameraActor::GetProjectionMatrix() const
{
    if (CameraComponent != nullptr)
    {
        return CameraComponent->GetProjectionMatrix();
    }

    return FMatrix::Identity;
}

float ACameraActor::GetCameraNearClip() const
{
    if (CameraComponent != nullptr)
    {
        return CameraComponent->GetNearClip();
    }
    return 0.1f;
}

float ACameraActor::GetCameraFarClip() const
{
    if (CameraComponent != nullptr)
    {
        return CameraComponent->GetFarClip();
    }

    return 1000.f;
}

TArray<FVector> ACameraActor::GetFrustumCorners() const
{
    TArray<FVector> FrustumCorners;
    FrustumCorners.Empty();
    FrustumCorners.Reserve(8);

    const FMatrix InvViewProj = FMatrix::Inverse(GetViewMatrix()) * FMatrix::Inverse(GetProjectionMatrix());

    // 3) NDC 8코너를 homogeneous 4D로 만든 뒤 변환·분할
    for (int ix = -1; ix <= 1; ix += 2)
    {
        for (int iy = -1; iy <= 1; iy += 2)
        {
            for (int iz = 0; iz <= 1; ++iz)
            {
                float zNDC = iz ? 1.0f : 0.0f;
                FVector ndc(static_cast<float>(ix), static_cast<float>(iy), zNDC);
                FVector world = InvViewProj.TransformPosition(ndc);

                FrustumCorners.Add(world);
            }
        }
    }
    return FrustumCorners;
}
