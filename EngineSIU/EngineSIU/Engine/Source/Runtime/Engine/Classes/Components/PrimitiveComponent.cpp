#include "PrimitiveComponent.h"

#include "Engine/Physics/OverlapInfo.h"
#include "GameFramework/Actor.h"
#include "UObject/Casts.h"
#include "World/World.h"

static bool CanComponentsGenerateOverlap(const UPrimitiveComponent* MyComponent, UPrimitiveComponent* OtherComp)
{
    return OtherComp
    && OtherComp->GetGenerateOverlapEvents()
    && MyComponent
    && MyComponent->GetGenerateOverlapEvents();
}

struct FPredicateFilterCanOverlap
{
    FPredicateFilterCanOverlap(const UPrimitiveComponent& OwningComponent)
        : MyComponent(OwningComponent)
    {
    }

    bool operator() (const FOverlapInfo& Info) const
    {
        return CanComponentsGenerateOverlap(&MyComponent, Info.OverlapInfo.GetComponent());
    }

private:
    const UPrimitiveComponent& MyComponent;
};

// Predicate to identify components from overlaps array that can no longer overlap
struct FPredicateFilterCannotOverlap
{
    FPredicateFilterCannotOverlap(const UPrimitiveComponent& OwningComponent)
    : MyComponent(OwningComponent)
    {
    }

    bool operator() (const FOverlapInfo& Info) const
    {
        return !CanComponentsGenerateOverlap(&MyComponent, Info.OverlapInfo.GetComponent());
    }

private:
    const UPrimitiveComponent& MyComponent;
};

UObject* UPrimitiveComponent::Duplicate(UObject* InOuter)
{
    ThisClass* NewComponent = Cast<ThisClass>(Super::Duplicate(InOuter));

    NewComponent->AABB = AABB;

    return NewComponent;
}

void UPrimitiveComponent::InitializeComponent()
{
	Super::InitializeComponent();
}

void UPrimitiveComponent::TickComponent(float DeltaTime)
{
	Super::TickComponent(DeltaTime);
}

int UPrimitiveComponent::CheckRayIntersection(FVector& rayOrigin, FVector& rayDirection, float& pfNearHitDistance)
{
    //if (!AABB.Intersect(rayOrigin, rayDirection, pfNearHitDistance)) return 0;
    int nIntersections = 0;
    //if (staticMesh == nullptr) return 0;
    //FVertexSimple* vertices = staticMesh->vertices.get();
    //int vCount = staticMesh->numVertices;
    //UINT* indices = staticMesh->indices.get();
    //int iCount = staticMesh->numIndices;

    //if (!vertices) return 0;
    //BYTE* pbPositions = reinterpret_cast<BYTE*>(staticMesh->vertices.get());
    //
    //int nPrimitives = (!indices) ? (vCount / 3) : (iCount / 3);
    //float fNearHitDistance = FLT_MAX;
    //for (int i = 0; i < nPrimitives; i++) {
    //    int idx0, idx1, idx2;
    //    if (!indices) {
    //        idx0 = i * 3;
    //        idx1 = i * 3 + 1;
    //        idx2 = i * 3 + 2;
    //    }
    //    else {
    //        idx0 = indices[i * 3];
    //        idx2 = indices[i * 3 + 1];
    //        idx1 = indices[i * 3 + 2];
    //    }

    //    // 각 삼각형의 버텍스 위치를 FVector로 불러옵니다.
    //    uint32 stride = sizeof(FVertexSimple);
    //    FVector v0 = *reinterpret_cast<FVector*>(pbPositions + idx0 * stride);
    //    FVector v1 = *reinterpret_cast<FVector*>(pbPositions + idx1 * stride);
    //    FVector v2 = *reinterpret_cast<FVector*>(pbPositions + idx2 * stride);

    //    float fHitDistance;
    //    if (IntersectRayTriangle(rayOrigin, rayDirection, v0, v1, v2, fHitDistance)) {
    //        if (fHitDistance < fNearHitDistance) {
    //            pfNearHitDistance =  fNearHitDistance = fHitDistance;
    //        }
    //        nIntersections++;
    //    }
    //   
    //}
    return nIntersections;
}

