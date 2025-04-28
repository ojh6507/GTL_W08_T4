#include "AnimPlayerActor.h"

#include "Components/BoxComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/StaticMeshComponent.h"

#include "Engine/FLoaderOBJ.h"

#include "GameFramework/Actor.h"

AAnimPlayerActor::AAnimPlayerActor()
{
    // Begin Test
    FString MeshName = "Contents/Jingu_1.obj";
    FString MeshName1 = "Contents/Jingu_2.obj";
    MeshAsset1 = FManagerOBJ::GetStaticMesh(MeshName.ToWideString());
    MeshAsset2 = FManagerOBJ::GetStaticMesh(MeshName1.ToWideString());

    StaticMeshComponent->SetStaticMesh(MeshAsset1);
    StaticMeshComponent1 = AddComponent<UStaticMeshComponent>("StaticMeshComponent_1");

    StaticMeshComponent1->SetStaticMesh(MeshAsset2);
    StaticMeshComponent1->SetVisibility(true);
    StaticMeshComponent->SetVisibility(false);

    BoxComponent = AddComponent<UBoxComponent>(TEXT("BoxComponent"));
    BoxComponent->SetupAttachment(RootComponent);
    //CapsuleComponent = AddComponent<UCapsuleComponent>(TEXT("CapsuleComponent"));
    //CapsuleComponent->SetupAttachment(RootComponent);
    // CapsuleComponent->SetRelativeLocation(FVector(5, 0, 0));

    // End Test
}

void AAnimPlayerActor::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);

    AccumulatedTime += DeltaTime;
    //if (AccumulatedTime >= ToggleInterval)
    //{
    //    AccumulatedTime -= ToggleInterval;  // 또는 = 0.0f;
    //    bIsMesh1Active = true;
    //}
    //else
    //    bIsMesh1Active = false;
}

void AAnimPlayerActor::ToggleStaticMesh(UStaticMeshComponent* staticmeshcomp)
{
    staticmeshcomp->SetVisibility(!staticmeshcomp->IsVisibility());
}
