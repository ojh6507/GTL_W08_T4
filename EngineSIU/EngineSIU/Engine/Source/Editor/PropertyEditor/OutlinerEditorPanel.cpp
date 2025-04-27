#include "OutlinerEditorPanel.h"
#include "World/World.h"
#include "GameFramework/Actor.h"
#include "Engine/EditorEngine.h"
#include <functional>

void OutlinerEditorPanel::Render()
{
    /* Pre Setup */
    ImGuiIO& io = ImGui::GetIO();

    const float PanelWidth = (Width) * 0.2f - 6.0f;
    const float PanelHeight = (Height) * 0.3f;

    const float PanelPosX = (Width) * 0.8f + 5.0f;
    constexpr float PanelPosY = 5.0f;

    constexpr ImVec2 MinSize(140, 100);
    constexpr ImVec2 MaxSize(FLT_MAX, 500);
    
    /* Min, Max Size */
    ImGui::SetNextWindowSizeConstraints(MinSize, MaxSize);
    
    /* Panel Position */
    ImGui::SetNextWindowPos(ImVec2(PanelPosX, PanelPosY), ImGuiCond_Always);

    /* Panel Size */
    ImGui::SetNextWindowSize(ImVec2(PanelWidth, PanelHeight), ImGuiCond_Always);

    /* Panel Flags */
    constexpr ImGuiWindowFlags PanelFlags = ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse;
    
    /* Render Start */
    ImGui::Begin("Outliner", nullptr, PanelFlags);

    ImGui::BeginChild("Objects");
    
    UEditorEngine* Engine = Cast<UEditorEngine>(GEngine);
    if (!Engine)
    {
        return;
    }

    std::function<void(AActor*)> CreateNode = [&CreateNode, &Engine](AActor* InActor)->void
    {
        const FString Name = InActor->GetActorLabel();

        // @todo Folder 추가 시 leaf 노드 Flag 분기를 추가하여야 함
        ImGuiTreeNodeFlags Flags = ImGuiTreeNodeFlags_None | ImGuiTreeNodeFlags_Leaf;

        // Draw Tree Node
        const bool bNodeOpen = ImGui::TreeNodeEx(*Name, Flags);

        if (ImGui::IsItemClicked())
        {
            Engine->SelectActor(InActor);
        }

        if (bNodeOpen)
        {
            //for (USceneComponent* Child : InActor->GetAttachChildren())
            //{
            //    CreateNode(Child);
            //}
            ImGui::TreePop(); // 트리 닫기
        }
    };

    // @todo World, Folder 구조 적용하기
    for (AActor* Actor : Engine->ActiveWorld->GetActiveLevel()->Actors)
    {
        CreateNode(Actor);
    }

    ImGui::EndChild();

    ImGui::End();
}
    
void OutlinerEditorPanel::OnResize(HWND hWnd)
{
    RECT ClientRect;
    GetClientRect(hWnd, &ClientRect);
    Width = ClientRect.right - ClientRect.left;
    Height = ClientRect.bottom - ClientRect.top;
}
