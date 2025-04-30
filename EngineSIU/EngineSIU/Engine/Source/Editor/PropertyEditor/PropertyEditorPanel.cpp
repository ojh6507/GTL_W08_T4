#include "PropertyEditorPanel.h"

#include <filesystem>
#include <shellapi.h>

#include "World/World.h"
#include "Actors/Player.h"

#include "Components/Light/LightComponent.h"
#include "Components/Light/PointLightComponent.h"
#include "Components/Light/SpotLightComponent.h"
#include "Components/Light/DirectionalLightComponent.h"
#include "Components/ProjectileMovementComponent.h"
#include "Components/Light/AmbientLightComponent.h"
#include "Components/StaticMeshComponent.h"
#include "Components/HeightFogComponent.h"
#include "Components/TextComponent.h"
#include "Components/UScriptComponent.h"

#include "Engine/EditorEngine.h"
#include "Engine/FLoaderOBJ.h"
#include "UnrealEd/ImGuiWidget.h"
#include "UObject/Casts.h"
#include "UObject/ObjectFactory.h"
#include "Engine/Engine.h"
#include "GameFramework/Actor.h"
#include "Engine/AssetManager.h"
#include "UObject/UObjectIterator.h"

#include "Renderer/Shadow/SpotLightShadowMap.h"
#include "Renderer/Shadow/PointLightShadowMap.h"
#include "Renderer/Shadow/DirectionalShadowMap.h"

#include "GameFramework/SpringArmComponent.h"

#include "Components/UScriptComponent.h"
#include "Camera/CameraComponent.h"


void PropertyEditorPanel::Render()
{
    /* Pre Setup */
    float PanelWidth = (Width) * 0.2f - 6.0f;
    float PanelHeight = (Height) * 0.65f;

    float PanelPosX = (Width) * 0.8f + 5.0f;
    float PanelPosY = (Height) * 0.3f + 15.0f;

    ImVec2 MinSize(140, 370);
    ImVec2 MaxSize(FLT_MAX, 900);

    /* Min, Max Size */
    ImGui::SetNextWindowSizeConstraints(MinSize, MaxSize);

    /* Panel Position */
    ImGui::SetNextWindowPos(ImVec2(PanelPosX, PanelPosY), ImGuiCond_Always);

    /* Panel Size */
    ImGui::SetNextWindowSize(ImVec2(PanelWidth, PanelHeight), ImGuiCond_Always);

    /* Panel Flags */
    ImGuiWindowFlags PanelFlags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse;

    /* Render Start */
    ImGui::Begin("Detail", nullptr, PanelFlags);

    UEditorEngine* Engine = Cast<UEditorEngine>(GEngine);
    if (!Engine)
    {
        return;
    }
    AEditorPlayer* Player = Engine->GetEditorPlayer();
    SelectedActor = Engine->GetSelectedActor();
    SelectedComponent = Engine->GetSelectedComponent();

#pragma region Add Component
    if (SelectedActor)
    {
        ImGui::SetItemDefaultFocus();

        // @todo Outliner 및 Detail 패널 개편하기
        // @todo AddComponent 레이아웃 개편
        ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.1f, 0.1f, 0.1f, 1.0f));
        {
            TArray<UClass*> ActorComponentClasses;
            for (auto const& [ClassName, ClassPtr] : UClass::GetClassMap())
            {
                // Check if the class is derived from UActorComponent
                // and make sure it's not UActorComponent itself (usually you don't add the base directly)
                // Also ensure ClassPtr is valid.
                if (ClassPtr && ClassPtr != UActorComponent::StaticClass() && ClassPtr->IsChildOf(UActorComponent::StaticClass()))
                {
                    // @todo Abstract Class 체크
                    // if (!ClassPtr->IsAbstract())
                    // {
                    ActorComponentClasses.Add(ClassPtr);
                    // }
                }
            }

            // @todo 정상적으로 가져오는지 확인하기
            //GetChildOfClass(UActorComponent::StaticClass(), ActorComponentClasses);

            ImGui::SeparatorText("Add Component");

            ImGui::Text("Add");
            ImGui::SameLine();
            if (ImGui::BeginCombo("##AddComponent", "Components", ImGuiComboFlags_None))
            {
                for (UClass* Class : ActorComponentClasses)
                {
                    if (ImGui::Selectable(GetData(Class->GetName()), false))
                    {
                        // TODO: 임시로 static uint32 NewCompIndex사용
                        static uint32 NewCompIndex = 0;

                        // Add Component to Selected Actor
                        UActorComponent* NewComp = SelectedActor->AddComponent(Class, FString::Printf(TEXT("%s_%d"), *Class->GetName(), NewCompIndex++), true);

                        if (USceneComponent* ParentComp = Cast<USceneComponent>(SelectedComponent))
                        {
                            if (USceneComponent* ChildComp = Cast<USceneComponent>(NewComp))
                            {
                                ChildComp->SetupAttachment(ParentComp);
                            }
                        }
                        else
                        {
                            if (USceneComponent* ChildComp = Cast<USceneComponent>(NewComp))
                            {
                                ChildComp->SetupAttachment(SelectedActor->GetRootComponent());
                            }
                        }

                        bActorComponentNodeDirty = true;
                    }
                }
                ImGui::EndCombo();
            }
        }
        ImGui::PopStyleColor();
    }
#pragma endregion

#pragma region Delete Component
    if (SelectedActor)
    {
        ImGui::SeparatorText("Delete Component");

        float windowWidth = ImGui::GetContentRegionAvail().x;
        if (ImGui::Button("Delete", ImVec2(windowWidth, 32)))
        {
            if (SelectedComponent)
            {
                if (SelectedComponent != SelectedActor->GetRootComponent())
                {
                    SelectedComponent->DestroyComponent();
                    Engine->SelectComponent(nullptr);
                    bActorComponentNodeDirty = true;
                }
                else
                {
                    UE_LOG(ELogLevel::Warning, TEXT("Cannot Delete Root Component."));
                }
            }
            else
            {
                UE_LOG(ELogLevel::Warning, TEXT("No Component Selected."));
            }
        }
    }
#pragma endregion

