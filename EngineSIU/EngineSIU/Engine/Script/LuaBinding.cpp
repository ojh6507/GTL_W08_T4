#pragma once

#include "LuaBinding.h"
#include "Engine/Engine.h"
#include "UObject/Object.h"          // UObject

#include "GameFramework/Actor.h"     // AActor
#include "Components/ActorComponent.h" // UActorComponent
#include "Components/SceneComponent.h" // USceneComponent

#include "UObject/Class.h"           // UClass
#include "Engine/EngineTypes.h"      // EEndPlayReason
#include "Container/Set.h"           // TSet

#include "../../../SoundManager.h"
#include "../../../Timer.h"
#include "../../../GameUI.h"

#include "UnrealEd/UnrealEd.h"
#include "World/WorldType.h"
#include "World/World.h"
#include "PropertyEditor/ControlEditorPanel.h"
#include "Components/UScriptComponent.h"
#include "Camera/CameraShakeModifier.h"
#include "Camera/PlayerCameraManager.h"
#include <Camera/CameraModifier_Interpolation.h>

// --- !!! 중요: 선행 바인딩 필요 !!! ---
// 이 파일 내의 바인딩 함수들은 서로 의존성을 가집니다.
// 예를 들어, FName은 FString을 사용하고, AActor는 FName, FString, 수학 타입 등을 사용합니다.
// BindCoreTypesForLua 함수에서 올바른 순서로 호출하는 것이 매우 중요합니다.
// 또한, 아래 플레이스홀더 함수들은 실제 바인딩 코드로 구현되어야 합니다.
// ------------------------------------
namespace LuaBindings
{

    void BindEWorldType(sol::state& lua)
    {
        lua.new_enum("EWorldType",
            "None", EWorldType::None,
            "Game", EWorldType::Game,
            "Editor", EWorldType::Editor,
            "PIE", EWorldType::PIE, // Play In Editor
            "EditorPreview", EWorldType::EditorPreview,
            "GamePreview", EWorldType::GamePreview,
            "Inactive", EWorldType::Inactive
        );
    }
    void BindUObject(sol::state& lua) {
        auto ut = lua.new_usertype<UObject>("UObject", sol::no_constructor);

        // 멤버 함수
        ut["Duplicate"] = &UObject::Duplicate;
        ut["GetOuter"] = &UObject::GetOuter;
        ut["GetWorld"] = &UObject::GetWorld;
        ut["GetFName"] = &UObject::GetFName;

        ut["GetName"] = [](const UObject& self) {
            FString fstr = self.GetName();
#if USE_WIDECHAR
            return fstr.ToAnsiString();
#else
            return static_cast<std::string>(fstr);
#endif
            };

        ut["GetUUID"] = &UObject::GetUUID;
        ut["GetInternalIndex"] = &UObject::GetInternalIndex;
        ut["GetClass"] = &UObject::GetClass;
        ut["IsA"] = static_cast<bool (UObject::*)(const UClass*) const>(&UObject::IsA);

        ut["EncodeUUID"] = &UObject::EncodeUUID;

        // 정적 함수
        sol::table tbl = lua["UObject"];
        tbl["StaticClass"] = &UObject::StaticClass;
    }
    AActor* SpawnActorWrapper(UWorld* world, UClass* actorClass, const FVector& location, const FRotator& rotation, FName actorName)
    {
        if (!world) {
            throw sol::error("UWorld::SpawnActor called on a nil world");
        }
        if (!actorClass) {
            throw sol::error("UWorld::SpawnActor called with a nil class");
        }

        // C++ UWorld::SpawnActor(UClass*, FName)는 위치/회전을 직접 받지 않으므로,
        // 먼저 스폰하고 나서 위치/회전을 설정합니다.
        AActor* spawnedActor = world->SpawnActor(actorClass, actorName);

        if (spawnedActor) {
            // 스폰 성공 시 위치와 회전 설정
            // AActor에 SetActorLocationAndRotation 같은 함수가 있다고 가정합니다.
            // 실제 엔진의 함수 이름에 맞춰야 할 수 있습니다.
            if (!location.IsZero() || !rotation.IsZero()) // 기본값(0,0,0)이 아니면 설정
            {
                // 실제 엔진에 맞는 함수 사용 (예시)
                spawnedActor->SetActorLocation(location);
                spawnedActor->SetActorRotation(rotation);
            }
        }
        return spawnedActor;
    }

    UClass* LuaFindClass(const std::string& className)
    {
        FName classFName(className.c_str());

        return UClass::FindClass(classFName);
    }


    void BindUClass(sol::state& lua) {
        // UObject와 FName이 먼저 바인딩되어야 함

        sol::usertype<UClass> ut = lua.new_usertype<UClass>("UClass",
            sol::no_constructor,
            sol::base_classes, sol::bases<UObject>(),

            // --- 멤버 함수 바인딩 ---
            "GetClassSize", &UClass::GetClassSize,
            "GetClassAlignment", &UClass::GetClassAlignment,
            "IsChildOf", static_cast<bool (UClass::*)(const UClass*) const>(&UClass::IsChildOf),
            "GetSuperClass", &UClass::GetSuperClass,
            "GetDefaultObject", static_cast<UObject * (UClass::*)() const>(&UClass::GetDefaultObject),
            "GetName", [](const UClass& self) {
                FName nm = self.GetName();
                FString fstr = nm.ToString();
#if USE_WIDECHAR
                return fstr.ToAnsiString();
#else
                return static_cast<std::string>(fstr);
#endif
            }
        );

        // --- 정적 함수 바인딩 ---
        // UClass 테이블 가져오기
        sol::table uclass_table = lua["UClass"];
        // FindClass 바인딩은 여기서 제거하고 전역으로 이동
        // tbl["FindClass"] = &LuaFindClass; // <<< 이 줄 제거 또는 주석 처리
    }

