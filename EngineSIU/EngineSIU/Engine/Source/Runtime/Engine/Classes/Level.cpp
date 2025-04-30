#include "Level.h"
#include "GameFramework/Actor.h"
#include "UObject/Casts.h"
#include "Components/UScriptComponent.h"
#include "Camera/PlayerCameraManager.h"
#include "Camera/CameraComponent.h"


void ULevel::InitLevel(UWorld* InOwningWorld)
{
    OwningWorld = InOwningWorld;

}

void ULevel::Release()
{
    for (AActor* Actor : Actors)
    {
        Actor->EndPlay(EEndPlayReason::WorldTransition);
        TSet<UActorComponent*> Components = Actor->GetComponents();
        for (UActorComponent* Component : Components)
        {
            GUObjectArray.MarkRemoveObject(Component);
        }
        GUObjectArray.MarkRemoveObject(Actor);
    }
    Actors.Empty();
}

UObject* ULevel::Duplicate(UObject* InOuter)
{
    ThisClass* NewLevel = Cast<ThisClass>(Super::Duplicate(InOuter));

    NewLevel->OwningWorld = OwningWorld;

    for (AActor* Actor : Actors)
    {
        AActor* DuplicatedActor = static_cast<AActor*>(Actor->Duplicate(InOuter));

        UScriptComponent* OriginalScriptComp = Actor->GetComponentByClass<UScriptComponent>();

        if (OriginalScriptComp && !OriginalScriptComp->GetScriptPath().IsEmpty())
        {
            // 복제된 액터에서 스크립트 컴포넌트 찾기
            UScriptComponent* NewScriptComp = DuplicatedActor->GetComponentByClass<UScriptComponent>();
            if (NewScriptComp)
            {
                // PIE 월드에서는 실제로 스크립트 로드 및 초기화
                bool success = NewScriptComp->LoadScript(OriginalScriptComp->GetScriptPath());
                std::cout << "PIE: 스크립트 로드 " << (success ? "성공" : "실패")
                    << " - " << *OriginalScriptComp->GetScriptPath() << std::endl;

            }
        }
        // 복제된 액터를 새 레벨에 추가
        NewLevel->Actors.Emplace(DuplicatedActor);
    }

    return NewLevel;
}

void ULevel::RegisterCamera(FName InName, UCameraComponent* InCameraComp, const FViewTarget& View)
{
    // 중복 제거 후 등록
    RemoveCamera(InName);
    Cameras.Add(InName, InCameraComp);
    ViewTargets.Add(InName, View);
}

FViewTarget ULevel::GetViewTarget(const FName InName)
{
    FViewTarget empty;
    if (ViewTargets.Contains(InName))
    {
        return ViewTargets[InName];
    }
    return empty;
}

UCameraComponent* ULevel::GetCameraComponent(FName InName)
{
    if (Cameras.Contains(InName))
    {
        return Cameras[InName];
    }
    return nullptr;
}

uint32 ULevel::RemoveCamera(const FName InName)
{
    const size_t OriginalSize = Cameras.Num();
    Cameras.Remove(InName);
    return OriginalSize - Cameras.Num();
}

