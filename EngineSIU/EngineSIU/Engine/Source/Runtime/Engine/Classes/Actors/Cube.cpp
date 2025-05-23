#include "Cube.h"

#include "Components/BoxComponent.h"
#include "Components/CapsuleComponent.h"
#include "Components/StaticMeshComponent.h"

#include "Engine/FLoaderOBJ.h"

#include "GameFramework/Actor.h"

ACube::ACube()
{
    // Begin Test
    FString MeshName = "Contents/Dorayaki.obj";
    StaticMeshComponent->SetStaticMesh(FManagerOBJ::GetStaticMesh(MeshName.ToWideString()));
    //StaticMeshComponent->SetStaticMesh(FManagerOBJ::GetStaticMesh(L"Contents/Reference/Reference.obj"));
    BoxComponent = AddComponent<UBoxComponent>(TEXT("BoxComponent"));
    //CapsuleComponent = AddComponent<UCapsuleComponent>(TEXT("CapsuleComponent"));
    //CapsuleComponent->SetupAttachment(RootComponent);
    // CapsuleComponent->SetRelativeLocation(FVector(5, 0, 0));
    
    // End Test
}

void ACube::Tick(float DeltaTime)
{
    Super::Tick(DeltaTime);
}

UObject* ACube::Duplicate(UObject* InOuter)
{
    ThisClass* NewActor = Cast<ThisClass>(Super::Duplicate(InOuter));
    for (UActorComponent* component : NewActor->GetComponents())
    {
        UBoxComponent* boxComponent = Cast<UBoxComponent>(component);
        if (boxComponent)
        {
            NewActor->BoxComponent->DestroyComponent();
            NewActor->BoxComponent = boxComponent;
        }
    }

    return NewActor;
}