#pragma region Generate Lua Script
    if (SelectedActor)
    {
        ImGui::SeparatorText("Lua Scripting");

        UScriptComponent* ScriptComp = SelectedActor->GetComponentByClass<UScriptComponent>();

        float WindowWidth = ImGui::GetContentRegionAvail().x;
        if (ScriptComp && !ScriptComp->GetScriptPath().IsEmpty())    // && 연산 순서에 유의 (ScriptPath가 nullptr일 수 있음
        {
            // Case 1: Has ScriptComponent and ScriptPath
            // => Edit Button (Open Script)
            if (ImGui::Button("Edit Script", ImVec2(WindowWidth, 0))) // 높이 0은 자동 높이
            {
                ShellExecuteOpen(std::filesystem::absolute(*ScriptComp->GetScriptPath()));
            }
        }
        else
        {
            // Case 2: No ScriptComponent or ScriptPath
            // => Add Button (Create and Open Script)
            if (ImGui::Button("Create Script", ImVec2(WindowWidth, 0)))
            {
                // 스크립트 이름 입력 팝업 열기
                ImGui::OpenPopup("Create New Lua Script");
            }

            // --- 스크립트 생성 모달 ---
            // 모달 창의 상태를 관리하기 위한 변수들
            static char ScriptNameBuffer[256] = { 0 };
            static std::string CreateErrorMsg;

            // 모달 창 정의
            if (ImGui::BeginPopupModal("Create New Lua Script", nullptr, ImGuiWindowFlags_AlwaysAutoResize))
            {
                ImGui::Text("Enter a name for the new Lua script:");
                ImGui::Spacing();

                // 스크립트 이름 입력 필드
                ImGui::PushItemWidth(300);
                if (ImGui::InputText("##ScriptName", ScriptNameBuffer, IM_ARRAYSIZE(ScriptNameBuffer), ImGuiInputTextFlags_EnterReturnsTrue))
                {
                    // 엔터 키로도 생성 시도 가능하게 하려면 여기에 'Create' 버튼 로직 복제 또는 함수화
                    ImGui::SetKeyboardFocusHere(-1); // 포커스 유지 방지
                }
                ImGui::PopItemWidth();
                ImGui::SameLine();
                ImGui::TextDisabled("(e.g., PlayerLogic, EnemyAI)");

                ImGui::Spacing();
                ImGui::Separator();
                ImGui::Spacing();

                // 오류 메시지 표시 영역
                if (!CreateErrorMsg.empty())
                {
                    ImGui::TextColored(ImVec4(1.0f, 0.0f, 0.0f, 1.0f), "Error: %s", CreateErrorMsg.c_str());
                    ImGui::Spacing();
                }

                if (ImGui::Button("Create", ImVec2(120, 0)))
                {
                    CreateErrorMsg = "";
                    if (strlen(ScriptNameBuffer) == 0)
                    {
                        CreateErrorMsg = "Script name cannot be empty.";
                    }
                    else
                    {
                        try
                        {
                            // 1. 파일명 및 경로 생성 (동적으로)
                            FString SceneName = GEngine->ActiveWorld->GetActiveLevel()->GetName();
                            FString ActorIdentifier = SelectedActor->GetName();
                            FString ScriptName(ScriptNameBuffer);

                            // 파일명 규칙: SceneName_ID_ActorName_ID_ScriptName.lua
                            FString FileName = SceneName + "_" + ActorIdentifier + "_" + ScriptName + ".lua";

                            std::filesystem::path OutputDir = "Contents/LuaScript"; // 예시 경로
                            std::filesystem::path TemplatePath = "Contents/LuaScript/template.lua"; // 예시 경로
                            std::filesystem::path OutputFilePath = (FString(OutputDir) + "/" + FileName).ToWideString();

                            // 2. 템플릿 파일 존재 확인
                            if (!std::filesystem::exists(TemplatePath))
                            {
                                throw std::runtime_error("Template file 'template.lua' not found at expected location.");
                            }

                            // 3. 출력 디렉토리 생성 (필요시)
                            if (!std::filesystem::exists(OutputDir))
                            {
                                if (!std::filesystem::create_directories(OutputDir))
                                {
                                    throw std::runtime_error("Failed to create output directory.");
                                }
                            }

                            // 4. 대상 파일이 이미 존재하는지 확인 (덮어쓰기 방지)
                            if (std::filesystem::exists(OutputFilePath))
                            {
                                throw std::runtime_error("A script with this name already exists for this actor/scene.");
                            }

                            // 5. 템플릿 파일 복사
                            std::filesystem::copy_file(TemplatePath, OutputFilePath); // copy_options 기본값은 덮어쓰기 안함

                            // 6. 성공 처리: 컴포넌트 추가 및 경로 설정
                            if (ScriptComp)
                            {
                                ScriptComp->SetScriptPath(OutputFilePath.string().c_str());
                                std::cout << "Successfully linked script: " << OutputFilePath.string() << '\n';

                                // 성공 후 모달 닫기 및 버퍼 초기화
                                ImGui::CloseCurrentPopup();
                                ScriptNameBuffer[0] = '\0';
                                CreateErrorMsg = "";

                                // Edit Script
                                ShellExecuteOpen(std::filesystem::absolute(*ScriptComp->GetScriptPath()));
                            }
                            else
                            {
                                UScriptComponent* NewScriptComp = SelectedActor->AddComponent<UScriptComponent>("ScriptComponent");
                                if (NewScriptComp)
                                {
                                    NewScriptComp->SetScriptPath(OutputFilePath.string().c_str());
                                    std::cout << "Successfully created and linked script: " << OutputFilePath.string() << '\n';

                                    // 성공 후 모달 닫기 및 버퍼 초기화
                                    ImGui::CloseCurrentPopup();
                                    ScriptNameBuffer[0] = '\0';
                                    CreateErrorMsg = "";

                                    // Edit Script
                                    ShellExecuteOpen(std::filesystem::absolute(*NewScriptComp->GetScriptPath()));

                                    bActorComponentNodeDirty = true;
                                }
                                else
                                {
                                    // 컴포넌트 추가 실패 시 파일은 생성되었으므로 롤백 또는 경고 필요
                                    std::filesystem::remove(OutputFilePath); // 생성된 파일 삭제 (롤백)
                                    throw std::runtime_error("Script file created, but failed to add UScriptComponent to the actor.");
                                }
                            }
                        }
                        catch (const std::filesystem::filesystem_error& FSErr)
                        {
                            CreateErrorMsg = "Filesystem error: ";
                            CreateErrorMsg += FSErr.what();
                            std::cerr << CreateErrorMsg << '\n';
                        }
                        catch (const std::exception& e)
                        {
                            CreateErrorMsg = e.what();
                            std::cerr << "Error: " << CreateErrorMsg << '\n';
                        }
                    }
                } // Button Create

                ImGui::SameLine();

                if (ImGui::Button("Cancel", ImVec2(120, 0)))
                {
                    // 취소 시 모달 닫기 및 버퍼/오류 초기화
                    ImGui::CloseCurrentPopup();
                    ScriptNameBuffer[0] = '\0';
                    CreateErrorMsg = "";
                }

                ImGui::EndPopup();
            } // if BeginPopupModal
        }

        // @todo AssetManager 갱신 로직 필요
        UAssetManager::Get().InitAssetManager();
        const TMap<FName, FAssetInfo> Assets = UAssetManager::Get().GetAssetRegistry();

        FString PreviewName = ScriptComp ? std::filesystem::path(*ScriptComp->GetScriptPath()).filename().string() : " ";
        if (ImGui::BeginCombo("##LuaScript", GetData(PreviewName), ImGuiComboFlags_None))
        {
            // @todo UScriptComponent 유무에 따른 더 나은 레이아웃이 필요
            if (ScriptComp)
            {
                if (ImGui::Selectable("None", false))
                {
                    // 빈 항목 선택 시 스크립트 경로를 비우거나 특별한 값으로 설정
                    ScriptComp->SetScriptPath("");
                }

                for (const auto& Asset : Assets)
                {
                    if (Asset.Value.AssetType == EAssetType::LuaScript)
                    {
                        if (ImGui::Selectable(GetData(Asset.Value.AssetName.ToString()), false))
                        {
                            FString LuaScriptName = Asset.Value.PackagePath.ToString() + "/" + Asset.Value.AssetName.ToString();
                            ScriptComp->SetScriptPath(LuaScriptName);
                        }
                    }
                }
            }
            else
            {
                SelectedActor->AddComponent<UScriptComponent>("ScriptComponent");
                bActorComponentNodeDirty = true;
            }

            ImGui::EndCombo();
        }
        ImGui::SameLine();
        ImGui::Text("Script File");
    }
#pragma endregion

#pragma region Actor Components
    if (SelectedActor)
    {
        ShowActorComponents("ActorComponentsTable");
    }
#pragma endregion

#pragma region Transform
    if (SelectedComponent && SelectedComponent->IsA<USceneComponent>())
    {
        ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.1f, 0.1f, 0.1f, 1.0f));
        if (ImGui::TreeNodeEx("Transform", ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_DefaultOpen))
        {
            USceneComponent* SceneComp = static_cast<USceneComponent*>(SelectedComponent);
            Location = SceneComp->GetRelativeLocation();
            Rotation = SceneComp->GetRelativeRotation();
            Scale = SceneComp->GetRelativeScale3D();
            FImGuiWidget::DrawVec3Control("Location", Location, 0, 85);
            ImGui::Spacing();
            FImGuiWidget::DrawRot3Control("Rotation", Rotation, 0, 85);
            ImGui::Spacing();
            FImGuiWidget::DrawVec3Control("Scale", Scale, 0, 85);
            ImGui::Spacing();
            SceneComp->SetRelativeLocation(Location);
            SceneComp->SetRelativeRotation(Rotation);
            SceneComp->SetRelativeScale3D(Scale);

            // 좌표 모드 버튼 라벨 설정 (중복 제거)
            FString CoordiButtonLabel;
            if (Player->GetCoordMode() == ECoordMode::CDM_WORLD)
            {
                CoordiButtonLabel = "World";
            }
            else if (Player->GetCoordMode() == ECoordMode::CDM_LOCAL)
            {
                CoordiButtonLabel = "Local";
            }

            float windowWidth = ImGui::GetContentRegionAvail().x;
            // 좌표 모드 버튼 (이전 중복 코드 제거)
            if (ImGui::Button(GetData(CoordiButtonLabel), ImVec2(windowWidth, 32)))
            {
                Player->AddCoordiMode();
            }

            ImGui::TreePop();
        }
        ImGui::PopStyleColor();
    }
