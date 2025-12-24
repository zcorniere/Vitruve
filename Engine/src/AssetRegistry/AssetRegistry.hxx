#pragma once

#include "AssetRegistry/Asset.hxx"

class FAssetRegistry
{
public:
    FAssetRegistry();
    ENGINE_API ~FAssetRegistry();

    ENGINE_API Ref<RModel> LoadAsset(const std::filesystem::path& Path);
    ENGINE_API Ref<RModel> RegisterMemoryOnlyAsset(Ref<RModel>& Asset);
    ENGINE_API Ref<RRHIMaterial> RegisterMemoryOnlyMaterial(Ref<RRHIMaterial>& Material);

    ENGINE_API Ref<RModel> GetAsset(const std::string& Name) const;
    ENGINE_API Ref<RRHIMaterial> GetMaterial(const std::string& Name) const;
    ENGINE_API Ref<RModel> GetAssetByID(uint64 ID) const;
    ENGINE_API Ref<RRHIMaterial> GetMaterialByID(uint64 ID) const;

    ENGINE_API void UnloadAsset(const std::string& Name);
    ENGINE_API void Purge();

    ENGINE_API Ref<RModel> GetCubeAsset() const
    {
        return AssetRegistry["Box"];
    }
    ENGINE_API Ref<RModel> GetCapsuleAsset() const
    {
        return AssetRegistry["Capsule"];
    }

private:
    TMap<uint64, Ref<RModel>> AssetRegistryById;
    TMap<uint64, Ref<RRHIMaterial>> MaterialRegistryId;

    TMap<std::string, Ref<RModel>> AssetRegistry;
    TMap<std::string, Ref<RRHIMaterial>> MaterialRegistry;
};