    // --- UWorld 바인딩 함수 ---
    void BindUWorld(sol::state& lua)
    {

        // UWorld usertype 정의
        lua.new_usertype<UWorld>("UWorld",
            sol::no_constructor, // Lua에서 직접 생성 금지
            sol::base_classes, sol::bases<UObject>(), // UObject 상속 (UObject 바인딩 필요)

            // --- 멤버 함수 바인딩 ---

            // SpawnActor (래퍼 함수 사용)
            // Lua: world:SpawnActor(class, location, rotation, name)
            // FName 기본값을 NAME_None으로 설정하기 위해 오버로드 사용
            "SpawnActor", sol::overload(
                // 이름만 받는 경우 (위치/회전은 기본값 0 사용)
                [](UWorld* world, UClass* actorClass, FName actorName) {
                    return SpawnActorWrapper(world, actorClass, FVector::ZeroVector, FRotator(), actorName);
                },
                // 이름 없이 위치/회전만 받는 경우
                [](UWorld* world, UClass* actorClass, const FVector& location, const FRotator& rotation) {
                    return SpawnActorWrapper(world, actorClass, location, rotation, NAME_None);
                },
                // 모든 인자를 받는 경우
                &SpawnActorWrapper,
                // 클래스만 받는 경우 (위치/회전/이름 모두 기본값)
                [](UWorld* world, UClass* actorClass) {
                    return SpawnActorWrapper(world, actorClass, FVector::ZeroVector, FRotator(), NAME_None);
                }
            ),
            "GetPlayerCameraManager", sol::overload(
                [](UWorld* world) {
                    return world->GetPlayerCameraManager();
                }),
            // DestroyActor
            // Lua: world:DestroyActor(actor_instance)
            "DestroyActor", &UWorld::DestroyActor,

            // GetWorld (UObject의 가상 함수 오버라이드)
            // Lua: world:GetWorld() -> world (자기 자신 반환)
            "GetWorld", &UWorld::GetWorld,

            "GetViewTarget", &UWorld::GetViewTarget,

            // GetActiveLevel (ULevel 바인딩이 필요함)
            // Lua: local level = world:GetActiveLevel()
            "GetActiveLevel", &UWorld::GetActiveLevel, // ULevel* 반환

            // GetWorldName
            // Lua: local name_str = world:GetWorldName()
            "GetWorldName", [](const UWorld& self) -> std::string {
                // FString을 std::string으로 변환 (FString 바인딩 방식에 따라 달라질 수 있음)
                FString worldName = self.GetWorldName();
#if USE_WIDECHAR
                return worldName.ToAnsiString(); // 예시: UTF-8 std::string 반환 가정
#else
                return static_cast<std::string>(worldName); // 예시: UTF-8 std::string 반환 가정
#endif
                // TODO: FString -> std::string 변환 로직 확인/수정 필요
            },

            // --- 멤버 변수 바인딩 ---

            // WorldType (읽기 전용으로 노출하는 것이 안전할 수 있음)
            // Lua: local type_enum = world.WorldType
            "WorldType", sol::readonly(&UWorld::WorldType) // EWorldType 반환

            // 참고: Tick, BeginPlay, Release 등은 보통 엔진 내부에서 호출되므로
            //       Lua에서 직접 호출할 필요가 없는 경우가 많아 바인딩에서 제외했습니다.
            //       필요하다면 추가할 수 있습니다.

            // 참고: DuplicateActor 템플릿 함수도 필요하다면 SpawnActor와 유사하게
            //       래퍼 함수를 만들어 바인딩해야 합니다.
        );

        // Lua에서 현재 World 인스턴스를 얻는 방법 제공 (예시 - 전역 변수)
        // 주의: GEngine->GetWorld()는 상황에 따라 null일 수 있으므로 사용 시 주의
        if (GEngine) // GEngine 유효성 검사
        {
            lua["GWorld"] = GEngine->GetWorld(); // 현재 월드를 GWorld 전역 변수로 제공
        }
        else
        {
            lua["GWorld"] = sol::nil; // GEngine 없으면 nil 설정
        }
        // 또는 함수 형태로 제공하는 것이 더 안전할 수 있습니다.
        // lua["GetWorld"] = [](){ return GEngine ? GEngine->GetWorld() : nullptr; };
    }
    // 검색 시 대소문자 구분 옵션 Enum 바인딩
    void BindESearchCase(sol::state& lua) {
        lua.new_enum("ESearchCase",
            "CaseSensitive", ESearchCase::CaseSensitive, // 대소문자 구분
            "IgnoreCase", ESearchCase::IgnoreCase   // 대소문자 무시
        );
    }

    // 검색 방향 Enum 바인딩
    void BindESearchDir(sol::state& lua) {
        lua.new_enum("ESearchDir",
            "FromStart", ESearchDir::FromStart, // 앞에서부터 검색
            "FromEnd", ESearchDir::FromEnd     // 뒤에서부터 검색
        );
    }

