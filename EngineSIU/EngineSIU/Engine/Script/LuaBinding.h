#pragma once // 헤더 가드

// --- 필수 헤더 ---
#include "sol/sol.hpp" // sol 필수 포함
#include <string> // std::string 사용

// --- 언리얼 타입 선언 (포인터/참조만 사용 시 전방 선언으로 충분할 수 있음) ---
#include "UObject/NameTypes.h"     // FName (값 타입이므로 포함)
#include "Container/String.h"    // FString (값 타입이므로 포함)
#include "Math/Vector.h"       // FVector, FVector2D (값 타입이므로 포함)
#include "Math/Rotator.h"      // FRotator (값 타입이므로 포함)
#include "Math/Quat.h"         // FQuat (값 타입이므로 포함)
#include "Math/Matrix.h"       // FMatrix (값 타입이므로 포함)
#include "Math/Color.h"
#include "Engine/EngineTypes.h"  // EWorldType, ESearchCase, ESearchDir (Enum 정의 포함)

// --- 전방 선언 (포인터/참조 타입) ---
class UObject;
class UClass;
class UWorld;
class AActor;
class UActorComponent;
class USceneComponent;
class UScriptComponent;
class SoundManager;
class Timer;
class GameUI;
class ControlEditorPanel;
// --- 전방 선언 끝 ---


// --- Lua 바인딩 함수 선언 ---
namespace LuaBindings
{
    // --- Enum & 기본 타입 바인딩 ---
    void BindEWorldType(sol::state& lua);
    void BindESearchCase(sol::state& lua);
    void BindESearchDir(sol::state& lua);
    void BindFString(sol::state& lua);
    void BindFName(sol::state& lua);

    // --- 수학 타입 바인딩 ---
    void BindFVector2D(sol::state& lua);
    void BindFVector(sol::state& lua);
    void BindFRotator(sol::state& lua);
    void BindFQuat(sol::state& lua);
    void BindFMatrix(sol::state& lua);
    void BindMathTypes(sol::state& lua); // 위 수학 타입 바인딩 함수들을 호출

    // --- UObject 및 파생 클래스 바인딩 ---
    void BindUObject(sol::state& lua);
    void BindUClass(sol::state& lua);
    void BindUWorld(sol::state& lua);
    void BindAActor(sol::state& lua);
    void BindUScriptComponent(sol::state& lua); // UActorComponent 바인딩이 선행되어야 함 (BindCoreTypesForLua에서 처리 가정)

    // --- 유틸리티 함수 ---
    AActor* SpawnActorWrapper(UWorld* world, UClass* actorClass, const FVector& location, const FRotator& rotation, FName actorName);
    UClass* LuaFindClass(const std::string& className);

    // --- 게임 관련 클래스 바인딩 ---
    void BindInputForLua(sol::state& lua);
    void BindSoundManager(sol::state& lua);
    void BindUI(sol::state& lua);
    void BindMagicButton(sol::state& lua);

    // --- 전체 바인딩 호출 함수 ---
    void BindCoreTypesForLua(sol::state& lua);

} // namespace LuaBindings