#pragma endregion

#pragma region PointLight
    if (SelectedComponent && SelectedComponent->IsA<UPointLightComponent>())
    {
        UPointLightComponent* PointlightComp = static_cast<UPointLightComponent*>(SelectedComponent);
        int NumPointLights = 0;
        for (const auto iter : TObjectRange<UPointLightComponent>())
        {
            if (iter != PointlightComp)
            {
                NumPointLights++;
            }
            else
            {
                break;
            }
        }

        GEngineLoop.Renderer.PointLightShadowMapPass->RenderLinearDepth(NumPointLights, PointlightComp);

        // Shadow Depth Map 시각화
        TArray<ID3D11ShaderResourceView*> ShaderSRVs = GEngineLoop.Renderer.PointLightShadowMapPass->GetShadowViewSRVArray();
        TArray<FVector> Directions = GEngineLoop.Renderer.PointLightShadowMapPass->GetDirectionArray();
        TArray<FVector> Ups = GEngineLoop.Renderer.PointLightShadowMapPass->GetUpArray();

        ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.1f, 0.1f, 0.1f, 1.0f));
        if (ImGui::TreeNodeEx("PointLight Component", ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_DefaultOpen))
        {
            DrawColorProperty("Light Color",
                [&]() { return PointlightComp->GetLightColor(); },
                [&](FLinearColor c) { PointlightComp->SetLightColor(c); });

            float Intensity = PointlightComp->GetIntensity();
            if (ImGui::SliderFloat("Intensity", &Intensity, 0.0f, 160.0f, "%.1f"))
            {
                PointlightComp->SetIntensity(Intensity);
            }

            float Radius = PointlightComp->GetRadius();
            if (ImGui::SliderFloat("Radius", &Radius, 0.01f, 200.f, "%.1f"))
            {
                PointlightComp->SetRadius(Radius);
            }

            ImGui::Separator();

            // ─ Shadow Map 미리보기 (1열) ─
            ImGui::Text("Shadow Maps (6 faces):");

            const int   MapsCount = ShaderSRVs.Num();
            constexpr float ImgSize = 256.0f;

            for (int i = 0; i < MapsCount; ++i)
            {
                ImGui::Text("Direction %1.f %1.f %1.f", Directions[i].X, Directions[i].Y, Directions[i].Z);
                ImGui::Text("Up %1.f %1.f %1.f", Ups[i].X, Ups[i].Y, Ups[i].Z);
                ImTextureID TexID = reinterpret_cast<ImTextureID>(ShaderSRVs[i]);
                ImGui::Image(TexID, ImVec2(ImgSize, ImgSize));

                ImGui::Spacing();
            }

            ImGui::TreePop();
        }
        ImGui::PopStyleColor();
    }
#pragma endregion

#pragma region SpotLight
    if (SelectedComponent && SelectedComponent->IsA<USpotLightComponent>())
    {
        USpotLightComponent* SpotLightComp = static_cast<USpotLightComponent*>(SelectedComponent);
        int NumSpotLights = 0;
        for (const auto iter : TObjectRange<USpotLightComponent>())
        {
            if (iter->GetWorld() == GEngine->ActiveWorld)
            {
                if (iter != SpotLightComp)
                {
                    NumSpotLights++;
                }
                else
                {
                    break;
                }
            }
        }

        FEngineLoop::Renderer.SpotLightShadowMapPass->RenderLinearDepth(NumSpotLights);

        // Shadow Depth Map 시각화
        ID3D11ShaderResourceView* ShaderSRV = FEngineLoop::Renderer.SpotLightShadowMapPass->GetShadowDebugSRV();

        ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.1f, 0.1f, 0.1f, 1.0f));
        if (ImGui::TreeNodeEx("SpotLight Component", ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_DefaultOpen))
        {
            DrawColorProperty(
                "Light Color",
                [&] { return SpotLightComp->GetLightColor(); },
                [&](FLinearColor c) { SpotLightComp->SetLightColor(c); }
            );

            float Intensity = SpotLightComp->GetIntensity();
            if (ImGui::SliderFloat("Intensity", &Intensity, 0.0f, 5000.0f, "%.1f"))
            {
                SpotLightComp->SetIntensity(Intensity);
            }

            float Radius = SpotLightComp->GetRadius();
            if (ImGui::SliderFloat("Radius", &Radius, 0.01f, 200.f, "%.1f"))
            {
                SpotLightComp->SetRadius(Radius);
            }

            LightDirection = SpotLightComp->GetDirection();
            FImGuiWidget::DrawVec3Control("Direction", LightDirection, 0, 85);

            float OuterDegree = SpotLightComp->GetOuterDegree();
            float InnerDegree = SpotLightComp->GetInnerDegree();

            if (ImGui::SliderFloat("InnerDegree", &InnerDegree, 0.f, 90.0f, "%.1f"))
            {
                SpotLightComp->SetInnerDegree(InnerDegree);
                SpotLightComp->SetOuterDegree(
                    FMath::Max(InnerDegree, OuterDegree)
                );
            }

            if (ImGui::SliderFloat("OuterDegree", &OuterDegree, 0.f, 90.f, "%.1f"))
            {
                SpotLightComp->SetOuterDegree(OuterDegree);
                SpotLightComp->SetInnerDegree(
                    FMath::Min(OuterDegree, InnerDegree)
                );
            }

            ImGui::Separator();

            // ─ Shadow Map 미리보기 (1열) ─
            ImGui::Text("Testing SpotLight:");

            constexpr float ImgSize = 256.0f;

            ImGui::Text("Direction %.01f %.01f %.01f", LightDirection.X, LightDirection.Y, LightDirection.Z);
            ImTextureID TexID = reinterpret_cast<ImTextureID>(ShaderSRV);
            ImGui::Image(TexID, ImVec2(ImgSize, ImgSize));

            ImGui::Spacing();

            ImGui::TreePop();
        }
        ImGui::PopStyleColor();
    }
#pragma endregion

#pragma region DirectionalLight
    if (SelectedComponent && SelectedComponent->IsA<UDirectionalLightComponent>())
    {
        UDirectionalLightComponent* DirectionalLightComp = static_cast<UDirectionalLightComponent*>(SelectedComponent);

        // @todo Directional Light는 1개만 존재하도록 수정
        int NumDirectionalLight = 0;
        for (const auto iter : TObjectRange<UDirectionalLightComponent>())
        {
            if (iter->GetWorld() == GEngine->ActiveWorld)
            {
                if (iter != DirectionalLightComp)
                {
                    NumDirectionalLight++;
                }
                else
                {
                    break;
                }
            }
        }

        // Shadow Depth Map 시각화
        ID3D11ShaderResourceView* ShaderSRV = FEngineLoop::Renderer.DirectionalShadowMap->GetShadowViewSRV(0);

        ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.1f, 0.1f, 0.1f, 1.0f));
        if (ImGui::TreeNodeEx("DirectionalLight Component", ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_DefaultOpen))
        {
            DrawColorProperty("Light Color",
                [&]() { return DirectionalLightComp->GetLightColor(); },
                [&](FLinearColor c) { DirectionalLightComp->SetLightColor(c); });

            float Intensity = DirectionalLightComp->GetIntensity();
            if (ImGui::SliderFloat("Intensity", &Intensity, 0.0f, 150.0f, "%.1f"))
            {
                DirectionalLightComp->SetIntensity(Intensity);
            }

            LightDirection = DirectionalLightComp->GetDirection();
            FImGuiWidget::DrawVec3Control("Direction", LightDirection, 0, 85);

            // ─ Shadow Map 미리보기 (1열) ─
            ImGui::Separator();
            ImGui::Text("Directional Light Shadow SRV:");

            constexpr float ImgSize = 256.0f;

            ImGui::Text("Direction %.01f %.01f %.01f", LightDirection.X, LightDirection.Y, LightDirection.Z);
            ImTextureID TexID = reinterpret_cast<ImTextureID>(ShaderSRV);
            ImGui::Image(TexID, ImVec2(ImgSize, ImgSize));

            ImGui::Spacing();

            ImGui::TreePop();
        }
        ImGui::PopStyleColor();
    }
