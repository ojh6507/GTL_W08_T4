#pragma once
#include "HitResult.h"
#include "HAL/PlatformType.h"

class UPrimitiveComponent;

struct FOverlapInfo
{
    FOverlapInfo()
    {}

    explicit FOverlapInfo(const FHitResult& InSweepResult)
        : bFromSweep(true), OverlapInfo(InSweepResult)
    {
    }

    explicit FOverlapInfo(UPrimitiveComponent* InComponent);

    int32 GetBodyIndex() const { return 0; }

    // 이 함수는 SweepResult 정보를 완전히 무시합니다. 
    // 이 함수를 사용하는 곳에서는 상관없는 것처럼 보이지만, 여전히 위험할 수 있습니다.
    friend bool operator == (const FOverlapInfo& LHS, const FOverlapInfo& RHS)
    {
        return LHS.OverlapInfo.HitComponent == RHS.OverlapInfo.HitComponent;
    }
    bool bFromSweep;

    /** 스윕(sweep) 및 오버랩(overlap) 쿼리에 대한 정보입니다. 
     * bFromSweep 값에 따라 유효한 멤버가 달라집니다.
     * - bFromSweep가 true이면, 일반적인 스윕 결과처럼 FHitResult의 모든 정보가 완전히 유효합니다.
     * - bFromSweep가 false이면, 실제로는 FOverlapResult와 동일하게 동작하므로 
     *   FHitResult::Component, FHitResult::Actor만 유효합니다.
     */
    FHitResult OverlapInfo;
};

struct FFastOverlapInfoCompare
{
    FFastOverlapInfoCompare(const FOverlapInfo& BaseInfo)
        : MyBaseInfo(BaseInfo)
    {
    }

    bool operator() (const FOverlapInfo& Info)
    {
        return MyBaseInfo.OverlapInfo.HitComponent == Info.OverlapInfo.HitComponent
            && MyBaseInfo.GetBodyIndex() == Info.GetBodyIndex();
    }

    bool operator() (const FOverlapInfo* Info)
    {
        return MyBaseInfo.OverlapInfo.HitComponent == Info->OverlapInfo.HitComponent
            && MyBaseInfo.GetBodyIndex() == Info->GetBodyIndex();
    }

private:
    const FOverlapInfo& MyBaseInfo;

};
