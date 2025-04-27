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

	TempStr = Properties.Find(TEXT("ScriptPath"));

	if (TempStr)
	{
		SetScriptPath(*TempStr);
	}
}

//bool UScriptComponent::LoadScript(const FString& InScriptPath)
//{
//	ScriptPath = InScriptPath;
//	bIsScriptLoaded = false; // 초기화
//	LuaScriptEnv = sol::nil; // 초기화
//
//	try
//	{
//		sol::state& Lua = *GLuaManager.GetState(); // 전역 상태
//
//		// 1. 환경 생성
//		LuaScriptEnv = sol::environment(Lua, sol::create, Lua.globals());
//		if (!LuaScriptEnv.valid())
//		{
//			std::cerr << "Failed to create Lua environment for script: " << *ScriptPath << '\n';
//			return false;
//		}
//		LuaScriptEnv["self"] = GetOwner();
//
//		// 2. 환경 내에서 스크립트 실행 (script_file 사용)
//		const sol::protected_function_result Result = Lua.script_file(
//			*ScriptPath,
//			LuaScriptEnv, // 실행 환경 지정
//			[this](lua_State* L, sol::protected_function_result pfr)
//			{ // 오류 핸들러
//				if (!pfr.valid())
//				{
//					sol::error e = pfr;
//					std::cerr << "Lua Script Error in " << *ScriptPath << " (env): " << e.what() << '\n';
//					if (lua_gettop(L) > 0 && lua_isstring(L, -1))
//					{
//						std::cerr << "Detailed Error: " << lua_tostring(L, -1) << '\n';
//						lua_pop(L, 1);
//					}
//				}
//				return pfr;
//			},
//			sol::load_mode::text
//		);
//
//		if (!Result.valid())
//		{
//			std::cerr << "Failed to execute script in environment: " << *ScriptPath << '\n';
//			LuaScriptEnv = sol::nil;
//			return false;
//		}
//
//		// --- RegisterLuaFunctions 호출 제거 ---
//		// RegisterLuaFunctions(Lua); // 제거! 전역 바인딩은 초기화 시 한 번만!
//
//		bIsScriptLoaded = true;
//		std::cout << "Successfully loaded script in environment: " << *ScriptPath << '\n';
//
//		return true;
//	}
//	catch (const sol::error& e)
//	{
//		std::cerr << "Lua Error during script loading: " << *ScriptPath << "\nError: " << e.what() << '\n';
//		LuaScriptEnv = sol::nil;
//		return false;
//	}
//	catch (const std::exception& e)
//	{
//		// 표준 예외 처리
//		std::cerr << "알 수 없는 오류 발생: " << e.what() << '\n';
//		bIsScriptLoaded = false;
//		return false;
//	}
//	catch (...)
//	{
//		// 모든 다른 예외 처리
//		std::cerr << "알 수 없는 치명적인 오류 발생" << '\n';
//		bIsScriptLoaded = false;
//		return false;
//	}
//}

bool UScriptComponent::LoadScript(const FString& InScriptPath)
{
	ScriptPath = InScriptPath;
	bIsScriptLoaded = false;
	LuaScriptEnv = sol::nil;
	sol::state* Lua = GLuaManager.GetState();
	
	// 1) 소스만 꺼내기
	FString Src = GLuaManager.GetOrLoadScriptSource(InScriptPath);
	std::string src_string = std::string(Src);
	
	// 2) 컴파일 → 매번 다른 클로저 생성
	sol::load_result lr = Lua->load_buffer(
		src_string.data(), src_string.size(), std::string(InScriptPath)
	);

	if (!lr.valid()) 
	{
		std::cerr << "compile Error!\n";
	}
	sol::protected_function fn = lr;

	// 3) 환경 생성 & self 바인딩
	sol::environment env(*Lua, sol::create, Lua->globals());
	env["self"] = GetOwner();

	// 4) env 적용 & 실행
	env.set_on(fn);
	auto res = fn();
	if (!res.valid()) 
	{
	std::cerr << "compile Error!\n";
	}

	// 5) 성공
	LuaScriptEnv = env;
	bIsScriptLoaded = true;
	return true;
}


void UScriptComponent::OnOverlap(AActor* OtherActor)
{
	if (bIsScriptLoaded)
	{
		CallScriptFunction("OnOverlap", OtherActor);
	}
}

void UScriptComponent::CallScriptFunction(const char* FunctionName)
{
	if (!bIsScriptLoaded || !LuaScriptEnv.valid())
	{
		return;
	}

	try
	{
		const sol::protected_function Func = LuaScriptEnv[FunctionName];
		if (!Func.valid())
		{
			std::cerr << "Lua function not found in env: " << FunctionName << '\n';
			return;
		}

		const auto Result = Func();
		if (!Result.valid())
		{
			sol::error e = Result;
			std::cerr << "Lua function call error (" << FunctionName << "): " << e.what() << '\n';
		}
	}
	catch (const sol::error& e)
	{
		std::cerr << "Lua function call exception (" << FunctionName << "): " << e.what() << '\n';
	}
}

void UScriptComponent::CallScriptFunction(const char* FunctionName, float Value)
{
	if (!bIsScriptLoaded || !LuaScriptEnv.valid())
	{
		return;
	}

	try
	{
		const sol::protected_function Func = LuaScriptEnv[FunctionName];
		if (!Func.valid())
		{
			return;
		}

		const auto Result = Func(Value);
		if (!Result.valid())
		{
			const sol::error e = Result;
			std::cerr << "Lua function call error (" << FunctionName << "): " << e.what() << '\n';
		}
	}
	catch (const sol::error& e)
	{
		std::cerr << "Lua function call exception (" << FunctionName << "): " << e.what() << '\n';
	}
}

void UScriptComponent::CallScriptFunction(const char* FunctionName, AActor* OtherActor)
{
	if (!bIsScriptLoaded || !LuaScriptEnv.valid())
	{
		return;
	}

	try
	{
		const sol::protected_function Func = LuaScriptEnv[FunctionName];
		if (!Func.valid())
		{
			return;
		}

		const auto Result = Func(OtherActor);
		if (!Result.valid())
		{
			const sol::error e = Result;
			std::cerr << "Lua function call error (" << FunctionName << "): " << e.what() << '\n';
		}
	}
	catch (const sol::error& e)
	{
		std::cerr << "Lua function call exception (" << FunctionName << "): " << e.what() << '\n';
	}
}