#pragma endregion

#pragma region AmbientLight
    if (SelectedComponent && SelectedComponent->IsA<UAmbientLightComponent>())
    {
        UAmbientLightComponent* AmbientLightComp = static_cast<UAmbientLightComponent*>(SelectedComponent);
        ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.1f, 0.1f, 0.1f, 1.0f));
        if (ImGui::TreeNodeEx("AmbientLight Component", ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_DefaultOpen))
        {
            DrawColorProperty("Light Color",
                [&]() { return AmbientLightComp->GetLightColor(); },
                [&](FLinearColor c) { AmbientLightComp->SetLightColor(c); });

            ImGui::TreePop();
        }
        ImGui::PopStyleColor();
    }
#pragma endregion

#pragma region ProjectileMovement
    if (SelectedComponent && SelectedComponent->IsA<UProjectileMovementComponent>())
    {
        UProjectileMovementComponent* ProjectileComp = static_cast<UProjectileMovementComponent*>(SelectedComponent);
        ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.1f, 0.1f, 0.1f, 1.0f));
        if (ImGui::TreeNodeEx("Projectile Movement Component", ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_DefaultOpen))
        {
            float InitialSpeed = ProjectileComp->GetInitialSpeed();
            if (ImGui::InputFloat("InitialSpeed", &InitialSpeed, 0.f, 10000.0f, "%.1f"))
            {
                ProjectileComp->SetInitialSpeed(InitialSpeed);
            }

            float MaxSpeed = ProjectileComp->GetMaxSpeed();
            if (ImGui::InputFloat("MaxSpeed", &MaxSpeed, 0.f, 10000.0f, "%.1f"))
            {
                ProjectileComp->SetMaxSpeed(MaxSpeed);
            }

            float Gravity = ProjectileComp->GetGravity();
            if (ImGui::InputFloat("Gravity", &Gravity, 0.f, 10000.f, "%.1f"))
            {
                ProjectileComp->SetGravity(Gravity);
            }

            float ProjectileLifetime = ProjectileComp->GetLifetime();
            if (ImGui::InputFloat("Lifetime", &ProjectileLifetime, 0.f, 10000.f, "%.1f"))
            {
                ProjectileComp->SetLifetime(ProjectileLifetime);
            }

            FVector CurrentVelocity = ProjectileComp->GetVelocity();

            float Velocity[3] = { CurrentVelocity.X, CurrentVelocity.Y, CurrentVelocity.Z };

            if (ImGui::InputFloat3("Velocity", Velocity, "%.1f"))
            {
                ProjectileComp->SetVelocity(FVector(Velocity[0], Velocity[1], Velocity[2]));
            }

            ImGui::TreePop();
        }
        ImGui::PopStyleColor();
    }
#pragma endregion

#pragma region Text
    if (SelectedComponent && SelectedComponent->IsA<UTextComponent>())
    {
        UTextComponent* TextComp = static_cast<UTextComponent*>(SelectedComponent);
        ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.1f, 0.1f, 0.1f, 1.0f));
        if (ImGui::TreeNodeEx("Text Component", ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_DefaultOpen)) // 트리 노드 생성
        {
            if (TextComp)
            {
                TextComp->SetTexture(L"Assets/Texture/font.png");
                TextComp->SetRowColumnCount(106, 106);
                FWString wText = TextComp->GetText();
                int Len = WideCharToMultiByte(CP_UTF8, 0, wText.c_str(), -1, nullptr, 0, nullptr, nullptr);
                std::string U8Text(Len, '\0');
                WideCharToMultiByte(CP_UTF8, 0, wText.c_str(), -1, U8Text.data(), Len, nullptr, nullptr);

                static char Buf[256];
                strcpy_s(Buf, U8Text.c_str());

                ImGui::Text("Text: ", Buf);
                ImGui::SameLine();
                ImGui::PushItemFlag(ImGuiItemFlags_NoNavDefaultFocus, true);
                if (ImGui::InputText("##Text", Buf, 256, ImGuiInputTextFlags_EnterReturnsTrue))
                {
                    TextComp->ClearText();
                    int wlen = MultiByteToWideChar(CP_UTF8, 0, Buf, -1, nullptr, 0);
                    FWString NewWText(wlen, L'\0');
                    MultiByteToWideChar(CP_UTF8, 0, Buf, -1, NewWText.data(), wlen);
                    TextComp->SetText(NewWText.c_str());
                }
                ImGui::PopItemFlag();
            }

            ImGui::TreePop();
        }
        ImGui::PopStyleColor();
    }
#pragma endregion

#pragma region StaticMesh
    if (SelectedComponent && SelectedComponent->IsA<UStaticMeshComponent>())
    {
        UStaticMeshComponent* StaticMeshComponent = static_cast<UStaticMeshComponent*>(SelectedComponent);
        RenderForStaticMesh(StaticMeshComponent);
        RenderForMaterial(StaticMeshComponent);
    }
#pragma endregion

#pragma region HeightFog
    if (SelectedComponent && SelectedComponent->IsA<UHeightFogComponent>())
    {
        UHeightFogComponent* FogComponent = static_cast<UHeightFogComponent*>(SelectedComponent);
        ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.1f, 0.1f, 0.1f, 1.0f));
        if (ImGui::TreeNodeEx("Exponential Height Fog", ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_DefaultOpen)) // 트리 노드 생성
        {
            FLinearColor CurrColor = FogComponent->GetFogColor();

            float R = CurrColor.R;
            float G = CurrColor.G;
            float B = CurrColor.B;
            float A = CurrColor.A;
            float H, S, V;
            float LightColor[4] = { R, G, B, A };

            // Fog Color
            ImGuiColorEditFlags Flag = ImGuiColorEditFlags_DisplayRGB
                | ImGuiColorEditFlags_NoSidePreview
                | ImGuiColorEditFlags_NoInputs
                | ImGuiColorEditFlags_Float;
            if (ImGui::ColorPicker4("##Fog Color", LightColor, Flag))
            {
                R = LightColor[0];
                G = LightColor[1];
                B = LightColor[2];
                A = LightColor[3];
                FogComponent->SetFogColor(FLinearColor(R, G, B, A));
            }
            RGBToHSV(R, G, B, H, S, V);
            // RGB/HSV
            bool ChangedRgb = false;
            bool ChangedHsv = false;

            // RGB
            ImGui::PushItemWidth(50.0f);
            if (ImGui::DragFloat("R##R", &R, 0.001f, 0.f, 1.f)) ChangedRgb = true;
            ImGui::SameLine();
            if (ImGui::DragFloat("G##G", &G, 0.001f, 0.f, 1.f)) ChangedRgb = true;
            ImGui::SameLine();
            if (ImGui::DragFloat("B##B", &B, 0.001f, 0.f, 1.f)) ChangedRgb = true;
            ImGui::Spacing();

            // HSV
            if (ImGui::DragFloat("H##H", &H, 0.1f, 0.f, 360)) ChangedHsv = true;
            ImGui::SameLine();
            if (ImGui::DragFloat("S##S", &S, 0.001f, 0.f, 1)) ChangedHsv = true;
            ImGui::SameLine();
            if (ImGui::DragFloat("V##V", &V, 0.001f, 0.f, 1)) ChangedHsv = true;
            ImGui::PopItemWidth();
            ImGui::Spacing();

            if (ChangedRgb && !ChangedHsv)
            {
                // RGB -> HSV
                RGBToHSV(R, G, B, H, S, V);
                FogComponent->SetFogColor(FLinearColor(R, G, B, A));
            }
            else if (ChangedHsv && !ChangedRgb)
            {
                // HSV -> RGB
                HSVToRGB(H, S, V, R, G, B);
                FogComponent->SetFogColor(FLinearColor(R, G, B, A));
            }

            float FogDensity = FogComponent->GetFogDensity();
            if (ImGui::SliderFloat("Density", &FogDensity, 0.00f, 3.0f))
            {
                FogComponent->SetFogDensity(FogDensity);
            }

            float FogDistanceWeight = FogComponent->GetFogDistanceWeight();
            if (ImGui::SliderFloat("Distance Weight", &FogDistanceWeight, 0.00f, 3.0f))
            {
                FogComponent->SetFogDistanceWeight(FogDistanceWeight);
            }

            float FogHeightFallOff = FogComponent->GetFogHeightFalloff();
            if (ImGui::SliderFloat("Height Fall Off", &FogHeightFallOff, 0.001f, 0.15f))
            {
                FogComponent->SetFogHeightFalloff(FogHeightFallOff);
            }

            float FogStartDistance = FogComponent->GetStartDistance();
            if (ImGui::SliderFloat("Start Distance", &FogStartDistance, 0.00f, 50.0f))
            {
                FogComponent->SetStartDistance(FogStartDistance);
            }

            float FogEndtDistance = FogComponent->GetEndDistance();
            if (ImGui::SliderFloat("End Distance", &FogEndtDistance, 0.00f, 50.0f))
            {
                FogComponent->SetEndDistance(FogEndtDistance);
            }

            ImGui::TreePop();
        }
        ImGui::PopStyleColor();
    }
