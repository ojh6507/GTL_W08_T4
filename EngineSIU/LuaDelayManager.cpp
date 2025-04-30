// LuaDelay.cpp
#include "LuaDelayManager.h"
#include <algorithm>

LuaDelayManager& LuaDelayManager::GetInstance() {
    static LuaDelayManager instance;
    return instance;
}

void LuaDelayManager::AddDelay(lua_State* L, float delayTime, int functionRef) {
    DelayedCall call;
    call.timeRemaining = delayTime;
    call.functionRef = functionRef;
    call.L = L;

    m_delayedCalls.push_back(call);
}

void LuaDelayManager::Update(float deltaTime) {
    // 완료된 호출을 추적하기 위한 인덱스 목록
    std::vector<size_t> completedCalls;

    // 모든 지연 호출 업데이트
    for (size_t i = 0; i < m_delayedCalls.size(); ++i) {
        auto& call = m_delayedCalls[i];
        call.timeRemaining -= deltaTime;

        // 시간이 다 되었으면 함수 호출
        if (call.timeRemaining <= 0) {
            lua_State* L = call.L;

            // 등록된 함수를 스택에 푸시
            lua_rawgeti(L, LUA_REGISTRYINDEX, call.functionRef);

            // 함수 호출
            if (lua_pcall(L, 0, 0, 0) != 0) {
                const char* error = lua_tostring(L, -1);
                printf("Delayed function error: %s\n", error);
                lua_pop(L, 1);
            }

            // 함수 참조 해제
            luaL_unref(L, LUA_REGISTRYINDEX, call.functionRef);

            // 완료된 호출 표시
            completedCalls.push_back(i);
        }
    }

    // 완료된 호출을 뒤에서부터 제거 (인덱스 무효화 방지)
    for (int i = completedCalls.size() - 1; i >= 0; --i) {
        size_t index = completedCalls[i];
        if (index < m_delayedCalls.size()) {
            m_delayedCalls.erase(m_delayedCalls.begin() + index);
        }
    }
}

void LuaDelayManager::Clear() {
    for (auto& call : m_delayedCalls) {
        luaL_unref(call.L, LUA_REGISTRYINDEX, call.functionRef);
    }
    m_delayedCalls.clear();
}

LuaDelayManager::~LuaDelayManager() {
    Clear();
}

// Lua에서 호출할 지연 함수
int Lua_Delay(lua_State* L) {
    // 인자 확인: (지연시간, 함수)
    if (lua_gettop(L) != 2 || !lua_isnumber(L, 1) || !lua_isfunction(L, 2)) {
        return luaL_error(L, "Delay expects (number, function)");
    }

    float delay = static_cast<float>(lua_tonumber(L, 1));

    // 함수를 복사하여 스택 최상단에 놓기
    lua_pushvalue(L, 2);

    // 함수를 레지스트리에 참조로 저장
    int functionRef = luaL_ref(L, LUA_REGISTRYINDEX);

    // 지연 매니저에 추가
    LuaDelayManager::GetInstance().AddDelay(L, delay, functionRef);

    return 0;
}

// Lua에 함수 등록
void RegisterDelayToLua(lua_State* L) {
    lua_register(L, "Delay", Lua_Delay);
}