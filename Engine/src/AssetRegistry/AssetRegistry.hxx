#pragma once

#include "AssetRegistry/Asset.hxx"

class  FAssetRegistry
{
public:
    FAssetRegistry();
    ENGINE_API ~FAssetRegistry();

    ENGINE_API Ref<RAsset> LoadAsset(const std::filesystem::path& Path);
    ENGINE_API Ref<RAsset> RegisterMemoryOnlyAsset(Ref<RAsset>& Asset);
    ENGINE_API Ref<RRHIMaterial> RegisterMemoryOnlyMaterial(Ref<RRHIMaterial>& Material);

    ENGINE_API Ref<RAsset> GetAsset(const std::string& Name) const;
    ENGINE_API Ref<RRHIMaterial> GetMaterial(const std::string& Name) const;
    ENGINE_API Ref<RAsset> GetAssetByID(uint64 ID) const;
    ENGINE_API Ref<RRHIMaterial> GetMaterialByID(uint64 ID) const;

    ENGINE_API void UnloadAsset(const std::string& Name);
    ENGINE_API void Purge();

    ENGINE_API Ref<RAsset> GetCubeAsset() const
    {
        return AssetRegistry["Box"];
    }
    ENGINE_API Ref<RAsset> GetCapsuleAsset() const
    {
        return AssetRegistry["Capsule"];
    }

private:
    TMap<uint64, Ref<RAsset>> AssetRegistryById;
    TMap<uint64, Ref<RRHIMaterial>> MaterialRegistryId;

    TMap<std::string, Ref<RAsset>> AssetRegistry;
    TMap<std::string, Ref<RRHIMaterial>> MaterialRegistry;
};
