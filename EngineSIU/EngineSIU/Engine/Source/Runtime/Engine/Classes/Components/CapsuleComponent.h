#pragma once
#include "UShapeComponent.h"

class UCapsuleComponent : public UShapeComponent
{
    DECLARE_CLASS(UCapsuleComponent, UShapeComponent)
public:
    UCapsuleComponent();
    ~UCapsuleComponent() override;
    void Serialize(FArchive& Ar) override;
    void UninitializeComponent() override;
    void InitializeComponent() override;
    void BeginPlay() override;
    void TickComponent(float DeltaTime) override;
    void OnComponentDestroyed() override;
    void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
    void DestroyComponent() override;
    UObject* Duplicate(UObject* InOuter) override;
    void GetProperties(TMap<FString, FString>& OutProperties) const override;
    void SetProperties(const TMap<FString, FString>& InProperties) override;
    bool CheckOverlapComponent(UShapeComponent* Other, FHitResult& OutHitResult) override; 
};