#pragma endregion

#pragma region Camera
    if (SelectedComponent && SelectedComponent->IsA<UCameraComponent>())
    {
        UCameraComponent* cameraComponent = Cast<UCameraComponent>(SelectedComponent);
        RenderForCamera(cameraComponent);
    }
#pragma endregion

#pragma region Spring Arm
    if (SelectedComponent && SelectedComponent->IsA<USpringArmComponent>())
    {
        USpringArmComponent* SpringArmComponent = static_cast<USpringArmComponent*>(SelectedComponent);
        ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.1f, 0.1f, 0.1f, 1.0f));
        if (ImGui::TreeNodeEx("Spring Arm", ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_DefaultOpen))
        {
            if (ImGui::TreeNodeEx("Camera", ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_DefaultOpen))
            {
                float TargetArmLength = SpringArmComponent->GetTargetArmLength();
                if (ImGui::SliderFloat("Target Arm Length", &TargetArmLength, -100.0f, 400.0f))
                {
                    SpringArmComponent->SetTargetArmLength(TargetArmLength);
                }

                //float SocketOffset[3] = { SpringArmComponent->GetSocketOffset().X, SpringArmComponent->GetSocketOffset().Y, SpringArmComponent->GetSocketOffset().Z };
                //if (ImGui::InputFloat3("Socket Offset", SocketOffset, "%.1f"))
                //{
                //    SpringArmComponent->SetSocketOffset(FVector(SocketOffset[0], SocketOffset[1], SocketOffset[2]));
                //}

                float TargetOffset[3] = { SpringArmComponent->TargetOffset.X, SpringArmComponent->TargetOffset.Y, SpringArmComponent->TargetOffset.Z };
                if (ImGui::InputFloat3("Target Offset", TargetOffset, "%.1f"))
                {
                    SpringArmComponent->TargetOffset = FVector(TargetOffset[0], TargetOffset[1], TargetOffset[2]);
                }

                ImGui::TreePop();
            }

            if (ImGui::TreeNodeEx("Camera Collision", ImGuiTreeNodeFlags_Framed))
            {
                ImGui::BeginDisabled(!SpringArmComponent->bDoCollisionTest);
                {
                    float ProbeSize = SpringArmComponent->ProbeSize;
                    if (ImGui::SliderFloat("Probe Size", &ProbeSize, 0.0f, 100.0f))
                    {
                        SpringArmComponent->ProbeSize = ProbeSize;
                    }

                    //float ProbeChannel = SpringArmComponent->ProbeChannel;
                    //if (ImGui::SliderFloat("Probe Channel", &ProbeChannel, 0.0f, 100.0f))
                    //{
                    //    SpringArmComponent->ProbeChannel = ProbeChannel;
                    //}
                }
                ImGui::EndDisabled();

                bool bDoCollisionTest = SpringArmComponent->bDoCollisionTest;
                if (ImGui::Checkbox("Do Collision Test", &bDoCollisionTest))
                {
                    SpringArmComponent->bDoCollisionTest = bDoCollisionTest;
                }

                ImGui::TreePop();
            }

            if (ImGui::TreeNodeEx("Camera Setting", ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_DefaultOpen))
            {
                bool bUsePawnControlRotation = SpringArmComponent->bUsePawnControlRotation;
                // @todo Pawn 구현 후 이름 수정하기
                if (ImGui::Checkbox("Use Actor Rotation", &bUsePawnControlRotation))
                {
                    SpringArmComponent->bUsePawnControlRotation = bUsePawnControlRotation;
                }

                {
                    bool bInheritPitch = SpringArmComponent->bInheritPitch;
                    if (ImGui::Checkbox("Inherit Pitch", &bInheritPitch))
                    {
                        SpringArmComponent->bInheritPitch = bInheritPitch;
                    }

                    bool bInheritYaw = SpringArmComponent->bInheritYaw;
                    if (ImGui::Checkbox("Inherit Yaw", &bInheritYaw))
                    {
                        SpringArmComponent->bInheritYaw = bInheritYaw;
                    }

                    bool bInheritRoll = SpringArmComponent->bInheritRoll;
                    if (ImGui::Checkbox("Inherit Roll", &bInheritRoll))
                    {
                        SpringArmComponent->bInheritRoll = bInheritRoll;
                    }
                }

                ImGui::TreePop();
            }

            if (ImGui::TreeNodeEx("Lag", ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_DefaultOpen))
            {
                bool bEnableCameraLag = SpringArmComponent->bEnableCameraLag;
                if (ImGui::Checkbox("Enable Camera Lag", &bEnableCameraLag))
                {
                    SpringArmComponent->bEnableCameraLag = bEnableCameraLag;
                }

                bool bEnableCameraRotationLag = SpringArmComponent->bEnableCameraRotationLag;
                if (ImGui::Checkbox("Enable Camera Rotation Lag", &bEnableCameraRotationLag))
                {
                    SpringArmComponent->bEnableCameraRotationLag = bEnableCameraRotationLag;
                }

                ImGui::BeginDisabled(!bEnableCameraLag);
                {
                    float CameraLagSpeed = SpringArmComponent->CameraLagSpeed;
                    if (ImGui::SliderFloat("Camera Lag Speed", &CameraLagSpeed, 0.0f, 1000.0f))
                    {
                        SpringArmComponent->CameraLagSpeed = CameraLagSpeed;
                    }
                }
                ImGui::EndDisabled();

                ImGui::BeginDisabled(!bEnableCameraRotationLag);
                {
                    float CameraRotationLagSpeed = SpringArmComponent->CameraRotationLagSpeed;
                    if (ImGui::SliderFloat("Camera Rotation Lag Speed", &CameraRotationLagSpeed, 0.0f, 1000.0f))
                    {
                        SpringArmComponent->CameraRotationLagSpeed = CameraRotationLagSpeed;
                    }
                }
                ImGui::EndDisabled();

                ImGui::BeginDisabled(!bEnableCameraLag);
                {
                    float CameraLagMaxDistance = SpringArmComponent->CameraLagMaxDistance;
                    if (ImGui::SliderFloat("Camera Lag Max Distance", &CameraLagMaxDistance, 0.0f, 1000.0f))
                    {
                        SpringArmComponent->CameraLagMaxDistance = CameraLagMaxDistance;
                    }
                }
                ImGui::EndDisabled();

                if (ImGui::TreeNodeEx("Advance", ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_DefaultOpen))
                {
                    bool bUseCameraLagSubstepping = SpringArmComponent->bUseCameraLagSubstepping;
                    if (ImGui::Checkbox("Use Camera Lag Substepping", &bUseCameraLagSubstepping))
                    {
                        SpringArmComponent->bUseCameraLagSubstepping = bUseCameraLagSubstepping;
                    }

                    ImGui::BeginDisabled(!bUseCameraLagSubstepping);
                    {
                        float CameraLagMaxTimeStep = SpringArmComponent->CameraLagMaxTimeStep;
                        if (ImGui::SliderFloat("Camera Lag Max TimeStep", &CameraLagMaxTimeStep, 0.005f, 0.5f))
                        {
                            SpringArmComponent->CameraLagMaxTimeStep = CameraLagMaxTimeStep;
                        }
                    }
                    ImGui::EndDisabled();

                    ImGui::TreePop();
                }

                ImGui::TreePop();
            }

            ImGui::TreePop();
        }
        ImGui::PopStyleColor();
    }