bool UPrimitiveComponent::IntersectRayTriangle(const FVector& rayOrigin, const FVector& rayDirection, const FVector& v0, const FVector& v1, const FVector& v2, float& hitDistance) const
{
    constexpr float epsilon = 1e-6f;
    FVector edge1 = v1 - v0;
    const FVector edge2 = v2 - v0;
    FVector FrayDirection = rayDirection;
    FVector h = FrayDirection.Cross(edge2);
    float a = edge1.Dot(h);

    if (fabs(a) < epsilon)
        return false; // Ray와 삼각형이 평행한 경우

    float f = 1.0f / a;
    FVector s = rayOrigin - v0;
    float u = f * s.Dot(h);
    if (u < 0.0f || u > 1.0f)
        return false;

    FVector q = s.Cross(edge1);
    float v = f * FrayDirection.Dot(q);
    if (v < 0.0f || (u + v) > 1.0f)
        return false;

    float t = f * edge2.Dot(q);
    if (t > epsilon) {

        hitDistance = t;
        return true;
    }

    return false;
}

void UPrimitiveComponent::GetProperties(TMap<FString, FString>& OutProperties) const
{
    Super::GetProperties(OutProperties);
    OutProperties.Add(TEXT("m_Type"), m_Type);
    OutProperties.Add(TEXT("AABB_min"), AABB.min.ToString());
    OutProperties.Add(TEXT("AABB_max"), AABB.max.ToString());
}


void UPrimitiveComponent::SetProperties(const TMap<FString, FString>& InProperties)
{
    Super::SetProperties(InProperties);

    const FString* TempStr = nullptr;

    // --- PrimitiveComponent 고유 속성 복원 ---

    TempStr = InProperties.Find(TEXT("m_Type"));
    if (TempStr)
    {
        this->m_Type = *TempStr; // m_Type이 FString이라고 가정
        // 만약 m_Type이 enum이라면 문자열로부터 enum 값을 파싱하는 로직 필요
    }

    const FString* AABBminStr = InProperties.Find(TEXT("AABB_min"));
    if (AABBminStr) AABB.min.InitFromString(*AABBminStr); 

    
    const FString* AABBmaxStr = InProperties.Find(TEXT("AABB_max"));
    if (AABBmaxStr) AABB.max.InitFromString(*AABBmaxStr); 
}

static bool AreActorsOverlapping(const AActor& A, const AActor& B)
{
    // Due to the implementation of IsOverlappingActor() that scans and queries all owned primitive components and their overlaps,
    // we guess that it's cheaper to scan the shorter of the lists.
    if (A.GetComponents().Num() <= B.GetComponents().Num())
    {
        return A.IsOverlappingActor(&B);
    }
    else
    {
        return B.IsOverlappingActor(&A);
    }
}

void UPrimitiveComponent::BeginComponentOverlap(const FOverlapInfo& OtherOverlap, bool bDoNotifies)
{
    UPrimitiveComponent* OtherComp = OtherOverlap.OverlapInfo.HitComponent;
    if (CanComponentsGenerateOverlap(this, OtherComp))
    {
        AActor* const OtherActor = OtherComp->GetOwner();
        AActor* const MyActor = GetOwner();

        const bool bSameActor = (MyActor == OtherActor);
        const bool bNotifyActorTouch = bDoNotifies && !bSameActor && !AreActorsOverlapping(*MyActor, *OtherActor);

        OverlappingComponents.Add(OtherOverlap);
        OtherComp->OverlappingComponents.Add(FOverlapInfo(this));

        const UWorld* World = GetWorld();
        //if ((World && World->WorldType == EWorldType::PIE))
        // TODO : 일단 테스트 용
        if (World)
        {
            if (this != nullptr)
            {
                OnComponentBeginOverlap.Broadcast(this, OtherActor, OtherComp, OtherOverlap.bFromSweep, OtherOverlap.OverlapInfo);
            }
            if (OtherComp != nullptr)
            {
                OtherComp->OnComponentBeginOverlap.Broadcast(OtherComp, MyActor, this, OtherOverlap.bFromSweep, OtherOverlap.bFromSweep ? FHitResult::GetReversedHit(OtherOverlap.OverlapInfo) : OtherOverlap.OverlapInfo);
            }

            if (bNotifyActorTouch)
            {
                // TODO : Actor
                if (MyActor != nullptr)
                {
                    MyActor->OnActorBeginOverlap.Broadcast(MyActor, OtherActor);
                }
                
                if (OtherActor != nullptr)
                {
                    OtherActor->OnActorBeginOverlap.Broadcast(OtherActor, MyActor);
                }
            }
        }        
    }
}

