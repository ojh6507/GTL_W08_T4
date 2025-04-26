#pragma once
#include "PrimitiveComponent.h"

class UShapeComponent : public UPrimitiveComponent
{
    DECLARE_CLASS(UShapeComponent, UPrimitiveComponent)

public:
    UShapeComponent();
    ~UShapeComponent() override;
    void UninitializeComponent() override;

    void InitializeComponent() override;
    void BeginPlay() override;

    void TickComponent(float DeltaTime) override;
    
    void OnComponentDestroyed() override;
    
    void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
    
    void DestroyComponent() override;
    
    UObject* Duplicate(UObject* InOuter) override;
    
    int CheckRayIntersection(FVector& rayOrigin, FVector& rayDirection, float& pfNearHitDistance) override;
    
    void GetProperties(TMap<FString, FString>& OutProperties) const override;
    void SetProperties(const TMap<FString, FString>& InProperties) override;
     
    virtual bool CheckOverlapComponent(UShapeComponent* Other, FHitResult& OutHitResult);
    
public:
    FLinearColor ShapeColor;
    bool bDrawOnlyIfSelected;
};
