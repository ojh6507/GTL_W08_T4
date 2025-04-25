#pragma once
#include "Components/SceneComponent.h"

class UPrimitiveComponent : public USceneComponent
{
    DECLARE_CLASS(UPrimitiveComponent, USceneComponent)

public:
    UPrimitiveComponent() = default;

    virtual UObject* Duplicate(UObject* InOuter) override;

    virtual void InitializeComponent() override;
    virtual void TickComponent(float DeltaTime) override;
    virtual int CheckRayIntersection(FVector& rayOrigin, FVector& rayDirection, float& pfNearHitDistance) override;
    bool IntersectRayTriangle(
        const FVector& rayOrigin, const FVector& rayDirection,
        const FVector& v0, const FVector& v1, const FVector& v2, float& hitDistance
    ) const;

    
    void GetProperties(TMap<FString, FString>& OutProperties) const override;
    void SetProperties(const TMap<FString, FString>& InProperties) override;


    FBoundingBox AABB;

private:
    FString m_Type;

public:
    FString GetType() { return m_Type; }

    void SetType(const FString& _Type)
    {
        m_Type = _Type;
        //staticMesh = FEngineLoop::resourceMgr.GetMesh(m_Type);
    }
    FBoundingBox GetBoundingBox() const { return AABB; }

    bool GetGeneratedOverlapEvents() const { return bGenerateOverlapEvents; }
    void SetGeneratedOverlapEvents(const bool bInGenerateOverlapEvents) { bGenerateOverlapEvents = bInGenerateOverlapEvents; }

private:
    //true이라면, 이 Component는 다른 Component(예: Begin Overlap)와 Overlap 때 Overlap 이벤트를 생성합니다.
    //Overlap 이벤트가 발생하려면 두 Component 모두 이 것을 true로 해야합니다.
    bool bGenerateOverlapEvents;
    bool bBlockComponent; // ComponentHit
    
public:
    // TODO : Primitvie Component가 그림자를 드리워야 하는지 여부를 제어합니다.
    bool CastShadow;
};