void UPrimitiveComponent::EndComponentOverlap(const FOverlapInfo& OtherOverlap, bool bDoNotifies, bool bSkipNotifySelf)
{
    UPrimitiveComponent* OtherComp = OtherOverlap.OverlapInfo.HitComponent;
    if (OtherComp == nullptr)
    {
        return;
    }

    const int32 OtherOverlapIdx = OtherComp->OverlappingComponents.IndexOfByPredicate(FFastOverlapInfoCompare(FOverlapInfo(this)));
    if (OtherOverlapIdx != INDEX_NONE)
    {
        OtherComp->OverlappingComponents.RemoveAtSwap(OtherOverlapIdx);
    }

    int32 OverlapIdx = OverlappingComponents.IndexOfByPredicate(FFastOverlapInfoCompare(OtherOverlap));
    if (OverlapIdx != INDEX_NONE)
    {
        OverlappingComponents.RemoveAtSwap(OverlapIdx);

        AActor* const MyActor = GetOwner();
        const UWorld* World = GetWorld();
        //if ((World && World->WorldType == EWorldType::PIE))
        // TODO : 일단 테스트 용
        if (World)
        {
            AActor* const OtherActor = OtherComp->GetOwner();
            if (OtherActor != nullptr)
            {
                OnComponentEndOverlap.Broadcast(this, OtherActor, OtherComp);
            }

            if (OtherComp != nullptr)
            {
                OtherComp->OnComponentEndOverlap.Broadcast(OtherComp, MyActor, this);
            }

            const bool bSameActor = (MyActor == OtherActor);

            if (MyActor != nullptr && !bSameActor && !AreActorsOverlapping(*MyActor, *OtherActor))
            {
                if (MyActor != nullptr)
                {
                    MyActor->OnActorEndOverlap.Broadcast(MyActor, OtherActor);
                }

                if (OtherActor != nullptr)
                {
                    OtherActor->OnActorEndOverlap.Broadcast(OtherActor, MyActor);
                }
            }
        }
    }
}

bool UPrimitiveComponent::IsOverlappingComponent(const UPrimitiveComponent* OtherComp) const
{
    for (int32 i=0; i < OverlappingComponents.Num(); ++i)
    {
        if (OverlappingComponents[i].OverlapInfo.HitComponent == OtherComp)
        {
            return true;
        }
    }
    return false;
}

bool UPrimitiveComponent::IsOverlappingComponent(const FOverlapInfo& Overlap)
{
    return OverlappingComponents.Find(Overlap) != INDEX_NONE;
}

bool UPrimitiveComponent::IsOverlappingActor(const AActor* Other) const
{
    if (Other)
    {
        for (int32 OverlapIdx=0; OverlapIdx<OverlappingComponents.Num(); ++OverlapIdx)
        {
            UPrimitiveComponent const* const PrimComp = OverlappingComponents[OverlapIdx].OverlapInfo.HitComponent;
            if (PrimComp && (PrimComp->GetOwner() == Other) )
            {
                return true;
            }
        }
    }

    return false;
}

