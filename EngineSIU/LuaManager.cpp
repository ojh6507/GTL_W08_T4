#include "LuaManager.h"
#include <iostream>

// 전역 인스턴스
LuaManager gLuaManager;

LuaManager::LuaManager() : L(nullptr), initialized(false) {}

LuaManager::~LuaManager() {
    Cleanup();
}

bool LuaManager::Initialize() {
    if (initialized) return true;

    // Lua 상태 생성
    L = luaL_newstate();
    if (!L) {
        std::cerr << "Lua 초기화 실패!" << std::endl;
        return false;
    }

    // 기본 라이브러리 로드
    luaL_openlibs(L);

    // 엔진 API 등록
    RegisterEngineAPI();

    initialized = true;
    std::cout << "Lua 초기화 완료" << std::endl;
    return true;
}

void LuaManager::Cleanup() {
    if (L) {
        lua_close(L);
        L = nullptr;
    }
    initialized = false;
}

bool LuaManager::RunString(const std::string& code) {
    if (!L) return false;

    if (luaL_dostring(L, code.c_str()) != 0) {
        std::cerr << "Lua 오류: " << lua_tostring(L, -1) << std::endl;
        lua_pop(L, 1);
        return false;
    }
    return true;
}

bool LuaManager::RunFile(const std::string& filename) {
    if (!L) return false;

    if (luaL_dofile(L, filename.c_str()) != 0) {
        std::cerr << "Lua 파일 오류: " << lua_tostring(L, -1) << std::endl;
        lua_pop(L, 1);
        return false;
    }
    return true;
}

// 로그 함수 (Lua에서 호출)
static int LuaLog(lua_State* L) {
    const char* message = luaL_checkstring(L, 1);
    std::cout << "Lua: " << message << std::endl;
    return 0;
}

void LuaManager::RegisterEngineAPI() {
    if (!L) return;

    // 유틸리티 함수 등록
    lua_register(L, "Log", LuaLog);

    // 여기에 엔진 고유 함수들을 등록하세요
    // 예: 렌더링, 입력, 물리 등의 함수
}