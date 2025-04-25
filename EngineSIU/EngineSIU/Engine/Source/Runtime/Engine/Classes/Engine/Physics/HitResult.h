#pragma once
#include "Container/Array.h"
#include "Math/Vector.h"

class UPrimitiveComponent;
class AActor;

struct FHitResult
{
    /**
     * Time
     * 트레이스 방향을 따라 충돌이 발생한 시점을
     * TraceStart → TraceEnd 구간(0.0 ~ 1.0)으로 나타낸 값입니다.
     * 스윕 이동(쿼리 제외)일 때는 인접한 지오메트리 정밀도 문제를
     * 방지하기 위해 실제 시점보다 약간 당겨질 수 있습니다.
     */
    float Time;
    
    /**
     * Distance
     * TraceStart에서 Location까지의 월드 공간 거리입니다.
     * 초기 오버랩(트레이스 시작 시 이미 다른 콜라이더 내부)인 경우 0이 됩니다.
     */
    float Distance;
    
    // 출돌 시 충돌 물체의 중심 좌표
    FVector Location;
    // 충돌 지점 월드 좌표
    FVector ImpactPoint;    

    // 세계 공간에서 Sweep된 물체에 대한 타격의 법선 벡터
    FVector Normal;
    // Sweep에 맞은 대상의 표면 법선 벡터
    FVector ImpactNormal;    

    //트레이스(또는 스윕)를 시작한 지점의 월드 좌표입니다.
    //예를 들어 구체 스윕 시에는 “스피어 중심”이 출발한 위치를 의미
    FVector TraceStart;
    //트레이스가 도달하려고 시도한 최종 지점의 월드 좌표
    //충돌이 발생하지 않았다면 이 지점까지 진행하며, 실제 임팩트 발생 지점과는 다릅니다
    FVector TraceEnd;

    //초기 트레이스가 다른 콜라이더와 이미 겹쳐진 상태(bStartPenetrating == true)에서,
    //그 겹침을 해소하기 위해 충돌 면 법선(Normal) 방향으로 얼마나 이동해야 하는지를 나타내는 거리
    float PenetrationDepth;

    // 충돌 발생 여부
    bool bBlockingHit = false;

    //트레이스가 초기부터 침투 상태(즉, 다른 콜라이더와 블로킹 오버랩이 이미 발생한 상태)로 시작했는지를 나타냅니다.
    bool bStartPenetrating = false;
    
    AActor* HitActor       = nullptr;  // 충돌된 액터
    UPrimitiveComponent* HitComponent   = nullptr;  // 충돌된 컴포넌트

    FHitResult()
    {
        Init();
    }

    explicit FHitResult(float InTime)
    {
        Init();
        Time = InTime;
    }

    explicit FHitResult(const FVector Start, const FVector End)
    {
        Init(Start, End);
    }

    /** Initialize empty hit result with given time. */
    FORCEINLINE void Init()
    {
        Time = 1.f;
    }

    FORCEINLINE void Init(const FVector Start, const FVector End)
    {
        Time = 1.f;
        TraceStart = Start;
        TraceEnd = End;
    }

    FHitResult(AActor* InActor, UPrimitiveComponent* InComponent, FVector const& HitLoc, FVector const& HitNorm);

    FORCEINLINE AActor* GetActor() const
    {
        return HitActor;
    }

    FORCEINLINE UPrimitiveComponent* GetComponent() const
    {
        return HitComponent;
    }

    static FHitResult* GetFirstBlockingHit(TArray<FHitResult>& InHits)
    {
        for(int32 HitIdx = 0; HitIdx < InHits.Num(); HitIdx++)
        {
            if(InHits[HitIdx].bBlockingHit)
            {
                return &InHits[HitIdx];
            }
        }
        return nullptr;
    }

    static int32 GetNumBlockingHits(const TArray<FHitResult>& InHits)
    {
        int32 NumBlocks = 0;
        for(int32 HitIdx = 0; HitIdx < InHits.Num(); HitIdx++)
        {
            if(InHits[HitIdx].bBlockingHit)
            {
                NumBlocks++;
            }
        }
        return NumBlocks;
    }

    static int32 GetNumOverlapHits(const TArray<FHitResult>& InHits)
    {
        return (InHits.Num() - GetNumBlockingHits(InHits));
    }

    static FHitResult GetReversedHit(const FHitResult& Hit)
    {
        FHitResult Result(Hit);
        Result.Normal = -Result.Normal;
        Result.ImpactNormal = -Result.ImpactNormal;
        
        return Result;
    }

    FString ToString() const;
};