    void BindFString(sol::state& lua)
    {
        // 의존하는 Enum 타입들 먼저 바인딩
        BindESearchCase(lua);
        BindESearchDir(lua);

        // 1단계: Usertype 기본 정의 (생성자 없이 시작)
        sol::usertype<FString> ut = lua.new_usertype<FString>("FString",
            sol::no_constructor // 생성자는 나중에 테이블에 직접 추가
        );

        // 2단계: 멤버 함수 및 변수 바인딩 (ut.set 사용)
        ut.set("Len", &FString::Len);
        ut.set("IsEmpty", &FString::IsEmpty);
        ut.set("Empty", &FString::Empty);
        ut.set("Equals", sol::resolve<bool(const FString&, ESearchCase::Type) const>(&FString::Equals));
        ut.set("Contains", sol::resolve<bool(const FString&, ESearchCase::Type, ESearchDir::Type) const>(&FString::Contains));
        ut.set("Find", sol::resolve<int32(const FString&, ESearchCase::Type, ESearchDir::Type, int32) const>(&FString::Find));
        ut.set("Reserve", &FString::Reserve);
        ut.set("Resize", &FString::Resize);
        ut.set("ToUpper", sol::resolve<FString() const&>(&FString::ToUpper));
        ut.set("ToLower", sol::resolve<FString() const&>(&FString::ToLower));
        ut.set("ToUpperInline", &FString::ToUpperInline);
        ut.set("ToLowerInline", &FString::ToLowerInline);
        ut.set("ToBool", &FString::ToBool);
        ut.set("RightChop", &FString::RightChop);
        ut.set("Append", &FString::operator+=); // += 연산자를 Append 멤버 함수로 바인딩
        ut.set("CharAt", [](const FString& self, int index) -> std::string {
            if (index >= 1 && index <= self.Len()) {
                FString::ElementType ch = self[index - 1];
#if USE_WIDECHAR
                wchar_t wch[2] = { ch, 0 };
                std::wstring temp_wstr(wch);
                int sizeNeeded = WideCharToMultiByte(CP_UTF8, 0, temp_wstr.c_str(), -1, nullptr, 0, nullptr, nullptr);
                if (sizeNeeded > 0) {
                    std::string result(sizeNeeded - 1, 0);
                    WideCharToMultiByte(CP_UTF8, 0, temp_wstr.c_str(), -1, &result[0], sizeNeeded, nullptr, nullptr);
                    return result;
                }
                else { return ""; }
#else
                char ach[2] = { ch, 0 };
                return std::string(ach);
#endif
            }
            throw sol::error("FString CharAt: Index out of bounds");
            });

        // 3단계: 연산자 (메타메소드) 바인딩 (ut.set 사용)
        ut.set(sol::meta_function::addition, sol::resolve<FString(const FString&, const FString&)>(&operator+));
        ut.set(sol::meta_function::equal_to, sol::resolve<bool(const FString&) const>(&FString::operator==));
        ut.set(sol::meta_function::length, &FString::Len);
        ut.set(sol::meta_function::to_string, [](const FString& self) -> std::string {
#if USE_WIDECHAR
            return self.ToAnsiString();
#else
            return static_cast<std::string>(self);
#endif
            // TODO: FString 변환 로직 확인
            });

        // 4단계: 생성자/팩토리 및 정적 함수를 Lua 테이블에 직접 추가
        sol::table fstring_table = lua["FString"]; // FString usertype 테이블 얻기

        // 생성자/팩토리를 'new' 함수로 오버로딩하여 추가
        fstring_table["new"] = sol::overload(
            []() { return FString(); }, // 기본 생성자
            [](const char* lua_str) { return FString(lua_str); },
            [](const std::string& lua_std_str) { return FString(lua_std_str); }
#if USE_WIDECHAR
        , [](const wchar_t* lua_wstr) { return FString(lua_wstr); }
        , [](const std::wstring& lua_std_wstr) { return FString(lua_std_wstr); }
#endif
            );

        // 정적 함수 추가
        fstring_table["ToInt"] = sol::resolve<int(const FString&)>(&FString::ToInt);
        fstring_table["ToFloat"] = sol::resolve<float(const FString&)>(&FString::ToFloat);
        fstring_table["FromInt"] = sol::overload( // 정적 함수 오버로딩
            [](int i) -> FString { return FString::FromInt(i); },
            [](long long ll) -> FString { return FString::FromInt(ll); }
        );
        fstring_table["SanitizeFloat"] = &FString::SanitizeFloat;

        // lua.add_usertype<FString>(); // 필요시 추가
    }
    // --- FName 바인딩 ---
    void BindFName(sol::state& lua)
    {
        lua.new_usertype<FName>("FName",
            sol::call_constructor,
            sol::constructors<
            FName(),              // FName.new() - None 상태
            FName(const char*),   // FName.new("MyName")
            FName(const FString&) // FName.new(fstring_obj) (주의: FString 바인딩 필요)
            >(),

            // 멤버 함수
            "ToString", [](const FName& self) -> std::string {
                FString fstr = self.ToString();
                // 중요: FString -> std::string 변환 로직 필요
#if USE_WIDECHAR
                return self.ToAnsiString(); // 예시: UTF-8 std::string 반환 가정
#else
                return static_cast<std::string>(fstr); // 예시: UTF-8 std::string 반환 가정
#endif
                // TODO: 위 가정/변환 로직 확인 필요
            },
            "IsNone", sol::resolve<bool(ENameNone) const>(&FName::operator==), // fname:IsNone()

            // 정적 멤버 변수
            "None", sol::var(FName()), // FName.None

            // 연산자 (메타메소드)
            sol::meta_function::equal_to, sol::resolve<bool(const FName&) const>(&FName::operator==), // fname1 == fname2
            sol::meta_function::to_string, [](const FName& self) -> std::string { // print(fname)
                FString fstr = self.ToString();
                // 중요: FString -> std::string 변환 로직 필요
#if USE_WIDECHAR
                return self.ToAnsiString(); // 예시: UTF-8 std::string 반환 가정
#else
                return static_cast<std::string>(fstr); // 예시: UTF-8 std::string 반환 가정
#endif
                // TODO: 위 가정/변환 로직 확인 필요
            }
        );
        // 관련 Enum 바인딩
        lua["NAME_None"] = ENameNone::NAME_None; // 전역 변수 NAME_None
    }

    // --- FVector2D 바인딩 ---
    void BindFVector2D(sol::state& lua)
    {
        lua.new_usertype<FVector2D>("FVector2D",
            sol::call_constructor,
            sol::constructors<FVector2D(), FVector2D(float, float), FVector2D(float)>(),
            "X", &FVector2D::X, "Y", &FVector2D::Y,
            "ZeroVector", sol::var(FVector2D::ZeroVector), "OneVector", sol::var(FVector2D::OneVector),
            sol::meta_function::addition, sol::resolve<FVector2D(const FVector2D&) const>(&FVector2D::operator+),
            sol::meta_function::subtraction, sol::resolve<FVector2D(const FVector2D&) const>(&FVector2D::operator-),
            sol::meta_function::multiplication, sol::resolve<FVector2D(float) const>(&FVector2D::operator*),
            sol::meta_function::division, sol::resolve<FVector2D(float) const>(&FVector2D::operator/),
            sol::meta_function::equal_to, sol::resolve<bool(const FVector2D&) const>(&FVector2D::operator==),
            "ToString", [](const FVector2D& self) -> std::string {
                FString fstr = self.ToString();
#if USE_WIDECHAR
                return self.ToAnsiString(); // 예시
#else
                return static_cast<std::string>(fstr); // 예시
#endif
                // TODO: FString 변환 로직 확인
            },
            "InitFromString", &FVector2D::InitFromString, // FString 바인딩 필요
            sol::meta_function::to_string, [](const FVector2D& self) -> std::string {
                FString fstr = self.ToString();
#if USE_WIDECHAR
                return self.ToAnsiString(); // 예시
#else
                return static_cast<std::string>(fstr); // 예시
#endif
                // TODO: FString 변환 로직 확인
            }
        );
    }

