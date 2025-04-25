#pragma once

extern "C" {
#include "lua.h"
#include "lualib.h"
#include "lauxlib.h"
}
// #include "luajit.h" 제거

#include <string>

class LuaManager {
private:
    lua_State* L;
    bool initialized;

public:
    LuaManager();
    ~LuaManager();

    bool Initialize();
    void Cleanup();
    lua_State* GetState() { return L; }

    bool RunString(const std::string& code);
    bool RunFile(const std::string& filename);

    void RegisterEngineAPI();
};

// 전역 LuaManager 인스턴스
extern LuaManager GLuaManager;