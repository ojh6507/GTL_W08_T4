#pragma once
#include "Actor.h"

class APlayerCameraManager;

// 나중에 AController 상속받게 수정
class APlayerController : public AActor
{
    DECLARE_CLASS(APlayerController, AActor)
public:
    APlayerController();

    APlayerCameraManager* PlayerCameraManager;
};