    // --- FVector 바인딩 ---
    void BindFVector(sol::state& lua)
    {
        lua.new_usertype<FVector>("FVector",
            sol::call_constructor,
            sol::constructors<FVector(), FVector(float, float, float), FVector(float), FVector(const FRotator&)>(), // FRotator 바인딩 필요
            "X", &FVector::X, "Y", &FVector::Y, "Z", &FVector::Z,
            // 정적 멤버들
            "ZeroVector", sol::var(FVector::ZeroVector), "OneVector", sol::var(FVector::OneVector),
            "UpVector", sol::var(FVector::UpVector), "DownVector", sol::var(FVector::DownVector),
            "ForwardVector", sol::var(FVector::ForwardVector), "BackwardVector", sol::var(FVector::BackwardVector),
            "RightVector", sol::var(FVector::RightVector), "LeftVector", sol::var(FVector::LeftVector),
            "XAxisVector", sol::var(FVector::XAxisVector), "YAxisVector", sol::var(FVector::YAxisVector), "ZAxisVector", sol::var(FVector::ZAxisVector),
            // 정적 함수들
            "Zero", &FVector::Zero, "One", &FVector::One, "UnitX", &FVector::UnitX, "UnitY", &FVector::UnitY, "UnitZ", &FVector::UnitZ,
            "Distance", &FVector::Distance, "DotProduct", &FVector::DotProduct, "CrossProduct", &FVector::CrossProduct, "GetAbs", &FVector::GetAbs,
            // 연산자 및 멤버 함수
            "Dot", sol::resolve<float(const FVector&) const>(&FVector::Dot),
            "Cross", sol::resolve<FVector(const FVector&) const>(&FVector::Cross),
            sol::meta_function::addition, sol::resolve<FVector(const FVector&) const>(&FVector::operator+),
            sol::meta_function::subtraction, sol::resolve<FVector(const FVector&) const>(&FVector::operator-),
            sol::meta_function::multiplication, sol::overload(
                sol::resolve<FVector(const FVector&) const>(&FVector::operator*), sol::resolve<FVector(float) const>(&FVector::operator*)
            ),
            sol::meta_function::division, sol::overload(
                sol::resolve<FVector(const FVector&) const>(&FVector::operator/), sol::resolve<FVector(float) const>(&FVector::operator/)
            ),
            sol::meta_function::unary_minus, sol::resolve<FVector() const>(&FVector::operator-),
            sol::meta_function::equal_to, sol::resolve<bool(const FVector&) const>(&FVector::operator==),
            "Equals", &FVector::Equals, "AllComponentsEqual", &FVector::AllComponentsEqual,
            "Length", &FVector::Length, "LengthSquared", &FVector::LengthSquared,
            "Normalize", sol::resolve<bool(float)>(&FVector::Normalize),
            "GetUnsafeNormal", &FVector::GetUnsafeNormal, "GetSafeNormal", &FVector::GetSafeNormal,
            "ComponentMin", &FVector::ComponentMin, "ComponentMax", &FVector::ComponentMax,
            "IsNearlyZero", &FVector::IsNearlyZero, "IsZero", &FVector::IsZero,
            // 문자열 변환
            "ToString", [](const FVector& self) -> std::string {
                FString fstr = self.ToString();
#if USE_WIDECHAR
                return self.ToAnsiString(); // 예시
#else
                return static_cast<std::string>(fstr); // 예시
#endif
                // TODO: FString 변환 로직 확인
            },
            "InitFromString", &FVector::InitFromString, // FString 바인딩 필요
            sol::meta_function::to_string, [](const FVector& self) -> std::string {
                FString fstr = self.ToString();
#if USE_WIDECHAR
                return self.ToAnsiString(); // 예시
#else
                return static_cast<std::string>(fstr); // 예시
#endif
                // TODO: FString 변환 로직 확인
            }
        );
    }

    // --- FRotator 바인딩 ---
    void BindFRotator(sol::state& lua)
    {
        lua.new_usertype<FRotator>("FRotator",
            sol::call_constructor,
            sol::constructors<FRotator(), FRotator(float, float, float), FRotator(const FRotator&), FRotator(const FVector&), FRotator(const FQuat&)>(), // FVector, FQuat 바인딩 필요
            "Pitch", &FRotator::Pitch, "Yaw", &FRotator::Yaw, "Roll", &FRotator::Roll,
            sol::meta_function::addition, sol::resolve<FRotator(const FRotator&) const>(&FRotator::operator+),
            sol::meta_function::subtraction, sol::resolve<FRotator(const FRotator&) const>(&FRotator::operator-),
            sol::meta_function::multiplication, sol::resolve<FRotator(float) const>(&FRotator::operator*),
            sol::meta_function::division, sol::overload(
                sol::resolve<FRotator(const FRotator&) const>(&FRotator::operator/), sol::resolve<FRotator(float) const>(&FRotator::operator/)
            ),
            sol::meta_function::unary_minus, sol::resolve<FRotator() const>(&FRotator::operator-),
            sol::meta_function::equal_to, sol::resolve<bool(const FRotator&) const>(&FRotator::operator==),
            "IsNearlyZero", &FRotator::IsNearlyZero, "IsZero", &FRotator::IsZero, "Equals", &FRotator::Equals, "Add", &FRotator::Add,
            "FromQuaternion", &FRotator::FromQuaternion, // FQuat 바인딩 필요
            "ToQuaternion", &FRotator::ToQuaternion,     // FQuat 바인딩 필요
            "ToVector", &FRotator::ToVector,             // FVector 바인딩 필요
            "ToMatrix", &FRotator::ToMatrix,             // FMatrix 바인딩 필요
            "Clamp", &FRotator::Clamp, "GetNormalized", &FRotator::GetNormalized, "Normalize", &FRotator::Normalize,
            "ToString", [](const FRotator& self) -> std::string {
                FString fstr = self.ToString();
#if USE_WIDECHAR
                return self.ToAnsiString(); // 예시
#else
                return static_cast<std::string>(fstr); // 예시
#endif
                // TODO: FString 변환 로직 확인
            },
            "InitFromString", &FRotator::InitFromString, // FString 바인딩 필요
            sol::meta_function::to_string, [](const FRotator& self) -> std::string {
                FString fstr = self.ToString();
#if USE_WIDECHAR
                return self.ToAnsiString(); // 예시
#else
                return static_cast<std::string>(fstr); // 예시
#endif
                // TODO: FString 변환 로직 확인
            }
        );
    }

