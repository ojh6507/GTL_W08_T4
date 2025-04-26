#include "LuaManager.h"
#include <iostream>

// 전역 인스턴스
LuaManager GLuaManager;

LuaManager::LuaManager() : initialized(false) {}

LuaManager::~LuaManager() {
    //Cleanup();
}

sol::state* LuaManager::GetState()
{
    if (!initialized) {
        throw std::runtime_error("Lua 상태가 초기화되지 않았습니다.");
    }
    return lua.get();
}

bool LuaManager::Initialize() {

    lua = std::make_unique<sol::state>();
    try {
        // 기본 라이브러리 로드
        lua->open_libraries(
            sol::lib::base,
            sol::lib::package,
            sol::lib::string,
            sol::lib::math,
            sol::lib::table
        );

        // 엔진 API 등록
        RegisterEngineAPI();

        initialized = true;
        std::cout << "Lua 초기화 완료" << std::endl;
        return true;
    }
    catch (const std::exception& e) {
        std::cerr << "Lua 초기화 실패: " << e.what() << std::endl;
        return false;
    }
}

void LuaManager::Cleanup() 
{
    // sol::state는 자동으로 정리됨
    initialized = false;
    if (lua)
    {
        lua.release();
        lua = nullptr;
    }
}

bool LuaManager::RunString(const std::string& code) {
    if (!initialized) return false;

    try {
        lua->script(code);
        return true;
    }
    catch (const sol::error& e) {
        std::cerr << "Lua 오류: " << e.what() << std::endl;
        return false;
    }
}

bool LuaManager::RunFile(const std::string& filename) {
    if (!initialized) return false;

    try {
        lua->script_file(filename);
        return true;
    }
    catch (const sol::error& e) {
        std::cerr << "Lua 파일 오류: " << e.what() << std::endl;
        return false;
    }
}

void LuaManager::RegisterEngineAPI() {
    // 로그 함수 등록 (람다 사용)
    (*lua)["Log"] = [](const std::string& message) {
        std::cout << "Lua: " << message << std::endl;
        };

    // 추가 엔진 함수 등록 예시
    // lua["Render"] = &YourRenderFunction;
    // lua["GetPlayerPosition"] = &YourPositionFunction;
}