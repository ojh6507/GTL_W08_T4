// LuaDelay.h
#pragma once
#include <lua.hpp>
#include <vector>
#include <functional>

struct DelayedCall {
    float timeRemaining;  // 남은 시간
    int functionRef;      // Lua 함수 참조
    lua_State* L;         // Lua 상태
};

class LuaDelayManager {
public:
    static LuaDelayManager& GetInstance();

    // 지연 함수 등록
    void AddDelay(lua_State* L, float delayTime, int functionRef);

    // 모든 지연 함수 업데이트
    void Update(float deltaTime);

    // 정리
    void Clear();
   
private:
    LuaDelayManager() {}
    ~LuaDelayManager();
    
    std::vector<DelayedCall> m_delayedCalls;
};

// Lua에 등록할 함수
int Lua_Delay(lua_State* L);

// Lua에 Delay 함수 등록
void RegisterDelayToLua(lua_State* L);