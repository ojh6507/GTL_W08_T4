#pragma once

extern "C" {
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
}
// #include "luajit.h" 제거
#pragma once
#include <sol/sol.hpp>
#include <memory>
#include <string>
#include "Core/Container/String.h"
#include "Core/Container/Map.h"
#include <mutex>   

class FLuaManager 
{
private:
    std::unique_ptr<sol::state> lua;
    bool initialized;

    void RegisterEngineAPI();
    TMap<FString, sol::protected_function> ScriptChunkCache;

    TMap<FString, FString> ScriptSourceCache;
    std::mutex CacheMutex;
public:
    FLuaManager();
    ~FLuaManager();

    sol::state* GetState();

    bool Initialize();
    void Cleanup();
    sol::protected_function GetOrLoadScriptChunk(const FString& ScriptPath);
    const FString GetOrLoadScriptSource(const FString& Path);
    void ClearScriptCache();
};

extern FLuaManager GLuaManager;
