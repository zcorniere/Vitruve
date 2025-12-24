#include "AssetRegistry/AssetRegistry.hxx"

#include "AssetRegistry.hxx"
#include "AssetRegistry/MeshFactory.hxx"

DECLARE_LOGGER_CATEGORY(Core, LogAssetRegistry, Info)

FAssetRegistry::FAssetRegistry()
{
    Ref<RModel> CubeAsset = MeshFactory::CreateBox({1.0f, 1.0f, 1.0f});
    RegisterMemoryOnlyAsset(CubeAsset);

    Ref<RModel> CapsuleAsset = MeshFactory::CreateCapsule(1.0f, 2.0f);
    RegisterMemoryOnlyAsset(CapsuleAsset);
}

FAssetRegistry::~FAssetRegistry()
{
}

Ref<RModel> FAssetRegistry::LoadAsset(const std::filesystem::path& Path)
{
    auto Asset = Ref<RModel>::Create(Path);
    Asset->ChangeLocation(EAssetLocation::LoadedRAM);

    {
        AssetRegistry.Insert(Asset->GetName(), Asset);
        AssetRegistryById.Insert(Asset->ID(), Asset);
        return Asset;
    }
    return nullptr;
}

Ref<RModel> FAssetRegistry::RegisterMemoryOnlyAsset(Ref<RModel>& Asset)
{
    if (!AssetRegistry.Contains(Asset->GetName()))
    {
        AssetRegistry.Insert(Asset->GetName(), Asset);
        AssetRegistryById.Insert(Asset->ID(), Asset);
        return Asset;
    }
    LOG(LogAssetRegistry, Warning, "Asset {:s} already registered", Asset->GetName());
    return nullptr;
}

Ref<RRHIMaterial> FAssetRegistry::RegisterMemoryOnlyMaterial(Ref<RRHIMaterial>& Material)
{
    if (!MaterialRegistry.Contains(Material->GetName()))
    {
        MaterialRegistryId.Insert(Material->ID(), Material);
        MaterialRegistry.Insert(Material->GetName(), Material);
        return Material;
    }
    LOG(LogAssetRegistry, Warning, "Material {:s} already registered", Material->GetName());
    return nullptr;
}

Ref<RModel> FAssetRegistry::GetAsset(const std::string& Name) const
{
    const Ref<RModel>* Asset = AssetRegistry.Find(Name);
    if (Asset)
    {
        return *Asset;
    }
    LOG(LogAssetRegistry, Warning, "Asset {:s} not found", Name);
    return nullptr;
}

Ref<RRHIMaterial> FAssetRegistry::GetMaterial(const std::string& Name) const
{
    const Ref<RRHIMaterial>* Material = MaterialRegistry.Find(Name);
    if (Material)
    {
        return *Material;
    }
    LOG(LogAssetRegistry, Warning, "Material {:s} not found", Name);
    return nullptr;
}

Ref<RModel> FAssetRegistry::GetAssetByID(uint64 ID) const
{
    const Ref<RModel>* Asset = AssetRegistryById.Find(ID);
    if (Asset)
    {
        return *Asset;
    }
    LOG(LogAssetRegistry, Warning, "Asset with ID {:d} not found", ID);
    return nullptr;
}

Ref<RRHIMaterial> FAssetRegistry::GetMaterialByID(uint64 ID) const
{
    const Ref<RRHIMaterial>* Material = MaterialRegistryId.Find(ID);
    if (Material)
    {
        return *Material;
    }
    LOG(LogAssetRegistry, Warning, "Material with ID {:d} not found", ID);
    return nullptr;
}

void FAssetRegistry::UnloadAsset(const std::string& Name)
{
    Ref<RModel>* Asset = AssetRegistry.Find(Name);
    if (Asset)
    {
        (*Asset)->ChangeLocation(EAssetLocation::Disk);
        AssetRegistry.Remove(Name);
    }
}

void FAssetRegistry::Purge()
{
    for (auto& [Name, Asset]: AssetRegistry)
    {
        Asset->ChangeLocation(EAssetLocation::Disk);
    }
    AssetRegistry.Clear();
    AssetRegistryById.Clear();

    MaterialRegistry.Clear();
    MaterialRegistryId.Clear();
}
