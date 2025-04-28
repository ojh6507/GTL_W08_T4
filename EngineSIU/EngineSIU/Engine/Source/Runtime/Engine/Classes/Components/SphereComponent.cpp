#include "SphereComponent.h"

#include "BoxComponent.h"
#include "CapsuleComponent.h"
#include "Engine/Physics/CollisionDispatcher.h"
#include "GameFramework/Actor.h"
#include "Runtime/Core/Math/JungleMath.h"
#include "UnrealEd/EditorViewportClient.h"
#include "UObject/Casts.h"

USphereComponent::USphereComponent()
{
    SetType(StaticClass()->GetName());
    // Test용 하드 코딩
    SphereRadius = 1;

    // TODO : Test용 하드 코딩 나중에 바꾸기
    const FVector LocalMin = -FVector(SphereRadius);  // (-X, -Y, -Z)
    const FVector LocalMax =  FVector(SphereRadius);  // ( +X, +Y, +Z)
    AABB.max = LocalMax;
    AABB.min = LocalMin;
}

USphereComponent::~USphereComponent()
{
}

void USphereComponent::UninitializeComponent()
{
    Super::UninitializeComponent();
}

void USphereComponent::BeginPlay()
{
    Super::BeginPlay();
    OnComponentBeginOverlap.AddDynamic(this, &USphereComponent::HandleBeginOverlap);
    OnComponentEndOverlap.AddDynamic(this, &USphereComponent::HandleEndOverlap);
}

void USphereComponent::OnComponentDestroyed()
{
    Super::OnComponentDestroyed();
}

void USphereComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    Super::EndPlay(EndPlayReason);
}

void USphereComponent::DestroyComponent()
{
    Super::DestroyComponent();
}

UObject* USphereComponent::Duplicate(UObject* InOuter)
{
    ThisClass* NewComponent = Cast<ThisClass>(Super::Duplicate(InOuter));
    SphereRadius = NewComponent->SphereRadius;
    
    return NewComponent;
}

int USphereComponent::CheckRayIntersection(FVector& rayOrigin, FVector& rayDirection, float& pfNearHitDistance)
{
    return Super::CheckRayIntersection(rayOrigin, rayDirection, pfNearHitDistance);
}

void USphereComponent::GetProperties(TMap<FString, FString>& OutProperties) const
{
    Super::GetProperties(OutProperties);
    OutProperties.Add(TEXT("SphereRadius"), FString::Printf(TEXT("%f"), SphereRadius));
}

void USphereComponent::SetProperties(const TMap<FString, FString>& InProperties)
{
    Super::SetProperties(InProperties);
    const FString* TempStr = nullptr;
    TempStr = InProperties.Find(TEXT("SphereRadius"));
    if (TempStr)
    {
        SphereRadius = FString::ToFloat(*TempStr);
    }
}

bool USphereComponent::CheckOverlapComponent(UShapeComponent* Other, FHitResult& OutHitResult)
{
    if (UBoxComponent* OtherBox = Cast<UBoxComponent>(Other))
    {
        return FCollisionDispatcher::OverlapSphereToBox(this, OtherBox, OutHitResult);
    }

    if (USphereComponent* OtherSphere = Cast<USphereComponent>(Other))
    {
        return FCollisionDispatcher::OverlapSphereToSphere(this, OtherSphere, OutHitResult);
    }

    if (UCapsuleComponent* OtherCapsule = Cast<UCapsuleComponent>(Other))
    {
        return FCollisionDispatcher::OverlapSphereToCapsule(this, OtherCapsule, OutHitResult);
    }

    return false;
}

void USphereComponent::InitializeComponent()
{
    Super::InitializeComponent();
}

void USphereComponent::TickComponent(float DeltaTime)
{
    Super::TickComponent(DeltaTime);
}

void USphereComponent::HandleBeginOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp, bool bFromSweep, const FHitResult& SweepResult)
{
    UE_LOG(ELogLevel::Warning, TEXT("[Overlap] %s at %s ↔ %s at %s"), *OverlappedComp->GetOwner()->GetName(),  *OverlappedComp->GetName(), *OtherActor->GetName(), *OtherComp->GetName());
}

void USphereComponent::HandleEndOverlap(UPrimitiveComponent* OverlappedComp, AActor* OtherActor, UPrimitiveComponent* OtherComp)
{
    UE_LOG(ELogLevel::Warning, TEXT("[OverlapEnd] %s at %s ↔ %s at %s"), *OverlappedComp->GetOwner()->GetName(), *OverlappedComp->GetName(), *OtherActor->GetName(), *OtherComp->GetName());
}