#pragma endregion

    SelectedActorPrev = SelectedActor;
    SelectedComponentPrev = SelectedComponent;

    ImGui::End();
}

void PropertyEditorPanel::RGBToHSV(float r, float g, float b, float& h, float& s, float& v) const
{
    float mx = FMath::Max(r, FMath::Max(g, b));
    float mn = FMath::Min(r, FMath::Min(g, b));
    float delta = mx - mn;

    v = mx;

    if (mx == 0.0f) {
        s = 0.0f;
        h = 0.0f;
        return;
    }
    else {
        s = delta / mx;
    }

    if (delta < 1e-6) {
        h = 0.0f;
    }
    else {
        if (r >= mx) {
            h = (g - b) / delta;
        }
        else if (g >= mx) {
            h = 2.0f + (b - r) / delta;
        }
        else {
            h = 4.0f + (r - g) / delta;
        }
        h *= 60.0f;
        if (h < 0.0f) {
            h += 360.0f;
        }
    }
}

void PropertyEditorPanel::HSVToRGB(float h, float s, float v, float& r, float& g, float& b) const
{
    // h: 0~360, s:0~1, v:0~1
    float c = v * s;
    float hp = h / 60.0f;             // 0~6 구간
    float x = c * (1.0f - fabsf(fmodf(hp, 2.0f) - 1.0f));
    float m = v - c;

    if (hp < 1.0f) { r = c;  g = x;  b = 0.0f; }
    else if (hp < 2.0f) { r = x;  g = c;  b = 0.0f; }
    else if (hp < 3.0f) { r = 0.0f; g = c;  b = x; }
    else if (hp < 4.0f) { r = 0.0f; g = x;  b = c; }
    else if (hp < 5.0f) { r = x;  g = 0.0f; b = c; }
    else { r = c;  g = 0.0f; b = x; }

    r += m;  g += m;  b += m;
}

void PropertyEditorPanel::RenderForStaticMesh(UStaticMeshComponent* StaticMeshComp) const
{
    ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.1f, 0.1f, 0.1f, 1.0f));
    if (ImGui::TreeNodeEx("Static Mesh", ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_DefaultOpen)) // 트리 노드 생성
    {
        ImGui::Text("StaticMesh");
        ImGui::SameLine();

        const TMap<FName, FAssetInfo> Assets = UAssetManager::Get().GetAssetRegistry();

        FString PreviewName = StaticMeshComp->GetStaticMesh() ? StaticMeshComp->GetStaticMesh()->GetRenderData()->DisplayName : "None";
        if (ImGui::BeginCombo("##StaticMesh", GetData(PreviewName), ImGuiComboFlags_None))
        {
            for (const auto& Asset : Assets)
            {
                if (Asset.Value.AssetType == EAssetType::StaticMesh)
                {
                    if (ImGui::Selectable(GetData(Asset.Value.AssetName.ToString()), false))
                    {
                        FString MeshName = Asset.Value.PackagePath.ToString() + "/" + Asset.Value.AssetName.ToString();
                        UStaticMesh* StaticMesh = FManagerOBJ::GetStaticMesh(MeshName.ToWideString());
                        if (StaticMesh)
                        {
                            StaticMeshComp->SetStaticMesh(StaticMesh);
                        }
                    }
                }
            }
            ImGui::EndCombo();
        }

        ImGui::TreePop();
    }
    ImGui::PopStyleColor();
}

void PropertyEditorPanel::RenderForMaterial(UStaticMeshComponent* StaticMeshComp)
{
    ImGui::PushStyleColor(ImGuiCol_Header, ImVec4(0.1f, 0.1f, 0.1f, 1.0f));
    if (ImGui::TreeNodeEx("Materials", ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_DefaultOpen)) // 트리 노드 생성
    {
        for (uint32 i = 0; i < StaticMeshComp->GetNumMaterials(); ++i)
        {
            if (ImGui::Selectable(GetData(StaticMeshComp->GetMaterialSlotNames()[i].ToString()), false, ImGuiSelectableFlags_AllowDoubleClick))
            {
                if (ImGui::IsMouseDoubleClicked(0))
                {
                    std::cout << GetData(StaticMeshComp->GetMaterialSlotNames()[i].ToString()) << std::endl;
                    SelectedMaterialIndex = i;
                    SelectedStaticMeshComp = StaticMeshComp;
                }
            }
        }

        if (ImGui::Button("    +    ")) {
            IsCreateMaterial = true;
        }

        ImGui::TreePop();
    }

    if (StaticMeshComp->GetStaticMesh() && ImGui::TreeNodeEx("SubMeshes", ImGuiTreeNodeFlags_Framed | ImGuiTreeNodeFlags_DefaultOpen)) // 트리 노드 생성
    {
        const auto Subsets = StaticMeshComp->GetStaticMesh()->GetRenderData()->MaterialSubsets;
        for (uint32 i = 0; i < Subsets.Num(); ++i)
        {
            FString Temp = "subset " + std::to_string(i);
            if (ImGui::Selectable(GetData(Temp), false, ImGuiSelectableFlags_AllowDoubleClick))
            {
                if (ImGui::IsMouseDoubleClicked(0))
                {
                    StaticMeshComp->SetselectedSubMeshIndex(i);
                    SelectedStaticMeshComp = StaticMeshComp;
                }
            }
        }
        FString Temp = "clear subset";
        if (ImGui::Selectable(GetData(Temp), false, ImGuiSelectableFlags_AllowDoubleClick))
        {
            if (ImGui::IsMouseDoubleClicked(0))
                StaticMeshComp->SetselectedSubMeshIndex(-1);
        }

        ImGui::TreePop();
    }
    ImGui::PopStyleColor();

    if (SelectedMaterialIndex != -1)
    {
        RenderMaterialView(SelectedStaticMeshComp->GetMaterial(SelectedMaterialIndex));
    }
    if (IsCreateMaterial)
    {
        RenderCreateMaterialView();
    }
}

