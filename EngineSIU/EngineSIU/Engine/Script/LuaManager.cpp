#include "LuaManager.h"
#include <iostream>
#include <filesystem>
#include <fstream>
// 전역 인스턴스
FLuaManager GLuaManager;

FLuaManager::FLuaManager() : initialized(false) {}

FLuaManager::~FLuaManager()
{
    //Cleanup();
}

sol::state* FLuaManager::GetState()
{
    if (!initialized)
    {
        throw std::runtime_error("Lua 상태가 초기화되지 않았습니다.");
    }
    return lua.get();
}

bool FLuaManager::Initialize()
{

    lua = std::make_unique<sol::state>();
    try
    {
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
    catch (const std::exception& e)
    {
        std::cerr << "Lua 초기화 실패: " << e.what() << std::endl;
        return false;
    }
}

void FLuaManager::Cleanup()
{
    // sol::state는 자동으로 정리됨
    initialized = false;
    if (lua)
    {
        lua.release();
        lua = nullptr;
    }
}

sol::protected_function FLuaManager::GetOrLoadScriptChunk(const FString& ScriptPath)
{
    //std::lock_guard<std::mutex> lock(CacheMutex);
    const FString CacheKey = ScriptPath;

    // 1. 캐시 확인
    if (ScriptChunkCache.Contains(CacheKey))
    {
        return ScriptChunkCache[CacheKey];
    }

    // 2. Lua 상태 검증
    if (!lua || !initialized)
    {
        std::cerr << "Lua state not initialized while trying to load chunk: "
            << std::string(ScriptPath.GetContainerPrivate()) << std::endl;
        return sol::protected_function();
    }

    // 3. 파일 경로 변환 및 존재 확인
    const std::string PathUtf8 = std::string(ScriptPath.GetContainerPrivate());
    if (!std::filesystem::exists(PathUtf8))
    {
        std::cerr << "Lua script file not found: " << PathUtf8 << std::endl;
        return sol::protected_function();
    }

    // 4. 파일 로드 및 컴파일
    sol::load_result LoadRes = lua->load_file(PathUtf8);
    sol::protected_function Fn;
    if (!LoadRes.valid())
    {
        sol::error Err = LoadRes;
        std::cerr << "Failed to compile Lua script: " << PathUtf8
            << ". Error: " << Err.what() << std::endl;
    }
    else
    {
        Fn = LoadRes;
    }

    ScriptChunkCache.Add(CacheKey, Fn);

    return Fn;
}

// LuaManager.cpp
const FString FLuaManager::GetOrLoadScriptSource(const FString& ScriptPath)
{
    // 1. 캐시 확인
    if (ScriptSourceCache.Contains(ScriptPath))
    {
        return ScriptSourceCache[ScriptPath];
    }

    // 2. 파일 경로 변환 및 파일 열기
    const std::string PathUtf8 = std::string(*ScriptPath);
    std::ifstream File(PathUtf8, std::ios::binary);
    if (!File) {
        std::cerr << "Failed to open script file: " << PathUtf8 << std::endl;
        static const std::string Empty;
        return Empty;
    }

    // 3. 파일 내용 읽기
    std::string Source;
    Source.assign(
        std::istreambuf_iterator<char>(File),
        std::istreambuf_iterator<char>()
    );

   ScriptSourceCache.Add(ScriptPath, std::move(Source));
   return ScriptSourceCache[ScriptPath];
}


void FLuaManager::ClearScriptCache()
{
    std::lock_guard<std::mutex> lock(CacheMutex);
    ScriptChunkCache.Empty();
    std::cout << "Lua script cache cleared." << std::endl;
}


void FLuaManager::RegisterEngineAPI()
{
    // 로그 함수 등록 (람다 사용)
    (*lua)["Log"] = [](const std::string& message) {
        std::cout << "Lua: " << message << std::endl;
        };

    // 추가 엔진 함수 등록 예시
    // lua["Render"] = &YourRenderFunction;
    // lua["GetPlayerPosition"] = &YourPositionFunction;
}
