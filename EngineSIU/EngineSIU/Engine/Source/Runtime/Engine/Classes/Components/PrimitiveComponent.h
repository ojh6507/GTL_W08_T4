#pragma once
#include "Components/SceneComponent.h"
#include "Delegates/DelegateCombination.h"
#include "Engine/Physics/HitResult.h"
#include "Engine/Physics/OverlapInfo.h"

struct FOverlapInfo;
DECLARE_DYNAMIC_MULTICAST_DELEGATE_FiveParams(FComponentHitSignature, UPrimitiveComponent*, AActor*, UPrimitiveComponent*, FVector,
                                              const FHitResult&);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_FiveParams(FComponentBeginOverlapSignature, UPrimitiveComponent*, AActor*, UPrimitiveComponent*, bool,  const FHitResult&);
DECLARE_DYNAMIC_MULTICAST_DELEGATE_ThreeParams(FComponentEndOverlapSignature, UPrimitiveComponent*, AActor*, UPrimitiveComponent*);


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

    bool GetGenerateOverlapEvents() const { return bGenerateOverlapEvents; }
    void SetGenerateOverlapEvents(const bool bInGenerateOverlapEvents) { bGenerateOverlapEvents = bInGenerateOverlapEvents; }
    bool IsBlockComponent() const { return bBlockComponent; }
    void SetBlockComponent(const bool bInBlockComponent) { bBlockComponent = bInBlockComponent; }

private:
    //true이라면, 이 Component는 다른 Component(예: Begin Overlap)와 Overlap 때 Overlap 이벤트를 생성합니다.
    //Overlap 이벤트가 발생하려면 두 Component 모두 이 것을 true로 해야합니다.
    bool bGenerateOverlapEvents;
    bool bBlockComponent; // ComponentHit
    
public:
    // TODO : Primitvie Component가 그림자를 드리워야 하는지 여부를 제어합니다.
    bool CastShadow;

protected:
    TArray<FOverlapInfo> OverlappingComponents;
public:
    void BeginComponentOverlap(const FOverlapInfo& OtherOverlap, bool bDoNotifies);
    void EndComponentOverlap(const FOverlapInfo& OtherOverlap, bool bDoNotifies=true, bool bSkipNotifySelf=false);
    bool IsOverlappingComponent(const UPrimitiveComponent* OtherComp) const;
    bool IsOverlappingComponent(const FOverlapInfo& Overlap);
    bool IsOverlappingActor(const AActor* Other) const;

    bool GetOverlapsWithActor(const AActor* Actor, TArray<FOverlapInfo>& OutOverlaps) const;

    void GetOverlappingActors(TArray<AActor*>& OutOverlappingActors, UClass* InClass) const;
    void GetOverlappingActors(TSet<AActor*>& OverlappingActors, UClass* InClass) const;
    void GetOverlappingComponents(TArray<UPrimitiveComponent*>& OutOverlappingComponents) const;
    void GetOverlappingComponents(TSet<UPrimitiveComponent*>& OutOverlappingComponents) const;
    const TArray<FOverlapInfo>& GetOverlapInfos() const;

public:
    FComponentHitSignature OnComponentHit;
    FComponentBeginOverlapSignature OnComponentBeginOverlap;
    FComponentEndOverlapSignature OnComponentEndOverlap;
};

