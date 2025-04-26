#include <sol/sol.hpp>
#include <string>
#include <stdexcept> // sol::error 사용을 위해 필요
#include <vector>    // FString::Printf 구현에서 사용 (C++ 코드 내)
#include <cmath>     // 수학 함수 사용 (C++ 코드 내)
#include <algorithm> // std::ranges::transform 등 사용 (C++ 코드 내)
#include <cctype>    // std::tolower, std::toupper 등 사용 (C++ 코드 내)


#include "UObject/NameTypes.h"         // FName
#include "Container/String.h"        // FString
#include "Math/Vector.h"           // FVector, FVector2D
#include "Math/Vector4.h"          // FVector4
#include "Math/Rotator.h"          // FRotator
#include "Math/Quat.h"             // FQuat
#include "Math/Matrix.h"           // FMatrix
#include "UObject/Object.h"          // UObject
#include "GameFramework/Actor.h"     // AActor
#include "Components/ActorComponent.h" // UActorComponent
#include "Components/SceneComponent.h" // USceneComponent
#include "UObject/Class.h"           // UClass
#include "Engine/EngineTypes.h"      // EEndPlayReason
#include "Container/Set.h"           // TSet

// --- !!! 중요: 선행 바인딩 필요 !!! ---
// 이 파일 내의 바인딩 함수들은 서로 의존성을 가집니다.
// 예를 들어, FName은 FString을 사용하고, AActor는 FName, FString, 수학 타입 등을 사용합니다.
// BindCoreTypesForLua 함수에서 올바른 순서로 호출하는 것이 매우 중요합니다.
// 또한, 아래 플레이스홀더 함수들은 실제 바인딩 코드로 구현되어야 합니다.
// ------------------------------------

namespace LuaBindings
{

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
            // 생성자
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

    // --- 수학 관련 타입 전체 바인딩 함수 ---
    void BindMathTypes(sol::state& lua)
    {
        // 의존성 순서 고려: FVector4는 FMatrix에서 사용되므로 먼저 바인딩 필요
        BindFVector2D(lua);
        BindFVector(lua);
        BindFQuat(lua);
        BindFRotator(lua);
        BindFMatrix(lua);
    }

    // --- AActor 바인딩 ---
    void BindAActor(sol::state& lua)
    {
        // 필요한 선행 바인딩들이 BindCoreTypesForLua에서 호출되었다고 가정

        lua.new_usertype<AActor>("AActor",
            sol::no_constructor, // Lua에서 직접 생성 금지
            sol::base_classes, sol::bases<UObject>(), // UObject 상속 (UObject 바인딩 필요)

            // 핵심 함수
            "Destroy", &AActor::Destroy,
            "IsActorBeingDestroyed", &AActor::IsActorBeingDestroyed,

            // 컴포넌트 관련
            "AddComponent", sol::resolve<UActorComponent * (UClass*, FName, bool)>( // UClass, FName 바인딩 필요
                static_cast<UActorComponent * (AActor::*)(UClass*, FName, bool)>(&AActor::AddComponent)
            ),
          
            "GetRootComponent", &AActor::GetRootComponent, // USceneComponent 바인딩 필요
            "SetRootComponent", &AActor::SetRootComponent, // USceneComponent 바인딩 필요

            // 소유자 관련
            "GetOwner", sol::resolve<AActor * () const>(&AActor::GetOwner),
            "SetOwner", &AActor::SetOwner,

            // 트랜스폼 관련
            "GetActorLocation", &AActor::GetActorLocation, "GetActorRotation", &AActor::GetActorRotation, "GetActorScale", &AActor::GetActorScale,
            "SetActorLocation", &AActor::SetActorLocation, "SetActorRotation", &AActor::SetActorRotation, "SetActorScale", &AActor::SetActorScale,
            "GetActorForwardVector", &AActor::GetActorForwardVector, "GetActorRightVector", &AActor::GetActorRightVector, "GetActorUpVector", &AActor::GetActorUpVector,

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

    // --- 코어 타입 전체 바인딩 호출 함수 ---
    void BindCoreTypesForLua(sol::state& lua)
    {
        // --- 바인딩 순서 정의 ---

        // 0. 기본 Enum 타입들
        // BindESearchCase(lua); // BindFString 내부에서 호출됨
        // BindESearchDir(lua);  // BindFString 내부에서 호출됨
        // BindEEndPlayReason_Placeholder(lua); // 실제 구현 필요

        // 1. 기본 문자열 및 이름 타입
        BindFString(lua); // Enum들에 의존
        BindFName(lua);   // FString에 의존

        // 2. 수학 타입 (다른 타입들에서 광범위하게 사용됨)
        BindMathTypes(lua); // FVector4 실제 구현 필요

        //// 3. UObject 및 기본 클래스 계층 (AActor, 컴포넌트 등의 기반)
        //BindUObject_Placeholder(lua);         // 실제 구현 필요
        //BindUClass_Placeholder(lua);          // 실제 구현 필요
        //BindUActorComponent_Placeholder(lua); // 실제 구현 필요
        //BindUSceneComponent_Placeholder(lua); // 실제 구현 필요

        // 4. 주요 게임플레이 클래스
        BindAActor(lua); // 수학 타입, UObject, 컴포넌트 등에 의존

        // 5. 컨테이너 관련 (필요한 경우)
        // TSet 등 컨테이너 바인딩 (또는 테이블 변환 헬퍼 사용)

        // --- UWorld 등 다른 필요한 타입 바인딩 ---
        // BindUWorld_Placeholder(lua); // SpawnActor 등 사용 시 필요
    }

} // namespace LuaBindings
