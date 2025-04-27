#pragma once

#include <filesystem>

#include "Components/ActorComponent.h"
#include "UnrealEd/EditorPanel.h"
#include "Math/Rotator.h"

class UStaticMeshComponent;

// 헬퍼 함수 예시
template<typename Getter, typename Setter>
void DrawColorProperty(const char* label, Getter get, Setter set)
{
    ImGui::PushItemWidth(200.0f);
    FLinearColor curr = get();
    float col[4] = { curr.R, curr.G, curr.B, curr.A };

    if (ImGui::ColorEdit4(label, col,
        ImGuiColorEditFlags_DisplayRGB |
        ImGuiColorEditFlags_NoSidePreview |
        ImGuiColorEditFlags_NoInputs |
        ImGuiColorEditFlags_Float))
    {
        set(FLinearColor(col[0], col[1], col[2], col[3]));
    }
    ImGui::PopItemWidth();
}

struct ActorComponentNode
{
    FString Name;
    FString Type;
    UActorComponent* Component;

    TArray<ActorComponentNode*> Children;

    ActorComponentNode(const FString& Name, const FString& Type, UActorComponent* Component)
        : Name(Name), Type(Type), Component(Component) {}

    ~ActorComponentNode()
    {
        for (const ActorComponentNode* Child : Children)
        {
            delete Child; // 동적 할당된 메모리 해제
        }
        Children.Empty();
    }

    ActorComponentNode* AddChild(FString Name, const FString& Type, UActorComponent* Component)
    {
        ActorComponentNode* Child = new ActorComponentNode(Name, Type, Component);
        Children.Add(Child);
        return Child;
    }

    void ClearChildren()
    {
        for (const ActorComponentNode* Child : Children)
        {
            delete Child; // 동적 할당된 메모리 해제
        }
        Children.Empty();
    }
};


class PropertyEditorPanel : public UEditorPanel
{
public:
    virtual void Render() override;
    virtual void OnResize(HWND hWnd) override;

private:
    void RenderActorComponentTreeNode(ActorComponentNode* Node);
    void RenderActorComponentTreeNodeTable(const char* TableID, ActorComponentNode* RootNode);
    void ShowActorComponents(const char* TableID);

private:
    void RGBToHSV(float r, float g, float b, float& h, float& s, float& v) const;
    void HSVToRGB(float h, float s, float v, float& r, float& g, float& b) const;

    /* Static Mesh Settings */
    void RenderForStaticMesh(UStaticMeshComponent* StaticMeshComp) const;
    
    /* Materials Settings */
    void RenderForMaterial(UStaticMeshComponent* StaticMeshComp);
    void RenderMaterialView(UMaterial* Material);
    void RenderCreateMaterialView();

    static void ShellExecuteOpen(const std::filesystem::path& FilePath);

private:
    float Width = 0, Height = 0;
    FVector Location = FVector(0, 0, 0);
    FRotator Rotation = FRotator(0, 0, 0);
    FVector Scale = FVector(0, 0, 0);
    FVector LightDirection = FVector(0, 0, 0);
    /* Material Property */
    int SelectedMaterialIndex = -1;
    int CurMaterialIndex = -1;
    UStaticMeshComponent* SelectedStaticMeshComp = nullptr;
    FObjMaterialInfo tempMaterialInfo;
    bool IsCreateMaterial;

    AActor* SelectedActor = nullptr;
    AActor* SelectedActorPrev = nullptr;
    UActorComponent* SelectedComponent = nullptr;
    UActorComponent* SelectedComponentPrev = nullptr;
    bool bActorComponentNodeDirty;
    ActorComponentNode* RootActorNode;
};
