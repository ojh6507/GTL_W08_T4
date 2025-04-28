#include "UScriptComponent.h"

UScriptComponent::UScriptComponent()
	: bIsScriptLoaded(false)
{

}

UScriptComponent::~UScriptComponent()
{
	// 필요한 정리 작업
}

void UScriptComponent::BeginPlay()
{
	Super::BeginPlay();

	if (bIsScriptLoaded)
	{
		CallScriptFunction("BeginPlay");
	}

	GetOwner()->OnActorBeginOverlap.AddDynamic(this, &UScriptComponent::OnOverlap);
}

void UScriptComponent::TickComponent(const float DeltaTime)
{
	Super::TickComponent(DeltaTime);

	if (bIsScriptLoaded)
	{
		CallScriptFunction("Tick", DeltaTime);
	}
}

void UScriptComponent::EndPlay(const EEndPlayReason::Type EndPlayReason)
{
	if (bIsScriptLoaded)
	{
		CallScriptFunction("EndPlay");
	}

	Super::EndPlay(EndPlayReason);
	LuaScriptEnv = sol::nil;
	bIsScriptLoaded = false;
}

UObject* UScriptComponent::Duplicate(UObject* InOuter)
{
	ThisClass* NewComponent = Cast<ThisClass>(Super::Duplicate(InOuter));

	NewComponent->ScriptPath = ScriptPath;
	NewComponent->bIsScriptLoaded = bIsScriptLoaded;
	return NewComponent;
}

void UScriptComponent::GetProperties(TMap<FString, FString>& OutProperties) const
{
	Super::GetProperties(OutProperties);
	OutProperties.Add(TEXT("ScriptPath"), ScriptPath);
	OutProperties.Add(TEXT("bIsScriptLoaded"), bIsScriptLoaded ? TEXT("true") : TEXT("false"));
}

void UScriptComponent::SetProperties(const TMap<FString, FString>& Properties)
{
	Super::SetProperties(Properties);
	const FString* TempStr = nullptr;


	// --- StaticMesh 설정 ---
	TempStr = Properties.Find(TEXT("ScriptPath"));

	if (TempStr)
	{
		SetScriptPath(*TempStr);
	}
}

bool UScriptComponent::LoadScript(const FString& InScriptPath)
{
	ScriptPath = InScriptPath;
	bIsScriptLoaded = false; // 초기화
	LuaScriptEnv = sol::nil; // 초기화

	try {
		sol::state& Lua = *GLuaManager.GetState(); // 전역 상태

		// 1. 환경 생성
		LuaScriptEnv = sol::environment(Lua, sol::create, Lua.globals());
		if (!LuaScriptEnv.valid()) {
			std::cerr << "Failed to create Lua environment for script: " << *ScriptPath << std::endl;
			return false;
		}
		LuaScriptEnv["self"] = GetOwner();

		// 2. 환경 내에서 스크립트 실행 (script_file 사용)
		sol::protected_function_result result = Lua.script_file(
			*ScriptPath,
			LuaScriptEnv, // 실행 환경 지정
			[this](lua_State* L, sol::protected_function_result pfr) { // 오류 핸들러
				if (!pfr.valid()) {
					sol::error err = pfr;
					std::cerr << "Lua Script Error in " << *ScriptPath << " (env): " << err.what() << std::endl;
					if (lua_gettop(L) > 0 && lua_isstring(L, -1)) {
						std::cerr << "Detailed Error: " << lua_tostring(L, -1) << std::endl;
						lua_pop(L, 1);
					}
				}
				return pfr;
			},
			sol::load_mode::text
		);

		if (!result.valid()) {
			std::cerr << "Failed to execute script in environment: " << *ScriptPath << std::endl;
			LuaScriptEnv = sol::nil;
			return false;
		}

		// --- RegisterLuaFunctions 호출 제거 ---
		// RegisterLuaFunctions(Lua); // 제거! 전역 바인딩은 초기화 시 한 번만!

		bIsScriptLoaded = true;
		std::cout << "Successfully loaded script in environment: " << *ScriptPath << std::endl;

		return true;
	}
	catch (const sol::error& e) {
		std::cerr << "Lua Error during script loading: " << *ScriptPath << "\nError: " << e.what() << std::endl;
		LuaScriptEnv = sol::nil;
		return false;
	}
	catch (const std::exception& e) {
		// 표준 예외 처리
		std::cerr << "알 수 없는 오류 발생: " << e.what() << std::endl;
		bIsScriptLoaded = false;
		return false;
	}
	catch (...) {
		// 모든 다른 예외 처리
		std::cerr << "알 수 없는 치명적인 오류 발생" << std::endl;
		bIsScriptLoaded = false;
		return false;
	}
}

void UScriptComponent::OnOverlap(AActor* OverlappedActor, AActor* OtherActor)
{
	if (bIsScriptLoaded)
	{
		CallScriptFunction("OnOverlap", OtherActor);
	}
}


void UScriptComponent::CallScriptFunction(const char* functionName)
{
	if (!bIsScriptLoaded || !LuaScriptEnv.valid()) return;
	try {
		sol::protected_function func = LuaScriptEnv[functionName]; // 환경에서 함수 찾기
		if (!func.valid()) {
			std::cerr << "Lua function not found in env: " << functionName << std::endl;
			return;
		}
		auto result = func();
		if (!result.valid()) {
			sol::error err = result;
			std::cerr << "Lua function call error (" << functionName << "): " << err.what() << std::endl;
		}
	}
	catch (const sol::error& e) {
		std::cerr << "Lua function call exception (" << functionName << "): " << e.what() << std::endl;
	}
}

void UScriptComponent::CallScriptFunction(const char* functionName, float value)
{
	if (!bIsScriptLoaded || !LuaScriptEnv.valid()) return;
	try {
		sol::protected_function func = LuaScriptEnv[functionName]; // 환경에서 함수 찾기
		if (!func.valid()) return;
		auto result = func(value);
		if (!result.valid()) {
			sol::error err = result;
			std::cerr << "Lua function call error (" << functionName << "): " << err.what() << std::endl;
		}
	}
	catch (const sol::error& e) {
		std::cerr << "Lua function call exception (" << functionName << "): " << e.what() << std::endl;
	}
}

void UScriptComponent::CallScriptFunction(const char* functionName, AActor* otherActor)
{
	if (!bIsScriptLoaded || !LuaScriptEnv.valid()) return;
	try {
		sol::protected_function func = LuaScriptEnv[functionName]; // 환경에서 함수 찾기
		if (!func.valid()) return;
		auto result = func(otherActor);
		if (!result.valid()) {
			sol::error err = result;
			std::cerr << "Lua function call error (" << functionName << "): " << err.what() << std::endl;
		}
	}
	catch (const sol::error& e) {
		std::cerr << "Lua function call exception (" << functionName << "): " << e.what() << std::endl;
	}
}
