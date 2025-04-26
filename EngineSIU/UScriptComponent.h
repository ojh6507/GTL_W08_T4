#pragma once
#include "Engine/Classes/Components/ActorComponent.h"
#include <string>
#include "Engine/Script/LuaManager.h"
#include "Engine/Source/Runtime/Engine/Classes/GameFramework/Actor.h"

// 전방 선언
class lua_State;

class UScriptComponent : public UActorComponent
{
    DECLARE_CLASS(UScriptComponent, UActorComponent)

public:
    UScriptComponent();
    virtual ~UScriptComponent();

    // UActorComponent 오버라이드 메서드
    virtual void BeginPlay() override;
    virtual void TickComponent(float DeltaTime) override;  // 'Tick'이 아닌 'TickComponent'로 수정
    virtual void EndPlay(const EEndPlayReason::Type EndPlayReason) override;
    virtual UObject* Duplicate(UObject* InOuter) override;
    virtual void GetProperties(TMap<FString, FString>& OutProperties) const override;
    void SetProperties(const TMap<FString, FString>& Properties) override;
    // 스크립트 관련 함수
    bool LoadScript(const FString& InScriptPath);
    void OnOverlap(AActor* OtherActor);

    void SetScriptPath(const FString& Path) { ScriptPath = Path; }
    // 스크립트 경로 반환
    const FString& GetScriptPath() const { return ScriptPath; }

private:
    FString ScriptPath;
    bool bIsScriptLoaded;
    sol::environment LuaScriptEnv;
    // 스크립트 함수 호출 유틸리티
    void CallScriptFunction(const char* functionName);
    void CallScriptFunction(const char* functionName, float value);
    void CallScriptFunction(const char* functionName, AActor* actor);
};