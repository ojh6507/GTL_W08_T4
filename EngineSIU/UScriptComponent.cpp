#include "UScriptComponent.h"

UScriptComponent::UScriptComponent()
    : bIsScriptLoaded(false)
{

}

UScriptComponent::~UScriptComponent()
{
    // 필요한 정리 작업
}

void UScriptComponent::BeginPlay()
{
    Super::BeginPlay();

    if (bIsScriptLoaded)
    {
        CallScriptFunction("BeginPlay");
    }
}

void UScriptComponent::TickComponent(const float DeltaTime)
{
    Super::TickComponent(DeltaTime);

    if (bIsScriptLoaded)
    {
        CallScriptFunction("Tick", DeltaTime);
    }
}

void UScriptComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    if (bIsScriptLoaded)
    {
        CallScriptFunction("EndPlay");
    }

    Super::EndPlay(EndPlayReason);
}

UObject* UScriptComponent::Duplicate(UObject* InOuter)
{
    ThisClass* NewComponent = Cast<ThisClass>(Super::Duplicate(InOuter));

    NewComponent->ScriptPath = ScriptPath;
    NewComponent->bIsScriptLoaded = bIsScriptLoaded;
    return NewComponent;
}

bool UScriptComponent::LoadScript(const FString& InScriptPath)
{
    ScriptPath = InScriptPath;

    try {
        // LuaManager를 통해 스크립트 로드
        sol::state& Lua = GLuaManager.GetState();

        // 액터 참조를 Lua 글로벌 변수로 설정
        Lua["obj"] = GetOwner();

        // 스크립트 파일 로드 및 컴파일
        const sol::protected_function_result Result = Lua.script_file(*ScriptPath,
            [this](lua_State* L, sol::protected_function_result pfr) {
                // 상세한 오류 로깅
                if (!pfr.valid()) {
                    const sol::error Err = pfr;
                    std::cerr << "Lua Script Compile Error in " << *ScriptPath << ": "
                        << Err.what() << std::endl;

                    // 추가 디버깅 정보
                    int top = lua_gettop(L);
                    if (top > 0) {
                        const char* errorMsg = lua_tostring(L, top);
                        if (errorMsg) {
                            std::cerr << "Detailed Error: " << errorMsg << std::endl;
                            lua_pop(L, 1);  // 오류 메시지 스택에서 제거
                        }
                    }
                }
                return pfr;
            }
        );

        // 스크립트 로드 결과 명시적 확인
        if (!Result.valid()) {
            std::cerr << "Failed to load script: " << *ScriptPath << std::endl;
            return false;
        }

        // 스크립트에서 사용할 함수 등록
        RegisterLuaFunctions(Lua);

        bIsScriptLoaded = true;
        return true;
    }
    catch (const sol::error& e) {
        // 일반적인 Sol2 예외 처리
        std::cerr << "스크립트 로드 실패: " << *ScriptPath
            << "\n오류: " << e.what() << std::endl;
        bIsScriptLoaded = false;
        return false;
    }
    catch (const std::exception& e) {
        // 표준 예외 처리
        std::cerr << "알 수 없는 오류 발생: " << e.what() << std::endl;
        bIsScriptLoaded = false;
        return false;
    }
    catch (...) {
        // 모든 다른 예외 처리
        std::cerr << "알 수 없는 치명적인 오류 발생" << std::endl;
        bIsScriptLoaded = false;
        return false;
    }
}

void UScriptComponent::OnOverlap(AActor* OtherActor)
{
    if (bIsScriptLoaded)
    {
        CallScriptFunction("OnOverlap", OtherActor);
    }
}

void UScriptComponent::RegisterLuaFunctions(sol::state& lua)
{
    // 위치 설정 함수 (참조로 캡처)
    lua["SetActorLocation"] = [&lua](AActor* actor, float x, float y, float z) {
        if (actor)
        {
            actor->SetActorLocation(FVector(x, y, z));
        }
        };

    // 위치 가져오기 함수 (참조로 캡처)
    lua["GetActorLocation"] = [&lua](AActor* actor) -> sol::table {
        if (actor)
        {
            FVector loc = actor->GetActorLocation();
            sol::table result = lua.create_table();
            result["x"] = loc.X;
            result["y"] = loc.Y;
            result["z"] = loc.Z;
            return result;
        }
        return sol::nil;
        };

    // 회전 설정 함수 (참조로 캡처)
    lua["SetActorRotation"] = [&lua](AActor* actor, float pitch, float yaw, float roll) {
        if (actor)
        {
            actor->SetActorRotation(FRotator(pitch, yaw, roll));
        }
        };

    // 컴포넌트 고유의 추가 함수들
    // 예: 특정 함수나 현재 액터와 관련된 특별한 기능들
    lua["GetComponentOwner"] = [this]() {
        return GetOwner();
        };
}

void UScriptComponent::CallScriptFunction(const char* functionName)
{
    try {
        sol::state& lua = GLuaManager.GetState();
        sol::function func = lua[functionName];

        if (!func.valid()) {
            // 함수가 존재하지 않을 경우
            std::cerr << "Lua 함수를 찾을 수 없음: " << functionName << std::endl;
            return;
        }

        // 함수 호출 시 오류 처리
        auto result = func.call();
        if (!result.valid()) {
            sol::error err = result;
            std::cerr << "Lua 함수 호출 오류 (" << functionName << "): "
                << err.what() << std::endl;

            // 필요하다면 스크립트 비활성화
            bIsScriptLoaded = false;
        }
    }
    catch (const sol::error& e) {
        std::cerr << "Lua 함수 호출 예외 (" << functionName << "): "
            << e.what() << std::endl;
        bIsScriptLoaded = false;
    }
}

void UScriptComponent::CallScriptFunction(const char* functionName, float value)
{
    try {
        sol::state& lua = GLuaManager.GetState();
        sol::function func = lua[functionName];

        if (!func.valid()) {
            // 함수가 존재하지 않을 경우
            std::cerr << "Lua 함수를 찾을 수 없음: " << functionName << std::endl;
            return;
        }

        // 함수 호출 시 오류 처리
        auto result = func.call(value);
        if (!result.valid()) {
            sol::error err = result;
            std::cerr << "Lua 함수 호출 오류 (" << functionName << "): "
                << err.what() << std::endl;

            // 필요하다면 스크립트 비활성화
            bIsScriptLoaded = false;
        }
    }
    catch (const sol::error& e) {
        std::cerr << "Lua 함수 호출 예외 (" << functionName << "): "
            << e.what() << std::endl;
        bIsScriptLoaded = false;
    }
}

void UScriptComponent::CallScriptFunction(const char* functionName, AActor* actor)
{
    try {
        sol::state& lua = GLuaManager.GetState();
        sol::function func = lua[functionName];

        if (!func.valid()) {
            // 함수가 존재하지 않을 경우
            std::cerr << "Lua 함수를 찾을 수 없음: " << functionName << std::endl;
            return;
        }

        // 함수 호출 시 오류 처리
        auto result = func.call(actor);
        if (!result.valid()) {
            sol::error err = result;
            std::cerr << "Lua 함수 호출 오류 (" << functionName << "): "
                << err.what() << std::endl;

            // 필요하다면 스크립트 비활성화
            bIsScriptLoaded = false;
        }
    }
    catch (const sol::error& e) {
        std::cerr << "Lua 함수 호출 예외 (" << functionName << "): "
            << e.what() << std::endl;
        bIsScriptLoaded = false;
    }
}