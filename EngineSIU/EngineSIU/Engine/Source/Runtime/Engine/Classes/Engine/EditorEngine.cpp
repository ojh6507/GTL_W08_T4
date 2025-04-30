#include "EditorEngine.h"

#include "World/World.h"
#include "Level.h"
#include "Actors/Cube.h"
#include "Actors/DirectionalLightActor.h"
#include "GameFramework/Actor.h"
#include "Classes/Engine/AssetManager.h"
#include "Components/Light/DirectionalLightComponent.h"
#include "Physics/PhysicsSystem.h"

#include "Engine/Script/LuaBinding.h"
#include "Engine/Script/LuaManager.h"
#include "LevelEditor/SLevelEditor.h"
#include "UnrealEd/EditorViewportClient.h"

namespace PrivateEditorSelection
{
    static AActor* GActorSelected = nullptr;
    static AActor* GActorHovered = nullptr;

    static UActorComponent* GComponentSelected = nullptr;
    static UActorComponent* GComponentHovered = nullptr;
}

void UEditorEngine::Init()
{
    Super::Init();

    // Initialize the engine
    GEngine = this;

    FWorldContext& EditorWorldContext = CreateNewWorldContext(EWorldType::Editor);

    EditorWorld = UWorld::CreateWorld(this, EWorldType::Editor, FString("EditorWorld"));

    EditorWorldContext.SetCurrentWorld(EditorWorld);
    ActiveWorld = EditorWorld;

    EditorPlayer = FObjectFactory::ConstructObject<AEditorPlayer>(this);

    if (AssetManager == nullptr)
    {
        AssetManager = FObjectFactory::ConstructObject<UAssetManager>(this);
        assert(AssetManager);
        AssetManager->InitAssetManager();
    }
    if (!GLuaManager.Initialize())
    {
        std::cerr << "Lua 초기화 실패!" << std::endl;
    }
    LuaBindings::BindCoreTypesForLua(*GLuaManager.GetState());
    LuaBindings::BindInputForLua(*GLuaManager.GetState());

#ifdef _DEBUG
    //AActor* Actor = EditorWorld->SpawnActor<ACube>();
    //
    //ADirectionalLight* DirLight = EditorWorld->SpawnActor<ADirectionalLight>();
    //DirLight->SetActorRotation(FRotator(20, -61, 11));
    //DirLight->SetActorLocation(FVector(0, 0, 20));
    //DirLight->SetIntensity(2.f);
#endif
}

void UEditorEngine::Tick(float DeltaTime)
{
    for (FWorldContext* WorldContext : WorldList)
    {
        if (WorldContext->WorldType == EWorldType::Editor)
        {
            if (UWorld* World = WorldContext->World())
            {
                EditorPlayer->Tick(DeltaTime);
            }
        }
        else if (WorldContext->WorldType == EWorldType::PIE)
        {
            if (UWorld* World = WorldContext->World())
            {
                World->Tick(DeltaTime);
                ULevel* Level = World->GetActiveLevel();
                // 지금은 그냥 충돌 검사 후 Actor Tick
                FPhysicsSystem::Get().UpdateCollisions();
                
                TArray CachedActors = Level->Actors;
                if (Level)
                {
                    for (AActor* Actor : CachedActors)
                    {
                        if (Actor)
                        {
                            Actor->Tick(DeltaTime);
                        }
                    }
                }
            }
        }
    }
}

void UEditorEngine::StartPIE()
{
    if (PIEWorld)
    {
        UE_LOG(ELogLevel::Warning, TEXT("PIEWorld already exists!"));
        return;
    }

    FWorldContext& PIEWorldContext = CreateNewWorldContext(EWorldType::PIE);

    PIEWorld = Cast<UWorld>(EditorWorld->Duplicate(this));
    PIEWorld->WorldType = EWorldType::PIE;

    PIEWorldContext.SetCurrentWorld(PIEWorld);
    ActiveWorld = PIEWorld;
    
    PIEWorld->BeginPlay();
    // 여기서 Actor들의 BeginPlay를 해줄지 안에서 해줄 지 고민.
    WorldList.Add(GetWorldContextFromWorld(PIEWorld));
}

