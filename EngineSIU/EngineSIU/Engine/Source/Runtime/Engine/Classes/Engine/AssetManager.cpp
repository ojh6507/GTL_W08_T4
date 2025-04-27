#include "AssetManager.h"
#include "Engine.h"

#include <filesystem>
#include "Engine/FLoaderOBJ.h"

bool UAssetManager::IsInitialized()
{
    return GEngine && GEngine->AssetManager;
}

UAssetManager& UAssetManager::Get()
{
    if (UAssetManager* Singleton = GEngine->AssetManager)
    {
        return *Singleton;
    }
    else
    {
        UE_LOG(ELogLevel::Error, "Cannot use AssetManager if no AssetManagerClassName is defined!");
        assert(0);
        return *new UAssetManager; // never calls this
    }
}

UAssetManager* UAssetManager::GetIfInitialized()
{
    return GEngine ? GEngine->AssetManager : nullptr;
}

void UAssetManager::InitAssetManager()
{
    AssetRegistry = std::make_unique<FAssetRegistry>();

    LoadAssetFiles();
}

const TMap<FName, FAssetInfo>& UAssetManager::GetAssetRegistry()
{
    return AssetRegistry->PathNameToAssetInfo;
}

void UAssetManager::LoadAssetFiles() const
{
    const std::string BasePathName = "Contents/";

    // Obj 파일 로드
	
    for (const auto& Entry : std::filesystem::recursive_directory_iterator(BasePathName))
    {
        if (Entry.is_regular_file())
        {
            FAssetInfo NewAssetInfo;
            NewAssetInfo.AssetName = FName(Entry.path().filename().string());
            NewAssetInfo.PackagePath = FName(Entry.path().parent_path().string());
            NewAssetInfo.Size = static_cast<uint32>(std::filesystem::file_size(Entry.path()));
            NewAssetInfo.AssetType = EAssetType::None;  // 기본값

            if (Entry.path().extension() == ".obj")
            {
                NewAssetInfo.AssetType = EAssetType::StaticMesh;

                FString MeshName = NewAssetInfo.PackagePath.ToString() + "/" + NewAssetInfo.AssetName.ToString();
                 FManagerOBJ::CreateStaticMesh(MeshName);
            }
            else if (Entry.path().extension() == ".lua")
            {
                NewAssetInfo.AssetType = EAssetType::LuaScript;
            }

            AssetRegistry->PathNameToAssetInfo.Add(NewAssetInfo.AssetName, NewAssetInfo);
        }
    }
}