    // --- FQuat 바인딩 ---
    void BindFQuat(sol::state& lua)
    {
        lua.new_usertype<FQuat>("FQuat",
            sol::constructors<FQuat(), FQuat(float, float, float, float), FQuat(const FVector&, float), FQuat(const FMatrix&)>(), // FVector, FMatrix 바인딩 필요
            "W", &FQuat::W, "X", &FQuat::X, "Y", &FQuat::Y, "Z", &FQuat::Z,
            sol::meta_function::multiplication, sol::resolve<FQuat(const FQuat&) const>(&FQuat::operator*),
            "RotateVector", &FQuat::RotateVector, // FVector 바인딩 필요
            "IsNormalized", &FQuat::IsNormalized, "Normalize", &FQuat::Normalize,
            "ToMatrix", &FQuat::ToMatrix, // FMatrix 바인딩 필요
            "FromAxisAngle", &FQuat::FromAxisAngle, // FVector 바인딩 필요
            "CreateRotation", &FQuat::CreateRotation
        );
    }

    // --- FMatrix 바인딩 ---
    void BindFMatrix(sol::state& lua)
    {
        lua.new_usertype<FMatrix>("FMatrix",
            "Identity", sol::var(FMatrix::Identity),
            sol::meta_function::addition, sol::resolve<FMatrix(const FMatrix&) const>(&FMatrix::operator+),
            sol::meta_function::subtraction, sol::resolve<FMatrix(const FMatrix&) const>(&FMatrix::operator-),
            sol::meta_function::multiplication, sol::overload(
                sol::resolve<FMatrix(const FMatrix&) const>(&FMatrix::operator*), sol::resolve<FMatrix(float) const>(&FMatrix::operator*)
            ),
            sol::meta_function::division, sol::resolve<FMatrix(float) const>(&FMatrix::operator/),
            "Get", [](const FMatrix& self, int row, int col) -> float {
                if (row >= 1 && row <= 4 && col >= 1 && col <= 4) return self.M[row - 1][col - 1];
                throw sol::error("Matrix indices out of bounds (1-4)");
            },
            "Set", [](FMatrix& self, int row, int col, float value) {
                if (row >= 1 && row <= 4 && col >= 1 && col <= 4) self.M[row - 1][col - 1] = value;
                else throw sol::error("Matrix indices out of bounds (1-4)");
            },
            "Transpose", &FMatrix::Transpose, "Inverse", &FMatrix::Inverse,
            "CreateRotationMatrix", sol::overload(
                sol::resolve<FMatrix(float, float, float)>(&FMatrix::CreateRotationMatrix),
                sol::resolve<FMatrix(const FRotator&)>(&FMatrix::GetRotationMatrix), // FRotator 바인딩 필요
                sol::resolve<FMatrix(const FQuat&)>(&FMatrix::GetRotationMatrix)     // FQuat 바인딩 필요
            ),
            "CreateScaleMatrix", sol::overload(
                sol::resolve<FMatrix(float, float, float)>(&FMatrix::CreateScaleMatrix),
                sol::resolve<FMatrix(const FVector&)>(&FMatrix::GetScaleMatrix)         // FVector 바인딩 필요
            ),
            "CreateTranslationMatrix", sol::overload(
                sol::resolve<FMatrix(const FVector&)>(&FMatrix::CreateTranslationMatrix),
                sol::resolve<FMatrix(const FVector&)>(&FMatrix::GetTranslationMatrix)    // FVector 바인딩 필요
            ),
            "TransformVector", sol::overload(
                sol::resolve<FVector(const FVector&, const FMatrix&)>(&FMatrix::TransformVector),   // FVector 바인딩 필요
                sol::resolve<FVector4(const FVector4&, const FMatrix&)>(&FMatrix::TransformVector)  // FVector4 바인딩 필요
            ),
            "TransformFVector4", &FMatrix::TransformFVector4, // FVector4 바인딩 필요
            "TransformPosition", &FMatrix::TransformPosition, // FVector 바인딩 필요
            "ToQuat", &FMatrix::ToQuat // FQuat 바인딩 필요
        );
    }
    void BindFColor(sol::state& lua)
    {
        lua.new_usertype<FColor>("FColor",
            sol::call_constructor,
            sol::constructors<
            FColor(),                               // FColor.new() -> Black, A=255
            FColor(uint8, uint8, uint8),            // FColor.new(r, g, b) -> A=255
            FColor(uint8, uint8, uint8, uint8),     // FColor.new(r, g, b, a)
            FColor(uint32)                          // FColor.new(bits)
            >(),

            // 멤버 변수
            "R", &FColor::R,
            "G", &FColor::G,
            "B", &FColor::B,
            "A", &FColor::A,
        
            // 연산자 (메타메소드)
            sol::meta_function::equal_to, &FColor::operator==, // color1 == color2
            sol::meta_function::to_string, [](const FColor& self) -> std::string {
                // 간단한 문자열 표현 제공
                return "FColor(R=" + std::to_string(self.R) +
                    ", G=" + std::to_string(self.G) +
                    ", B=" + std::to_string(self.B) +
                    ", A=" + std::to_string(self.A) + ")";
            }
        );

        // 정적 멤버 변수 (미리 정의된 색상) - 테이블에 직접 추가
        sol::table fcolor_table = lua["FColor"];
        fcolor_table["White"] = sol::var(FColor::White);
        fcolor_table["Black"] = sol::var(FColor::Black);
        fcolor_table["Transparent"] = sol::var(FColor::Transparent);
        fcolor_table["Red"] = sol::var(FColor::Red);
        fcolor_table["Green"] = sol::var(FColor::Green);
        fcolor_table["Blue"] = sol::var(FColor::Blue);
        fcolor_table["Yellow"] = sol::var(FColor::Yellow);
        fcolor_table["Cyan"] = sol::var(FColor::Cyan);
        fcolor_table["Magenta"] = sol::var(FColor::Magenta);
        fcolor_table["Orange"] = sol::var(FColor::Orange);
        fcolor_table["Purple"] = sol::var(FColor::Purple);
        fcolor_table["Turquoise"] = sol::var(FColor::Turquoise);
        fcolor_table["Silver"] = sol::var(FColor::Silver);
        fcolor_table["Emerald"] = sol::var(FColor::Emerald);
    }

