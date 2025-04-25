#include "UScriptComponent.h"

UScriptComponent::UScriptComponent()
    : isScriptLoaded(false)
{

}

UScriptComponent::~UScriptComponent()
{
    // 필요한 정리 작업
}

void UScriptComponent::BeginPlay()
{
    Super::BeginPlay();

    if (isScriptLoaded)
    {
        CallScriptFunction("BeginPlay");
    }
}

void UScriptComponent::TickComponent(float DeltaTime)
{
    Super::TickComponent(DeltaTime);

    if (isScriptLoaded)
    {
        CallScriptFunction("Tick", DeltaTime);
    }
}

void UScriptComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
    if (isScriptLoaded)
    {
        CallScriptFunction("EndPlay");
    }

    Super::EndPlay(EndPlayReason);
}

UObject* UScriptComponent::Duplicate(UObject* InOuter)
{
    ThisClass* NewComponent = Cast<ThisClass>(Super::Duplicate(InOuter));

    NewComponent->scriptPath = scriptPath;
    NewComponent->isScriptLoaded = isScriptLoaded;
    return NewComponent;
}

bool UScriptComponent::LoadScript(const std::string& inScriptPath)
{
    scriptPath = inScriptPath;

    // LuaManager를 통해 스크립트 로드
    try {
        // 스크립트 로드
        sol::state& lua = GLuaManager.GetState();

        // 액터 참조를 Lua 글로벌 변수로 설정
        lua["obj"] = GetOwner();

        // 스크립트 파일 실행
        lua.script_file(scriptPath);

        // 스크립트에서 사용할 함수 등록
        RegisterLuaFunctions(lua);

        isScriptLoaded = true;
        return true;
    }
    catch (const sol::error& e) {
        std::cerr << "스크립트 로드 실패: " << scriptPath
            << "\n오류: " << e.what() << std::endl;
        return false;
    }
}

void UScriptComponent::OnOverlap(AActor* OtherActor)
{
    if (isScriptLoaded)
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

        if (func.valid()) {
            func();
        }
    }
    catch (const sol::error& e) {
        std::cerr << "Lua 함수 호출 오류 (" << functionName << "): "
            << e.what() << std::endl;
    }
}

void UScriptComponent::CallScriptFunction(const char* functionName, float value)
{
    try {
        sol::state& lua = GLuaManager.GetState();
        sol::function func = lua[functionName];

        if (func.valid()) {
            func(value);
        }
    }
    catch (const sol::error& e) {
        std::cerr << "Lua 함수 호출 오류 (" << functionName << "): "
            << e.what() << std::endl;
    }
}

void UScriptComponent::CallScriptFunction(const char* functionName, AActor* actor)
{
    try {
        sol::state& lua = GLuaManager.GetState();
        sol::function func = lua[functionName];

        if (func.valid()) {
            func(actor);
        }
    }
    catch (const sol::error& e) {
        std::cerr << "Lua 함수 호출 오류 (" << functionName << "): "
            << e.what() << std::endl;
    }
}