bool UPrimitiveComponent::GetOverlapsWithActor(const AActor* Actor, TArray<FOverlapInfo>& OutOverlaps) const
{
    const int32 InitialCount = OutOverlaps.Num();
    if (Actor)
    {
        for (int32 OverlapIdx = 0; OverlapIdx < OverlappingComponents.Num(); ++OverlapIdx)
        {
            UPrimitiveComponent const* const PrimComp = OverlappingComponents[OverlapIdx].OverlapInfo.HitComponent;
            if (PrimComp && (PrimComp->GetOwner() == Actor))
            {
                OutOverlaps.Add(OverlappingComponents[OverlapIdx]);
            }
        }
    }

    return InitialCount != OutOverlaps.Num();
}

void UPrimitiveComponent::GetOverlappingActors(TArray<AActor*>& OutOverlappingActors, UClass* InClass) const
{
    if (OverlappingComponents.Num() <= 12)
    {
        // TArray with fewer elements is faster than using a set (and having to allocate it).
        OutOverlappingActors.SetNum(OverlappingComponents.Num());
        for (const FOverlapInfo& OtherOverlap : OverlappingComponents)
        {
            if (UPrimitiveComponent* OtherComponent = OtherOverlap.OverlapInfo.HitComponent)
            {
                AActor* OtherActor = OtherComponent->GetOwner();
                if (OtherActor && ((InClass) == nullptr || OtherActor->IsA(InClass)))
                {
                    OutOverlappingActors.AddUnique(OtherActor);
                }
            }
        }
    }
    else
    {
        // Fill set (unique)
        TSet<AActor*> OverlapSet;
        GetOverlappingActors(OverlapSet, InClass);

        // Copy to array
        OutOverlappingActors.SetNum(OverlapSet.Num());
        for (AActor* OverlappingActor : OverlapSet)
        {
            OutOverlappingActors.Add(OverlappingActor);
        }
    }
}

void UPrimitiveComponent::GetOverlappingActors(TSet<AActor*>& OutOverlappingActors, UClass* InClass) const
{
    OutOverlappingActors.Empty();
    OutOverlappingActors.Reserve(OverlappingComponents.Num());

    for (const FOverlapInfo& OtherOverlap : OverlappingComponents)
    {
        if (UPrimitiveComponent* OtherComponent = OtherOverlap.OverlapInfo.HitComponent)
        {
            AActor* OtherActor = OtherComponent->GetOwner();
            if (OtherActor && OtherActor->IsA(InClass))
            {
                OutOverlappingActors.Add(OtherActor);
            }
        }
    }
}

void UPrimitiveComponent::GetOverlappingComponents(TArray<UPrimitiveComponent*>& OutOverlappingComponents) const
{
    if (OverlappingComponents.Num() <= 12)
    {
        // TArray with fewer elements is faster than using a set (and having to allocate it).
        OutOverlappingComponents.SetNum(OverlappingComponents.Num());
        for (const FOverlapInfo& OtherOverlap : OverlappingComponents)
        {
            UPrimitiveComponent* const OtherComp = OtherOverlap.OverlapInfo.HitComponent;
            if (OtherComp)
            {
                OutOverlappingComponents.AddUnique(OtherComp);
            }
        }
    }
    else
    {
        // Fill set (unique)
        TSet<UPrimitiveComponent*> OverlapSet;
        GetOverlappingComponents(OverlapSet);
		
        // Copy to array
        OutOverlappingComponents.SetNum(OverlapSet.Num());
        for (UPrimitiveComponent* OtherOverlap : OverlapSet)
        {
            OutOverlappingComponents.Add(OtherOverlap);
        }
    }
}

const TArray<FOverlapInfo>& UPrimitiveComponent::GetOverlapInfos() const
{
    return OverlappingComponents;
}

void UPrimitiveComponent::GetOverlappingComponents(TSet<UPrimitiveComponent*>& OutOverlappingComponents) const
{
    OutOverlappingComponents.Empty();
    OutOverlappingComponents.Reserve(OverlappingComponents.Num());

    for (const FOverlapInfo& OtherOverlap : OverlappingComponents)
    {
        UPrimitiveComponent* const OtherComp = OtherOverlap.OverlapInfo.HitComponent;
        if (OtherComp)
        {
            OutOverlappingComponents.Add(OtherComp);
        }
    }
}
