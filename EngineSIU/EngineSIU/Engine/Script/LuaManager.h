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

class FLuaManager 
{
private:
    std::unique_ptr<sol::state> lua;
    bool initialized;

    void RegisterEngineAPI();

public:
    FLuaManager();
    ~FLuaManager();

    sol::state* GetState();

    bool Initialize();
    void Cleanup();
    bool RunString(const std::string& code);
    bool RunFile(const std::string& filename);
};

extern FLuaManager GLuaManager;