    void BindFLinearColor(sol::state& lua)
    {
        // 의존성: FString, FVector, FVector4, FColor 바인딩 필요
        lua.new_usertype<FLinearColor>("FLinearColor",
            sol::call_constructor,
            sol::constructors<
            FLinearColor(),                                 // FLinearColor.new() -> (0,0,0,0)
            FLinearColor(float, float, float),              // FLinearColor.new(r, g, b) -> A=1.0
            FLinearColor(float, float, float, float),       // FLinearColor.new(r, g, b, a)
            // FString 생성자는 InitFromString을 통해 간접적으로 지원됨 (아래 참조)
            // explicit 생성자는 sol::constructors로 직접 바인딩 가능
            FLinearColor(const FVector&),                   // FLinearColor.new(vector3) -> A=1.0
            FLinearColor(const FVector4&),                  // FLinearColor.new(vector4)
            FLinearColor(const FColor&)                     // FLinearColor.new(fcolor)
            >(),

            // 멤버 변수
            "R", &FLinearColor::R,
            "G", &FLinearColor::G,
            "B", &FLinearColor::B,
            "A", &FLinearColor::A,

            // 멤버 함수
            "GetClamp", sol::overload( // 기본 인자 처리를 위한 오버로드
                [](const FLinearColor& c) { return c.GetClamp(); },
                [](const FLinearColor& c, float min) { return c.GetClamp(min); },
                sol::resolve<FLinearColor(float, float) const>(&FLinearColor::GetClamp)
            ),
            "Equals", sol::overload( // 기본 인자 처리를 위한 오버로드
                [](const FLinearColor& c, const FLinearColor& other) { return c.Equals(other); },
                sol::resolve<bool(const FLinearColor&, float) const>(&FLinearColor::Equals)
            ),
            // 참고: Lerp는 인스턴스를 수정하므로, 필요하다면 정적 버전(FMath::Lerp)을 별도 바인딩하는 것이 더 일반적일 수 있음
            "Lerp", &FLinearColor::Lerp, // 인스턴스 자체를 변경함: color:Lerp(start, end, alpha)
            "GetMax", &FLinearColor::GetMax,
            "GetMin", &FLinearColor::GetMin,
            "ToString", [](const FLinearColor& self) -> std::string {
                FString fstr = self.ToString();
#if USE_WIDECHAR
                return self.ToAnsiString(); // 예시
#else
                return static_cast<std::string>(fstr); // 예시
#endif
                // TODO: FString 변환 로직 확인
            },
            "InitFromString", [](FLinearColor& self, const std::string& source) -> bool {
                // Lua 문자열(std::string)을 FString으로 변환하여 호출
                return self.InitFromString(FString(source.c_str()));
            },

            // 연산자 (메타메소드)
            sol::meta_function::addition, sol::resolve<FLinearColor(const FLinearColor&) const>(&FLinearColor::operator+),
            sol::meta_function::subtraction, sol::resolve<FLinearColor(const FLinearColor&) const>(&FLinearColor::operator-),
            sol::meta_function::multiplication, sol::overload(
                sol::resolve<FLinearColor(const FLinearColor&) const>(&FLinearColor::operator*),
                sol::resolve<FLinearColor(float) const>(&FLinearColor::operator*)
            ),
            sol::meta_function::division, sol::overload(
                sol::resolve<FLinearColor(const FLinearColor&) const>(&FLinearColor::operator/),
                sol::resolve<FLinearColor(float) const>(&FLinearColor::operator/)
            ),
            sol::meta_function::equal_to, sol::resolve<bool(const FLinearColor&) const>(&FLinearColor::operator==),
            sol::meta_function::to_string, [](const FLinearColor& self) -> std::string { // print(color)
                FString fstr = self.ToString();
#if USE_WIDECHAR
                return self.ToAnsiString(); // 예시
#else
                return static_cast<std::string>(fstr); // 예시
#endif
                // TODO: FString 변환 로직 확인
            }
            // 참고: +=, -=, *=, /= 연산자는 Lua에서 a = a + b 등으로 구현되므로 별도 바인딩은 보통 불필요
        );

        // 정적 멤버 변수 (미리 정의된 색상) - 테이블에 직접 추가
        sol::table flinearcolor_table = lua["FLinearColor"];
        flinearcolor_table["White"] = sol::var(FLinearColor::White);
        flinearcolor_table["Gray"] = sol::var(FLinearColor::Gray);
        flinearcolor_table["Black"] = sol::var(FLinearColor::Black);
        flinearcolor_table["Transparent"] = sol::var(FLinearColor::Transparent);
        flinearcolor_table["Red"] = sol::var(FLinearColor::Red);
        flinearcolor_table["Green"] = sol::var(FLinearColor::Green);
        flinearcolor_table["Blue"] = sol::var(FLinearColor::Blue);
        flinearcolor_table["Yellow"] = sol::var(FLinearColor::Yellow);

        // 정적 함수
        flinearcolor_table["FromColor"] = &FLinearColor::FromColor; // FColor 바인딩 필요
        // FString 생성자 래핑 (InitFromString 사용)
        flinearcolor_table["FromString"] = [](const std::string& source) -> FLinearColor {
            FLinearColor color;
            color.InitFromString(FString(source.c_str()));
            return color;
            };
    }
    // --- 수학 관련 타입 전체 바인딩 함수 ---
    void BindMathTypes(sol::state& lua)
    {
        // 의존성 순서 고려: FVector4는 FMatrix에서 사용되므로 먼저 바인딩 필요
        BindFVector2D(lua);
        BindFVector(lua);
        BindFQuat(lua);
        BindFRotator(lua);
        BindFMatrix(lua);
        BindFColor(lua);
        BindFLinearColor(lua);
    }