void PropertyEditorPanel::RenderMaterialView(UMaterial* Material)
{
    ImGui::SetNextWindowSize(ImVec2(380, 400), ImGuiCond_Once);
    ImGui::Begin("Material Viewer", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoNav);

    static ImGuiSelectableFlags BaseFlag = ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel | ImGuiColorEditFlags_None | ImGuiColorEditFlags_NoAlpha;

    const FVector MatDiffuseColor = Material->GetMaterialInfo().Diffuse;
    const FVector MatSpecularColor = Material->GetMaterialInfo().Specular;
    const FVector MatAmbientColor = Material->GetMaterialInfo().Ambient;
    const FVector MatEmissiveColor = Material->GetMaterialInfo().Emissive;

    float dr = MatDiffuseColor.X;
    float dg = MatDiffuseColor.Y;
    float db = MatDiffuseColor.Z;
    float da = 1.0f;
    float DiffuseColorPick[4] = { dr, dg, db, da };

    ImGui::Text("Material Name |");
    ImGui::SameLine();
    ImGui::Text(GetData(Material->GetMaterialInfo().MaterialName));
    ImGui::Separator();

    ImGui::Text("  Diffuse Color");
    ImGui::SameLine();
    if (ImGui::ColorEdit4("Diffuse##Color", reinterpret_cast<float*>(&DiffuseColorPick), BaseFlag))
    {
        const FVector NewColor = { DiffuseColorPick[0], DiffuseColorPick[1], DiffuseColorPick[2] };
        Material->SetDiffuse(NewColor);
    }

    float sr = MatSpecularColor.X;
    float sg = MatSpecularColor.Y;
    float sb = MatSpecularColor.Z;
    float sa = 1.0f;
    float SpecularColorPick[4] = { sr, sg, sb, sa };

    ImGui::Text("Specular Color");
    ImGui::SameLine();
    if (ImGui::ColorEdit4("Specular##Color", reinterpret_cast<float*>(&SpecularColorPick), BaseFlag))
    {
        const FVector NewColor = { SpecularColorPick[0], SpecularColorPick[1], SpecularColorPick[2] };
        Material->SetSpecular(NewColor);
    }

    float ar = MatAmbientColor.X;
    float ag = MatAmbientColor.Y;
    float ab = MatAmbientColor.Z;
    float aa = 1.0f;
    float AmbientColorPick[4] = { ar, ag, ab, aa };

    ImGui::Text("Ambient Color");
    ImGui::SameLine();
    if (ImGui::ColorEdit4("Ambient##Color", reinterpret_cast<float*>(&AmbientColorPick), BaseFlag))
    {
        const FVector NewColor = { AmbientColorPick[0], AmbientColorPick[1], AmbientColorPick[2] };
        Material->SetAmbient(NewColor);
    }

    float er = MatEmissiveColor.X;
    float eg = MatEmissiveColor.Y;
    float eb = MatEmissiveColor.Z;
    float ea = 1.0f;
    float EmissiveColorPick[4] = { er, eg, eb, ea };

    ImGui::Text("Emissive Color");
    ImGui::SameLine();
    if (ImGui::ColorEdit4("Emissive##Color", reinterpret_cast<float*>(&EmissiveColorPick), BaseFlag))
    {
        const FVector NewColor = { EmissiveColorPick[0], EmissiveColorPick[1], EmissiveColorPick[2] };
        Material->SetEmissive(NewColor);
    }

    ImGui::Spacing();
    ImGui::Separator();

    ImGui::Text("Choose Material");
    ImGui::Spacing();

    ImGui::Text("Material Slot Name |");
    ImGui::SameLine();
    ImGui::Text(GetData(SelectedStaticMeshComp->GetMaterialSlotNames()[SelectedMaterialIndex].ToString()));

    ImGui::Text("Override Material |");
    ImGui::SameLine();
    ImGui::SetNextItemWidth(160);
    // 메테리얼 이름 목록을 const char* 배열로 변환
    std::vector<const char*> MaterialChars;
    for (const auto& Material : FManagerOBJ::GetMaterials()) {
        MaterialChars.push_back(*Material.Value->GetMaterialInfo().MaterialName);
    }

    //// 드롭다운 표시 (currentMaterialIndex가 범위를 벗어나지 않도록 확인)
    //if (currentMaterialIndex >= FManagerOBJ::GetMaterialNum())
    //    currentMaterialIndex = 0;

    if (ImGui::Combo("##MaterialDropdown", &CurMaterialIndex, MaterialChars.data(), FManagerOBJ::GetMaterialNum())) {
        UMaterial* Material = FManagerOBJ::GetMaterial(MaterialChars[CurMaterialIndex]);
        SelectedStaticMeshComp->SetMaterial(SelectedMaterialIndex, Material);
    }

    if (ImGui::Button("Close"))
    {
        SelectedMaterialIndex = -1;
        SelectedStaticMeshComp = nullptr;
    }

    ImGui::End();
}

void PropertyEditorPanel::RenderCreateMaterialView()
{
    ImGui::SetNextWindowSize(ImVec2(300, 500), ImGuiCond_Once);
    ImGui::Begin("Create Material Viewer", nullptr, ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoNav);

    static ImGuiSelectableFlags BaseFlag = ImGuiColorEditFlags_NoInputs | ImGuiColorEditFlags_NoLabel | ImGuiColorEditFlags_None | ImGuiColorEditFlags_NoAlpha;

    ImGui::Text("New Name");
    ImGui::SameLine();
    static char MaterialName[256] = "New Material";
    // 기본 텍스트 입력 필드
    ImGui::SetNextItemWidth(128);
    if (ImGui::InputText("##NewName", MaterialName, IM_ARRAYSIZE(MaterialName)))
    {
        tempMaterialInfo.MaterialName = MaterialName;
    }

    const FVector MatDiffuseColor = tempMaterialInfo.Diffuse;
    const FVector MatSpecularColor = tempMaterialInfo.Specular;
    const FVector MatAmbientColor = tempMaterialInfo.Ambient;
    const FVector MatEmissiveColor = tempMaterialInfo.Emissive;

    float dr = MatDiffuseColor.X;
    float dg = MatDiffuseColor.Y;
    float db = MatDiffuseColor.Z;
    float da = 1.0f;
    float DiffuseColorPick[4] = { dr, dg, db, da };

    ImGui::Text("Set Property");
    ImGui::Indent();

    ImGui::Text("  Diffuse Color");
    ImGui::SameLine();
    if (ImGui::ColorEdit4("Diffuse##Color", reinterpret_cast<float*>(&DiffuseColorPick), BaseFlag))
    {
        FVector NewColor = { DiffuseColorPick[0], DiffuseColorPick[1], DiffuseColorPick[2] };
        tempMaterialInfo.Diffuse = NewColor;
    }

    float sr = MatSpecularColor.X;
    float sg = MatSpecularColor.Y;
    float sb = MatSpecularColor.Z;
    float sa = 1.0f;
    float SpecularColorPick[4] = { sr, sg, sb, sa };

    ImGui::Text("Specular Color");
    ImGui::SameLine();
    if (ImGui::ColorEdit4("Specular##Color", reinterpret_cast<float*>(&SpecularColorPick), BaseFlag))
    {
        const FVector NewColor = { SpecularColorPick[0], SpecularColorPick[1], SpecularColorPick[2] };
        tempMaterialInfo.Specular = NewColor;
    }

    float ar = MatAmbientColor.X;
    float ag = MatAmbientColor.Y;
    float ab = MatAmbientColor.Z;
    float aa = 1.0f;
    float AmbientColorPick[4] = { ar, ag, ab, aa };

    ImGui::Text("Ambient Color");
    ImGui::SameLine();
    if (ImGui::ColorEdit4("Ambient##Color", reinterpret_cast<float*>(&AmbientColorPick), BaseFlag))
    {
        const FVector NewColor = { AmbientColorPick[0], AmbientColorPick[1], AmbientColorPick[2] };
        tempMaterialInfo.Ambient = NewColor;
    }

    float er = MatEmissiveColor.X;
    float eg = MatEmissiveColor.Y;
    float eb = MatEmissiveColor.Z;
    float ea = 1.0f;
    float EmissiveColorPick[4] = { er, eg, eb, ea };

    ImGui::Text("Emissive Color");
    ImGui::SameLine();
    if (ImGui::ColorEdit4("Emissive##Color", reinterpret_cast<float*>(&EmissiveColorPick), BaseFlag))
    {
        const FVector NewColor = { EmissiveColorPick[0], EmissiveColorPick[1], EmissiveColorPick[2] };
        tempMaterialInfo.Emissive = NewColor;
    }
    ImGui::Unindent();

    ImGui::NewLine();
    if (ImGui::Button("Create Material")) {
        FManagerOBJ::CreateMaterial(tempMaterialInfo);
    }

    ImGui::NewLine();
    if (ImGui::Button("Close"))
    {
        IsCreateMaterial = false;
    }

    ImGui::End();
}

