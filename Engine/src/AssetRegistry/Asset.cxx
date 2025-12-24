#include "AssetRegistry/Asset.hxx"

#include "Engine/RHI/RHI.hxx"

RModel::RModel(const std::filesystem::path& Path): AssetPath(Path.string())
{
    SetName(Path.stem().string());
}

RModel::RModel(const TResourceArray<FVertex>& Vertices, const TResourceArray<uint32>& Indices)
{
    CurrentLocation = EAssetLocation::LoadedRAM;
    VertexData = Vertices;
    IndexData = Indices;
}

RModel::~RModel()
{
}

EAssetLocation RModel::GetLocation() const
{
    return CurrentLocation;
}

EAssetLocation RModel::ChangeLocation(EAssetLocation NewLocation)
{
    ensure(!IsLocationChanging());
    NextLocation = NewLocation;

    if (GetLocation() == NewLocation)
    {
        NextLocation = std::nullopt;
        return NewLocation;
    }
    else if (GetLocation() < NewLocation)
    {
        while (GetLocation() < NewLocation)
        {
            Load();
        }
        NextLocation = std::nullopt;
    }
    else
    {
        while (GetLocation() > NewLocation)
        {
            Unload();
            if (IsMemoryOnly() && GetLocation() == EAssetLocation::LoadedRAM)
            {
                break;
            }
        }
        NextLocation = std::nullopt;
    }

    return CurrentLocation;
}

bool RModel::IsLocationChanging() const
{
    return NextLocation.has_value();
}

bool RModel::Load()
{
    switch (CurrentLocation)
    {
        case EAssetLocation::Disk:
        {
            // We can't be on disk and a memory only, something is wrong
            check(!IsMemoryOnly());

            // TODO: Loading asset from disk
            CurrentLocation = EAssetLocation::LoadedRAM;
        }
        break;
        case EAssetLocation::LoadedRAM:
        {
            VertexBuffer = RHI::CreateBuffer(FRHIBufferDesc{
                .Size = VertexData.GetByteSize(),
                .Stride = sizeof(FVertex),
                .Usage = EBufferUsageFlags::VertexBuffer | EBufferUsageFlags::SourceCopy |
                         EBufferUsageFlags::KeepCPUAccessible,
                .ResourceArray = &VertexData,
                .DebugName = std::format("{:s}.StagingVertexBuffer", GetName()),
            });
            IndexBuffer = RHI::CreateBuffer(FRHIBufferDesc{
                .Size = IndexData.GetByteSize(),
                .Stride = sizeof(uint32),
                .Usage = EBufferUsageFlags::IndexBuffer | EBufferUsageFlags::SourceCopy |
                         EBufferUsageFlags::KeepCPUAccessible,
                .ResourceArray = &IndexData,
                .DebugName = std::format("{:s}.StagingIndexBuffer", GetName()),
            });

            ENQUEUE_RENDER_COMMAND(CopyBuffer)
            (
                [this](FFRHICommandList& CommandList)
                {
                    Ref<RRHIBuffer> NewVertexBuffer = RHI::CreateBuffer(FRHIBufferDesc{
                        .Size = VertexBuffer->GetSize(),
                        .Stride = VertexBuffer->GetStride(),
                        .Usage = EBufferUsageFlags::VertexBuffer | EBufferUsageFlags::DestinationCopy,
                        .ResourceArray = nullptr,
                        .DebugName = std::format("{:s}.VertexBuffer", GetName()),
                    });
                    Ref<RRHIBuffer> NewIndexBuffer = RHI::CreateBuffer(FRHIBufferDesc{
                        .Size = IndexBuffer->GetSize(),
                        .Stride = IndexBuffer->GetStride(),
                        .Usage = EBufferUsageFlags::IndexBuffer | EBufferUsageFlags::DestinationCopy,
                        .ResourceArray = nullptr,
                        .DebugName = std::format("{:s}.IndexBuffer", GetName()),
                    });
                    CommandList.CopyBufferToBuffer(VertexBuffer, NewVertexBuffer, 0, 0, NewVertexBuffer->GetSize());
                    CommandList.CopyBufferToBuffer(IndexBuffer, NewIndexBuffer, 0, 0, NewIndexBuffer->GetSize());

                    RHI::RHIWaitUntilIdle();    // #TODO: not that

                    VertexBuffer = NewVertexBuffer;
                    IndexBuffer = NewIndexBuffer;
                });
            CurrentLocation = EAssetLocation::LoadedVRAM;
        }
        break;
        case EAssetLocation::LoadedVRAM:
        {
            // We're already load in full, nothing to do here
        }
        break;
    }

    return true;
}

bool RModel::Unload()
{
    switch (CurrentLocation)
    {
        case EAssetLocation::LoadedVRAM:
        {
            VertexBuffer = nullptr;
            IndexBuffer = nullptr;

            CurrentLocation = EAssetLocation::LoadedRAM;
        }
        break;
        case EAssetLocation::LoadedRAM:
        {
            if (!IsMemoryOnly())
            {
                VertexData.Clear();
                IndexData.Clear();

                CurrentLocation = EAssetLocation::Disk;
            }
        }
        break;
        case EAssetLocation::Disk:
        {
            // We don't have a whole lot to unload in this case
        }
        break;
    }
    return true;
}
