#pragma once
#include "Container/Map.h"
#include "Container/String.h"

class UEditorPanel;

class UnrealEd
{
public:
    UnrealEd() = default;
    ~UnrealEd() = default;
    void Initialize();
    
     void Render() const;
     void OnResize(HWND hWnd) const;
    
    void AddEditorPanel(const FString& PanelId, const std::shared_ptr<UEditorPanel>& EditorPanel);
    static std::shared_ptr<UEditorPanel> GetEditorPanel(const FString& PanelId);

private:
    static TMap<FString, std::shared_ptr<UEditorPanel>> Panels;
};