    // --- AActor 바인딩 ---
    void BindAActor(sol::state& lua)
    {

        lua.new_usertype<AActor>("AActor",
            sol::no_constructor, // Lua에서 직접 생성 금지
            sol::base_classes, sol::bases<UObject>(), // UObject 상속 (UObject 바인딩 필요)

            // 핵심 함수
            "Destroy", &AActor::Destroy,
            "IsActorBeingDestroyed", &AActor::IsActorBeingDestroyed,

            // 컴포넌트 관련
            "AddComponent", sol::resolve<UActorComponent* (UClass*, FName, bool)>( // UClass, FName 바인딩 필요
                static_cast<UActorComponent * (AActor::*)(UClass*, FName, bool)>(&AActor::AddComponent)
            ),

            "GetRootComponent", &AActor::GetRootComponent, // USceneComponent 바인딩 필요
            "SetRootComponent", &AActor::SetRootComponent, // USceneComponent 바인딩 필요

            // 소유자 관련
            "GetOwner", sol::resolve<AActor * () const>(&AActor::GetOwner),
            "SetOwner", &AActor::SetOwner,

            // 트랜스폼 관련
            "GetActorLocation", &AActor::GetActorLocation,
            "GetActorRotation", &AActor::GetActorRotation,
            "GetActorScale", &AActor::GetActorScale,

            "SetActorLocation", &AActor::SetActorLocation,
            "SetActorRotation", &AActor::SetActorRotation,
            "SetActorScale", &AActor::SetActorScale,

            "GetActorForwardVector", &AActor::GetActorForwardVector,
            "GetActorRightVector", &AActor::GetActorRightVector,
            "GetActorUpVector", &AActor::GetActorUpVector,

            // 에디터 관련
            "GetActorLabel", &AActor::GetActorLabel, // FString 바인딩 필요
            "SetActorLabel", &AActor::SetActorLabel, // FString 바인딩 필요
            "IsActorTickInEditor", &AActor::IsActorTickInEditor, "SetActorTickInEditor", &AActor::SetActorTickInEditor,

            // 수명 주기 함수
            "BeginPlay", &AActor::BeginPlay, "Tick", &AActor::Tick, "EndPlay", &AActor::EndPlay, // EEndPlayReason 바인딩 필요

            // UObject 상속 함수
            "GetName", [](const AActor& self) -> std::string {
                FName objectName = self.GetName(); // UObject::GetName() 가정
                FString fstr = objectName.ToString();
#if USE_WIDECHAR
                return self.ToAnsiString(); // 예시
#else
                return static_cast<std::string>(fstr); // 예시
#endif
                // TODO: FString 변환 로직 확인
            },
            "GetClass", &AActor::GetClass // UClass 바인딩 필요
        );
    }

    void BindInputForLua(sol::state& lua)
    {
        // 키 상태 확인 함수 (Windows API 사용)
        lua["IsKeyDown"] = [](int32 VirtualKey) -> bool {
            return (GetKeyState(VirtualKey) & 0x8000) != 0;
            };

        lua["KEY_LEFT"] = VK_LEFT;
        lua["KEY_UP"] = VK_UP;
        lua["KEY_RIGHT"] = VK_RIGHT;
        lua["KEY_DOWN"] = VK_DOWN;

        for (int i = 'A'; i <= 'Z'; i++) {
            std::string keyName = "KEY_" + std::string(1, (char)i);
            lua[keyName.c_str()] = i;
        }
    }
    void BindUScriptComponent(sol::state& lua)
    {
        // Ensure UActorComponent is bound first
        auto ut = lua.new_usertype<UScriptComponent>("ScriptComponent", // Lua name can be shorter
            sol::no_constructor,
            sol::base_classes, sol::bases<UActorComponent, UObject>(), // Specify inheritance

            // Bind relevant public methods
            "LoadScript", [](UScriptComponent& self, const std::string& luaPath) -> bool {
                // Convert std::string from Lua to FString for C++ function
                return self.LoadScript(FString(luaPath.c_str()));
            },
            "SetScriptPath", [](UScriptComponent& self, const std::string& luaPath) {
                // Convert std::string from Lua to FString
                self.SetScriptPath(FString(luaPath.c_str()));
            },
            "GetScriptPath", [](const UScriptComponent& self) -> std::string {
                // Convert FString return value to std::string for Lua
                const FString& path = self.GetScriptPath();
#if USE_WIDECHAR
                return path.ToAnsiString(); // Or other appropriate conversion
#else
                return static_cast<std::string>(path);
#endif
            }

            // Note: BeginPlay, TickComponent, EndPlay, OnOverlap are typically called *by* the
            // engine or C++ code, which then call Lua functions. They usually aren't called *from* Lua.
            // So, we don't bind them here for Lua to call.

            // Note: GetProperties/SetProperties are often for editor/serialization,
            // maybe not needed for direct Lua gameplay scripting. Omitted for now.

            // Note: Duplicate is also less common for direct Lua gameplay. Omitted.
        );
    }