void PropertyEditorPanel::RenderForCamera(UCameraComponent* CameraComp)
{
        if (!CameraComp)
        return;

    if (ImGui::Begin("Camera Component Settings"))
    {
        // Camera Name 편집
        {
            // 버퍼 준비
            char buf[128];
            std::string nameStr = *CameraComp->CameraName;
            strncpy_s(buf, sizeof(buf), nameStr.c_str(), _TRUNCATE);
            buf[sizeof(buf)-1];
            if (ImGui::InputText("Camera Name", buf, sizeof(buf)))
            {
                CameraComp->CameraName = FString(buf);
            }
        }
            
        // FOV
        float fov = CameraComp->GetVeiwFovDegrees();
        if (ImGui::DragFloat("View FOV (deg)", &fov, 0.1f, 1.0f, 179.0f))
        {
            CameraComp->SetViewFovDegrees(fov);
        }

        // 클리핑
        float nearClip = CameraComp->GetNearClip();
        if (ImGui::DragFloat("Near Clip", &nearClip, 0.01f, 0.01f, 1000.0f))
        {
            CameraComp->SetNearClip(nearClip);
        }
        float farClip = CameraComp->GetFarClip();
        if (ImGui::DragFloat("Far Clip", &farClip, 1.0f, 1.0f, 10000.0f))
        {
            CameraComp->SetFarClip(farClip);
        }


        // Projection Mode
        //const char* modes[] = { "Perspective", "Orthographic" };
        //int pm = static_cast<int>(CameraComp->ProjectionMode);
        //if (ImGui::Combo("Projection Mode", &pm, modes, IM_ARRAYSIZE(modes)))
        //{
            //CameraComp->ProjectionMode = static_cast<ECameraProjectionMode::Type>(pm);
        //}

        ImGui::End();
    }
}

void PropertyEditorPanel::ShellExecuteOpen(const std::filesystem::path& FilePath)
{
    if (!std::filesystem::exists(FilePath))
    {
        MessageBox(nullptr, (L"존재하지 않는 파일."), (L"오류"), MB_OK | MB_ICONWARNING);
        return;
    }

    // ShellExecute는 TCHAR* 를 인자로 받으므로, 프로젝트 설정에 따라 변환 필요
    LPCTSTR lpPath = FilePath.c_str();

    HINSTANCE hInst = ShellExecute(
        nullptr,        // 부모 윈도우 핸들
        L"open",        // 동작(verb.): ".lua" 확장자에 연결된 기본 프로그램으로 열기
        lpPath,         // 열고자 하는 파일의 전체 경로
        nullptr,        // 추가 실행 인자 (보통 필요 없음)
        nullptr,        // 작업 디렉토리 (보통 NULL)
        SW_SHOWNORMAL   // 창 표시 방법
    );

    // ShellExecute 반환 값 확인 (32 이하면 오류)
    if (reinterpret_cast<INT_PTR>(hInst) <= 32)
    {
        // 오류 코드에 따라 더 자세한 메시지 표시 가능 (FormatMessage 등 사용)
        MessageBox(nullptr, (L"파일을 여는 데 실패했습니다. 파일 경로와 기본 연결 프로그램을 확인하세요."), (L"오류"), MB_OK | MB_ICONERROR);
    }
}

void PropertyEditorPanel::OnResize(HWND hWnd)
{
    RECT ClientRect;
    GetClientRect(hWnd, &ClientRect);
    Width = ClientRect.right - ClientRect.left;
    Height = ClientRect.bottom - ClientRect.top;
}

void PropertyEditorPanel::RenderActorComponentTreeNode(ActorComponentNode* Node)
{
    if (!Node)
    {
        return;
    }

    ImGui::TableNextRow(); // 새 행 시작

    // --- 1열: 트리 노드 ---
    ImGui::TableNextColumn(); // 첫 번째 열로 이동

    // TreeNodeEx 플래그 설정
    ImGuiTreeNodeFlags Flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_SpanFullWidth;
    if (Node->Children.IsEmpty())
    {
        // 자식이 없으면 Leaf 노드
        Flags |= ImGuiTreeNodeFlags_Leaf;// | ImGuiTreeNodeFlags_Bullet;
    }
    if (Node->Component == SelectedComponent)
    {
        Flags |= ImGuiTreeNodeFlags_Selected;
    }

    // bool bIsOpen = ImGui::TreeNodeEx((void*)Node, Flags, "%s", *Node->Name);
    bool bIsOpen = ImGui::TreeNodeEx(*Node->Name, Flags);
    if (ImGui::IsItemFocused())
    {
        Cast<UEditorEngine>(GEngine)->SelectComponent(Node->Component);
    }

    // --- 2열: 타입 ---
    ImGui::TableNextColumn(); // 두 번째 열로 이동
    ImGui::TextUnformatted(*Node->Type);

    // --- 3열: 기타 ---
    // ... 필요시 추가 (ex: ScriptCompoent의 경우 스크립트 편집 버튼 등)

    // 노드가 열려 있고 자식이 있다면, 자식 노드들을 재귀적으로 렌더링
    if (bIsOpen)
    {
        for (const auto& Child : Node->Children)
        {
            RenderActorComponentTreeNode(Child);
        }
        ImGui::TreePop(); // TreeNodeEx와 쌍을 맞춰 TreePop 호출
    }
}

void PropertyEditorPanel::RenderActorComponentTreeNodeTable(const char* TableID, ActorComponentNode* RootNode)
{
    ImGuiTableFlags Flags = ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_Resizable | ImGuiTableFlags_ScrollY;
    constexpr int NumColumns = 2; // 열 개수

    if (ImGui::BeginTable(TableID, NumColumns, Flags)) {
        // --- 테이블 헤더 설정 ---
        // ImGuiTableColumnFlags_WidthStretch: 남은 공간을 채움
        // ImGuiTableColumnFlags_WidthFixed: 고정 너비
        ImGui::TableSetupColumn("Name", ImGuiTableColumnFlags_WidthStretch);
        ImGui::TableSetupColumn("Type", ImGuiTableColumnFlags_WidthFixed, 80.0f);
        // @note 기타 열 추가 시 추가 바람
        ImGui::TableHeadersRow(); // 헤더 행 그리기

        // --- 테이블 내용 (트리 구조) 렌더링 ---
        RenderActorComponentTreeNode(RootNode);

        ImGui::EndTable(); // 테이블 종료
    }
}

void PropertyEditorPanel::ShowActorComponents(const char* TableID)
{
    if (SelectedActor != SelectedActorPrev)
    {
        RootActorNode = new ActorComponentNode(SelectedActor->GetName(), SelectedActor->GetClass()->GetName(), SelectedActor->GetRootComponent());  // Actor의 경우 RootComponent로 연결
        bActorComponentNodeDirty = true;
    }

    if (bActorComponentNodeDirty)
    {
        RootActorNode->ClearChildren();
        // @todo RootComponent가 없는 경우에 대하여 처리 필요, 임시로 nullptr 체크로 대체
        if (SelectedActor->GetRootComponent())
        {
            ActorComponentNode* RootComponentNode = RootActorNode->AddChild(SelectedActor->GetRootComponent()->GetName(), SelectedActor->GetRootComponent()->GetClass()->GetName(), SelectedActor->GetRootComponent());

            // Recursive function to add children of USceneComponent (i.e. RootComponent)
            auto AddSceneComponentChildrenRecursively = [](ActorComponentNode* ParentNode, const USceneComponent* ParentComponent, auto& AddSceneComponentChildrenRecursivelyRef)
                -> void
                {
                    for (USceneComponent* ChildComponent : ParentComponent->GetAttachChildren())
                    {
                        ActorComponentNode* ChildNode = ParentNode->AddChild(ChildComponent->GetName(), ChildComponent->GetClass()->GetName(), ChildComponent);
                        AddSceneComponentChildrenRecursivelyRef(ChildNode, ChildComponent, AddSceneComponentChildrenRecursivelyRef);
                    }
                };

            // Add RootComponent's Children (i.e. SceneComponents)
            if (const USceneComponent* RootSceneComponent = SelectedActor->GetRootComponent())
            {
                AddSceneComponentChildrenRecursively(RootComponentNode, RootSceneComponent, AddSceneComponentChildrenRecursively);
            }

            // @todo 더 나은 방식 찾기
            // Add Non-SceneComponents
            for (UActorComponent* Component : SelectedActor->GetComponents())
            {
                if (!Component->IsA<USceneComponent>())
                {
                    RootActorNode->AddChild(Component->GetName(), Component->GetClass()->GetName(), Component);
                }
            }
        }
        bActorComponentNodeDirty = false;
    }

    RenderActorComponentTreeNodeTable(TableID, RootActorNode);
}
