#pragma once

#include "Engine/Containers/ResourceArray.hxx"
#include "RHI/Resources/RHIBuffer.hxx"

BEGIN_UNALIGNED_PARAMETER_STRUCT(FVertex)
PARAMETER(FVector3, Position)
PARAMETER(FVector3, Normal)
PARAMETER(FVector3, Tangant)
PARAMETER(FVector3, Binormal)
PARAMETER(UVector2, Texcoord)
END_PARAMETER_STRUCT();

enum class EAssetLocation : uint8
{
    Disk,
    LoadedRAM,
    LoadedVRAM,
};

class RAsset : public RObject
{
    RTTI_DECLARE_TYPEINFO(RAsset, RObject)

public:
    virtual EAssetLocation GetLocation() const = 0;

    virtual EAssetLocation ChangeLocation(EAssetLocation NewStatus) = 0;
    virtual bool IsLocationChanging() const = 0;
};

class RModel : public RAsset
{
    RTTI_DECLARE_TYPEINFO(RModel, RAsset);

public:
    struct FDrawInfo
    {
        uint32 NumVertices;
        uint32 NumIndices;
        uint32 NumPrimitives;
    };

public:
    RModel(const std::filesystem::path& Path);
    RModel(const TResourceArray<FVertex>& Vertices, const TResourceArray<uint32>& Indices);
    ~RModel();

    virtual EAssetLocation GetLocation() const override;

    virtual EAssetLocation ChangeLocation(EAssetLocation NewStatus) override;
    virtual bool IsLocationChanging() const override;

    const Ref<RRHIBuffer> GetVertexBuffer() const
    {
        return VertexBuffer;
    }

    const Ref<RRHIBuffer> GetIndexBuffer() const
    {
        return IndexBuffer;
    }

    bool IsMemoryOnly() const
    {
        return AssetPath.empty();
    }
    FDrawInfo GetDrawInfo() const
    {
        return {VertexData.Size(), IndexData.Size(), IndexData.Size()};
    }

private:
    bool Load();
    bool Unload();

private:
    EAssetLocation CurrentLocation = EAssetLocation::Disk;
    std::optional<EAssetLocation> NextLocation = std::nullopt;

    std::string AssetPath = "";

    Ref<RRHIBuffer> VertexBuffer = nullptr;
    Ref<RRHIBuffer> IndexBuffer = nullptr;

    TResourceArray<FVertex> VertexData;
    TResourceArray<uint32> IndexData;
};