    void BindSoundManager(sol::state& lua)
    {
        // SoundManager 싱글톤을 위한 함수 바인딩
        lua.set_function("GetSoundManager", []() -> SoundManager& {
            return SoundManager::GetInstance();
            });

        // SoundManager 클래스 바인딩
        lua.new_usertype<SoundManager>("SoundManager",
            sol::no_constructor, // 생성자 없음 (싱글톤)

            // 멤버 함수들 바인딩
            "Initialize", &SoundManager::Initialize,
            "ShutDown", &SoundManager::ShutDown,
            "LoadSound", &SoundManager::LoadSound,
            "PlaySound", &SoundManager::PlaySound,
            "Update", &SoundManager::Update,
            "LoadSoundFiles", &SoundManager::LoadSoundFiles,
            "Stop", &SoundManager::Stop
        );

        lua["SoundManager"] = &SoundManager::GetInstance();
    }

    // LuaBindings.cpp 파일에 추가
    void BindUI(sol::state& lua)
    {
        lua.new_usertype<Timer>("Timer",
            // 생성자
            sol::constructors<Timer()>(),

            // 타이머 제어 함수
            "Start", &Timer::Start,
            "Stop", &Timer::Stop,
            "Reset", &Timer::Reset,
            "Pause", &Timer::Pause,
            "Resume", &Timer::Resume,

            // 타이머 상태 확인 함수
            "IsRunning", &Timer::IsRunning,
            "IsPaused", &Timer::IsPaused,
            "GetTime", &Timer::GetTime,

            // 타이머 업데이트
            "Update", &Timer::Update

        );

        lua.new_usertype<GameUI>("GameUI",
            sol::no_constructor, // 직접 생성 방지 (엔진에서 관리)
            "Initialize", &GameUI::Initialize,
            "Shutdown", &GameUI::Shutdown,
            "Update", &GameUI::Update,
            "RenderTimerUI", &GameUI::RenderTimerUI,
            "GetTimer", &GameUI::GetTimer,  // 타이머 객체 접근자

            "SetStartButtonCallback", &GameUI::SetStartButtonCallback,

            "SetPauseButtonCallback", &GameUI::SetPauseButtonCallback,

            "SetResumeButtonCallback", &GameUI::SetResumeButtonCallback,

            "SetResetButtonCallback", &GameUI::SetResetButtonCallback,
            "SetDamageEventCallback", &GameUI::SetDamageEventCallback,


            "SetLives", &GameUI::SetLives,

            "GetLives", &GameUI::GetLives,

            "LoseLife", &GameUI::LoseLife,

            "ResetLives", &GameUI::ResetLives

        );

        // 전역 GameUI 인스턴스 제공 (엔진에서 관리하는 인스턴스 주소)
        auto gameUIPanel = UnrealEd::GetEditorPanel("GameUI");
        auto gameUI = std::dynamic_pointer_cast<GameUI>(gameUIPanel);
        lua["gameUI"] = gameUI;
        lua["Timer"] = gameUI->GetTimer();
    }

    // LuaBindings.cpp 파일 또는 해당하는 파일에 추가

    void BindMagicButton(sol::state& lua)
    {
        lua.new_usertype<ControlEditorPanel>("ControlEditorPanel",
            "StartPIE", &ControlEditorPanel::StartPIE,
            "EndPIE", &ControlEditorPanel::EndPIE
        );

        auto controlPanel = UnrealEd::GetEditorPanel("ControlPanel");
        auto control = std::dynamic_pointer_cast<ControlEditorPanel>(controlPanel);

        lua["ControlEditorPanelInstance"] = control;
    }

    void BindPlayerCameraManager(sol::state& lua)
    {
        lua.new_usertype<APlayerCameraManager>("PlayerCameraManager",
            sol::no_constructor, // Lua에서 직접 생성 금지

            "StartCameraFade", &APlayerCameraManager::StartCameraFade,
           
            // 모디파이어 관련 메서드

            "GetShakeModifier", [](APlayerCameraManager* self) -> UCameraShakeModifier* {
                UCameraModifier* modifier = self->GetModifierByType(EModifierType::Shake);
                return Cast<UCameraShakeModifier>(modifier);
            },

            "GetMoveModifier", [](APlayerCameraManager* self) -> UCameraModifier_Interpolation* {
                UCameraModifier* modifier = self->GetModifierByType(EModifierType::Move);
                return Cast<UCameraModifier_Interpolation>(modifier);
            },

            "AddModifier", &APlayerCameraManager::AddModifier,
            "RemoveModifier", &APlayerCameraManager::RemoveModifier
        );

        lua.new_enum("EModifierType",
            "Shake", EModifierType::Shake
        );

       

        lua.new_usertype<UCameraShakeModifier>("CameraShakeModifier",
            sol::no_constructor, // Lua에서 직접 생성 금지

            "StartShake", &UCameraShakeModifier::StartShake,
            "StopShake", &UCameraShakeModifier::StopShake,

            // 속성들 노출
            "ShakeIntensity", &UCameraShakeModifier::ShakeIntensity,
            "ShakeDuration", &UCameraShakeModifier::ShakeDuration,
            "ShakeTimeRemaining", &UCameraShakeModifier::ShakeTimeRemaining,
            "bIsShaking", &UCameraShakeModifier::bIsShaking,
            "bIsStart", &UCameraShakeModifier::bIsStart
        );

        lua.new_usertype<UCameraModifier_Interpolation>("CameraMoveModifier",
            sol::no_constructor,
            "Initialize", &UCameraModifier_Interpolation::Initialize
            );
    }

    // --- 코어 타입 전체 바인딩 호출 함수 ---
    void BindCoreTypesForLua(sol::state& lua)
    {
        // --- 바인딩 순서 정의 ---
        BindEWorldType(lua);

        // 1. 기본 문자열 및 이름 타입
        BindFString(lua); // Enum들에 의존
        BindFName(lua);   // FString에 의존

        // 2. 수학 타입 (다른 타입들에서 광범위하게 사용됨)
        BindMathTypes(lua); // FVector4 실제 구현 필요

        BindUObject(lua);
        BindUClass(lua);

        // 4. 주요 게임플레이 클래스
        BindUWorld(lua);
        BindAActor(lua);

        // 5. 컨테이너 관련 (필요한 경우)
        lua["FindClass"] = &LuaFindClass;

        BindSoundManager(lua);
        BindUI(lua);
        BindMagicButton(lua);
    
        BindPlayerCameraManager(lua);
    }
} // namespace LuaBindings
