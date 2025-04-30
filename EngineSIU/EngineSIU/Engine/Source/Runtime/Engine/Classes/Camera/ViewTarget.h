#pragma once
#include "CameraTypes.h"

class AActor;

struct FViewTarget
{
public:
    /** Target Actor used to compute POV */
    AActor* Target;

    /** Computed point of view */
    FMinimalViewInfo POV;

protected:
    // 관전(spectating) 모드에서 캐릭터(Pawn)가 바뀌더라도 동일한 플레이어 상태를 유지하며 계속 같은 플레이어를 따라가기 위해 사용됩니다.
    //class APlayerState PlayerState;

public:
    //class APlayerState* GetPlayerState() const { return PlayerState; }
	
    void SetNewTarget(AActor* NewTarget) { Target = NewTarget; }

    // 지금 Pawn 없어서 주석처리
    //class APawn* GetTargetPawn() const;

    bool Equal(const FViewTarget& OtherTarget) const;

    FViewTarget()
        : Target(nullptr)
    {}
};