void UEditorEngine::EndPIE()
{
    if (PIEWorld)
    {
        //WorldList.Remove(*GetWorldContextFromWorld(PIEWorld.get()));
        WorldList.Remove(GetWorldContextFromWorld(PIEWorld));
        PIEWorld->Release();
        GUObjectArray.MarkRemoveObject(PIEWorld);
        PIEWorld = nullptr;

        // TODO: PIE에서 EditorWorld로 돌아올 때, 기존 선택된 Picking이 유지되어야 함. 현재는 에러를 막기위해 임시조치.
        SelectActor(nullptr);
        SelectComponent(nullptr);
        FPhysicsSystem::Get().Clear();
    }
    // 다시 EditorWorld로 돌아옴.
    ActiveWorld = EditorWorld;
}

FWorldContext& UEditorEngine::GetEditorWorldContext(/*bool bEnsureIsGWorld*/)
{
    for (FWorldContext* WorldContext : WorldList)
    {
        if (WorldContext->WorldType == EWorldType::Editor)
        {
            return *WorldContext;
        }
    }
    return CreateNewWorldContext(EWorldType::Editor);
}

FWorldContext* UEditorEngine::GetPIEWorldContext(/*int32 WorldPIEInstance*/)
{
    for (FWorldContext* WorldContext : WorldList)
    {
        if (WorldContext->WorldType == EWorldType::PIE)
        {
            return WorldContext;
        }
    }
    return nullptr;
}

void UEditorEngine::SelectActor(AActor* InActor) const
{
    // 선택된 Actor 변경 시, 선택된 Component를 Actor의 RootComponent로 변경
    if (InActor && CanSelectActor(InActor))
    {
        PrivateEditorSelection::GActorSelected = InActor;
        PrivateEditorSelection::GComponentSelected = InActor->GetRootComponent();
    }
    else if (InActor == nullptr)
    {
        PrivateEditorSelection::GActorSelected = nullptr;
        PrivateEditorSelection::GComponentSelected = nullptr;
    }
}

bool UEditorEngine::CanSelectActor(const AActor* InActor) const
{
    return InActor != nullptr && InActor->GetWorld() == ActiveWorld && !InActor->IsActorBeingDestroyed();
}

AActor* UEditorEngine::GetSelectedActor() const
{
    return PrivateEditorSelection::GActorSelected;
}

void UEditorEngine::HoverActor(AActor* InActor) const
{
    if (InActor)
    {
        PrivateEditorSelection::GActorHovered = InActor;
    }
    else if (InActor == nullptr)
    {
        PrivateEditorSelection::GActorHovered = nullptr;
    }
}

void UEditorEngine::NewWorld()
{
    SelectActor(nullptr);
    SelectComponent(nullptr);

    if (ActiveWorld->GetActiveLevel())
    {
        ActiveWorld->GetActiveLevel()->Release();
    }
}

void UEditorEngine::SelectComponent(UActorComponent* InComponent) const
{
    if (InComponent && CanSelectComponent(InComponent))
    {
        PrivateEditorSelection::GComponentSelected = InComponent;
    }
    else if (InComponent == nullptr)
    {
        PrivateEditorSelection::GComponentSelected = nullptr;
    }
}

bool UEditorEngine::CanSelectComponent(const UActorComponent* InComponent) const
{
    return InComponent != nullptr && InComponent->GetOwner() && InComponent->GetOwner()->GetWorld() == ActiveWorld && !InComponent->GetOwner()->IsActorBeingDestroyed();
}

UActorComponent* UEditorEngine::GetSelectedComponent() const
{
    return PrivateEditorSelection::GComponentSelected;
}

void UEditorEngine::HoverComponent(UActorComponent* InComponent) const
{
    if (InComponent)
    {
        PrivateEditorSelection::GComponentHovered = InComponent;
    }
    else if (InComponent == nullptr)
    {
        PrivateEditorSelection::GComponentHovered = nullptr;
    }
}

AEditorPlayer* UEditorEngine::GetEditorPlayer() const
{
    return EditorPlayer;
}
