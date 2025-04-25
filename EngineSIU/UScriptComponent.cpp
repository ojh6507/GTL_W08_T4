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

bool UScriptComponent::LoadScript(const std::string& inScriptPath)
{
    scriptPath = inScriptPath;

    // LuaManager를 통해 스크립트 로드
    lua_State* L = GLuaManager.GetState();
    if (!L) return false;

    if (!GLuaManager.RunFile(scriptPath))
    {
        std::cerr << "스크립트 로드 실패: " << scriptPath << std::endl;
        return false;
    }

    // 스크립트에 액터 참조 설정
    lua_pushlightuserdata(L, GetOwner());
    lua_setglobal(L, "obj");

    // 스크립트에서 사용할 함수 등록
    RegisterLuaFunctions();

    isScriptLoaded = true;
    return true;
}

void UScriptComponent::OnOverlap(AActor* OtherActor)
{
    if (isScriptLoaded)
    {
        CallScriptFunction("OnOverlap", OtherActor);
    }
}

void UScriptComponent::RegisterLuaFunctions()
{
    // 여기서 스크립트에서 사용할 C++ 함수를 등록
    lua_State* L = GLuaManager.GetState();
    if (!L) return;

    // 예시: 위치 설정 함수
    lua_register(L, "SetActorLocation", [](lua_State* L) -> int {
        AActor* actor = (AActor*)lua_touserdata(L, 1);
        float x = (float)luaL_checknumber(L, 2);
        float y = (float)luaL_checknumber(L, 3);
        float z = (float)luaL_checknumber(L, 4);

        if (actor)
        {
            actor->SetActorLocation(FVector(x, y, z));
        }
        return 0;
        });

    // 필요한 다른 함수들도 등록
}

void UScriptComponent::CallScriptFunction(const char* functionName)
{
    lua_State* L = GLuaManager.GetState();
    if (!L) return;

    lua_getglobal(L, functionName);
    if (lua_isfunction(L, -1))
    {
        if (lua_pcall(L, 0, 0, 0) != 0)
        {
            std::cerr << "Lua 함수 호출 오류 (" << functionName << "): "
                << lua_tostring(L, -1) << std::endl;
            lua_pop(L, 1);
        }
    }
    else
    {
        lua_pop(L, 1);
    }
}

void UScriptComponent::CallScriptFunction(const char* functionName, float value)
{
    lua_State* L = GLuaManager.GetState();
    if (!L) return;

    lua_getglobal(L, functionName);
    if (lua_isfunction(L, -1))
    {
        lua_pushnumber(L, value);
        if (lua_pcall(L, 1, 0, 0) != 0)
        {
            std::cerr << "Lua 함수 호출 오류 (" << functionName << "): "
                << lua_tostring(L, -1) << std::endl;
            lua_pop(L, 1);
        }
    }
    else
    {
        lua_pop(L, 1);
    }
}

void UScriptComponent::CallScriptFunction(const char* functionName, AActor* actor)
{
    lua_State* L = GLuaManager.GetState();
    if (!L) return;

    lua_getglobal(L, functionName);
    if (lua_isfunction(L, -1))
    {
        lua_pushlightuserdata(L, actor);
        if (lua_pcall(L, 1, 0, 0) != 0)
        {
            std::cerr << "Lua 함수 호출 오류 (" << functionName << "): "
                << lua_tostring(L, -1) << std::endl;
            lua_pop(L, 1);
        }
    }
    else
    {
        lua_pop(L, 1);
    }
}