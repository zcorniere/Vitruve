#include "RHI/RHIScene.hxx"

#include "ECS/World.hxx"

#include "Engine/Core/Engine.hxx"
#include "RHI/GenericRHI.hxx"
#include "RHI/RHI.hxx"

RRHIScene::RRHIScene(ecs::RWorld* OwnerWorld): Context(RHI::Get()->RHIGetCommandContext())
{
    u_CameraBuffer = RHI::CreateBuffer(FRHIBufferDesc{
        .Size = sizeof(UCameraData),
        .Stride = sizeof(UCameraData),
        .Usage = EBufferUsageFlags::UniformBuffer | EBufferUsageFlags::KeepCPUAccessible,
        .ResourceArray = nullptr,
        .DebugName = "Camera Buffer",
    });

    CameraData.View = FMatrix4::Identity();
    CameraData.Projection = FMatrix4::Identity();
    CameraData.ViewProjection = FMatrix4::Identity();

    /// WIP
    {
        UPointLight Light;
        Light.Position = {1.2, 1.0, 2.0};
        Light.Radiance = {1.0, 1.0, 1.0};
        Light.Radius = 2.0f;
        Light.Multiplier = 1.f;

        PointLights.Add(Light);

        UPointLightArray GPUArray;
        GPUArray.LightCount = PointLights.Size();
        std::memcpy(GPUArray.Lights, PointLights.Raw(), PointLights.Size() * sizeof(UPointLight));
        TResourceArray<UPointLightArray> Array;
        Array.Add(GPUArray);

        u_PointLightBuffer = RHI::CreateBuffer(FRHIBufferDesc{
            .Size = sizeof(UPointLightArray),
            .Stride = sizeof(UPointLightArray),
            .Usage = EBufferUsageFlags::UniformBuffer | EBufferUsageFlags::KeepCPUAccessible,
            .ResourceArray = &Array,
            .DebugName = "Point Light Buffer",
        });
    }

    {
        UDirectionalLight Light;
        Light.Direction = {0, 0, 0};
        Light.Multiplier = 1.f;
        Light.Radiance = {1.0f};

        DirectionalLights.Add(Light);

        UDirectionalLightArray GPUArray;
        GPUArray.LightCount = DirectionalLights.Size();
        std::memcpy(GPUArray.Lights, PointLights.Raw(), PointLights.Size() * sizeof(UPointLight));
        TResourceArray<UDirectionalLightArray> Array;
        Array.Add(GPUArray);

        u_DirectionalLightBuffer = RHI::CreateBuffer(FRHIBufferDesc{
            .Size = sizeof(UDirectionalLightArray),
            .Stride = sizeof(UDirectionalLightArray),
            .Usage = EBufferUsageFlags::UniformBuffer | EBufferUsageFlags::KeepCPUAccessible,
            .ResourceArray = &Array,
            .DebugName = "Directional Light Buffer",
        });
    }
    /// WIP
}

RRHIScene::~RRHIScene()
{
    RHI::Get()->RHIReleaseCommandContext(Context);
}

void RRHIScene::CollectRenderTargets(ecs::FRenderTargetComponent& InRenderPassTarget)
{
    if (InRenderPassTarget.AssignedSceneIndex == -1)
    {
        RenderPassTarget.Add(InRenderPassTarget);
        InRenderPassTarget.AssignedSceneIndex = RenderPassTarget.Size() - 1;
    }
    else
    {
        RenderPassTarget[InRenderPassTarget.AssignedSceneIndex] = InRenderPassTarget;
    }
}

void RRHIScene::CameraSystem(ecs::FCameraComponent& Camera, ecs::FRenderTargetComponent& RenderTarget,
                             ecs::FTransformComponent& Transform)
{
    // @TODO: support multiple cameras
    if (Camera.bIsActive)
    {
        const UVector2 Size = RenderTarget.GetRenderTargetSize();

        Camera.ViewPoint.SetAspectRatio(static_cast<float>(Size.x) / Size.y);
        CameraData.View = Camera.ViewPoint.GetViewMatrix(static_cast<FTransform&>(Transform));
        CameraData.Projection = Camera.ViewPoint.GetProjectionMatrix();
        CameraData.ViewProjection = CameraData.Projection * CameraData.View;
        bCameraDataDirty = true;
    }
}

void RRHIScene::Tick(double DeltaTime)
{
    VIT_PROFILE_FUNC()

    (void)DeltaTime;

    if (bCameraDataDirty)
    {
        VIT_PROFILE_FUNC("RRHIScene::Tick - UpdateCameraBuffer")

        ENQUEUE_RENDER_COMMAND(UpdateCameraBuffer)(
            [this](FFRHICommandList& CommandList)
            {
                TResourceArray<UCameraData> Array{CameraData};
                CommandList.CopyResourceArrayToBuffer(&Array, u_CameraBuffer, 0, 0, sizeof(UCameraData));
            });
    }
    {
        VIT_PROFILE_FUNC("RRHIScene::Tick - Update Transform Buffers - Wait")
        if (AsyncTaskUpdateResult.valid())
        {
            AsyncTaskUpdateResult.wait();
            AsyncTaskUpdateResult = std::future<void>();
        }
    }

    {
        VIT_PROFILE_FUNC("RRHIScene::Tick - Update Transform Buffers")
        for (auto& [AssetName, TransformArrays]: TransformResourceArray)
        {
            Ref<RRHIBuffer>& TransformBuffer = TransformBuffers.FindOrAdd(AssetName);
            if (TransformBuffer == nullptr || TransformBuffer->GetSize() < TransformArrays.Size())
            {
                TransformBuffer = RHI::CreateBuffer(FRHIBufferDesc{
                    .Size = static_cast<uint32>(std::max(TransformArrays.Size() * sizeof(FMatrix4),
                                                         sizeof(FMatrix4))),    // @TODO: not that
                    .Stride = sizeof(FMatrix4),
                    .Usage = EBufferUsageFlags::VertexBuffer | EBufferUsageFlags::KeepCPUAccessible,
                    .ResourceArray = &TransformArrays,
                    .DebugName = "Transform Buffer",
                });
            }
            else
            {
                ENQUEUE_RENDER_COMMAND(UpdateTransformBuffer)(
                    [TransformBuffer](FFRHICommandList& CommandList, TResourceArray<FMatrix4> TransformMatrices) mutable
                    {
                        CommandList.CopyResourceArrayToBuffer(&TransformMatrices, TransformBuffer, 0, 0,
                                                              TransformMatrices.GetByteSize());
                    },
                    TransformArrays);
            }
        }
    }
}

void RRHIScene::TickRenderer(FFRHICommandList& CommandList)
{
    VIT_PROFILE_FUNC()

    ecs::FRenderTargetComponent& RenderTarget = RenderPassTarget[0];
    UVector2 Size = RenderTarget.GetRenderTargetSize();
    TArray<FRHIRenderTarget> ColorTargets = RenderTarget.GetColorTargets();
    std::optional<FRHIRenderTarget> DepthTarget = RenderTarget.GetDepthTarget();

    if (RenderTarget.Viewport)
    {
        CommandList.SetViewport({0, 0, 0}, {static_cast<float>(RenderTarget.Viewport->GetSize().x),
                                            static_cast<float>(RenderTarget.Viewport->GetSize().y), 1.0f});
        CommandList.SetScissor({0, 0}, {RenderTarget.Viewport->GetSize().x, RenderTarget.Viewport->GetSize().y});
        CommandList.BeginRenderingViewport(RenderTarget.Viewport.Raw());
    }

    FRHIRenderPassDescription Description{
        .RenderAreaLocation = {0, 0},
        .RenderAreaSize = Size,
        .ColorTargets = ColorTargets,
        .DepthTarget = DepthTarget,
    };
    CommandList.BeginRendering(Description);

    {
        VIT_PROFILE_FUNC("RRHIScene::TickRenderer - Draw")
        for (auto& [Key, Requests]: RenderCalls)
        {
            if (Key.Asset->GetLocation() != EAssetLocation::LoadedVRAM)
            {
                Key.Asset->ChangeLocation(EAssetLocation::LoadedVRAM);
                continue;
            }
            CommandList.SetMaterial(Key.Material);

            Ref<RRHIBuffer> TransformVertexBuffer = TransformBuffers[Key.Asset->ID()];
            RModel::FDrawInfo Cube = Key.Asset->GetDrawInfo();

            ensure(Key.Asset->GetVertexBuffer() != nullptr);
            ensure(TransformVertexBuffer != nullptr);

            CommandList.SetVertexBuffer(Key.Asset->GetVertexBuffer(), 0, 0);
            CommandList.SetVertexBuffer(TransformVertexBuffer, 1, 0);
            CommandList.DrawIndexed(Key.Asset->GetIndexBuffer(), 0, 0, Cube.NumVertices, 0, Cube.NumPrimitives,
                                    Requests.Size());
        }
    }

    CommandList.EndRendering();

    if (RenderTarget.Viewport)
    {
        CommandList.EndRenderingViewport(RenderTarget.Viewport.Raw());
    }
}

// void RRHIScene::Async_UpdateActorRepresentations(FRHISceneUpdateBatch& Batch)
// {
//     VIT_PROFILE_FUNC()

//     Math::ComputeModelMatrixBatch(Batch.Actors.Size(), Batch.PositionX.Raw(), Batch.PositionY.Raw(),
//                                   Batch.PositionZ.Raw(), Batch.QuaternionX.Raw(), Batch.QuaternionY.Raw(),
//                                   Batch.QuaternionZ.Raw(), Batch.QuaternionW.Raw(), Batch.ScaleX.Raw(),
//                                   Batch.ScaleY.Raw(), Batch.ScaleZ.Raw(), Batch.MatrixArray.Raw());

//     for (unsigned i = 0; i < Batch.Actors.Size();)
//     {
//         VIT_PROFILE_FUNC("Update Actor Representations")

//         uint64 ActorId = Batch.Actors[i];
//         TRenderSceneLock<ERenderSceneLockType::Read> Lock(this);
//         TArray<FMeshRepresentation>* Iter = WorldActorRepresentation.Find(ActorId);
//         ensure(Iter);

//         for (FMeshRepresentation& Mesh: *Iter)
//         {
//             ensure(ActorId == Batch.Actors[i]);    //  make sure that we're still with the same actor
//             if (Mesh.Mesh->Asset == nullptr || Mesh.Mesh->Material == nullptr)
//             {
//                 continue;
//             }

//             // Update the transform resource array
//             TransformResourceArray.FindOrAdd(Mesh.Mesh->Asset->ID())[Mesh.TransformBufferIndex] =
//             Batch.MatrixArray[i]; Mesh.Transform.ModelMatrix = Batch.MatrixArray[i]; Mesh.Transform.bModelMatrixDirty
//             = false;

//             i++;
//         }
//     }
// }

FRHISceneUpdateBatch::FRHISceneUpdateBatch(unsigned Count)
{
    Actors.Reserve(Count);

    PositionX.Reserve(Count);
    PositionY.Reserve(Count);
    PositionZ.Reserve(Count);

    ScaleX.Reserve(Count);
    ScaleY.Reserve(Count);
    ScaleZ.Reserve(Count);

    QuaternionX.Reserve(Count);
    QuaternionY.Reserve(Count);
    QuaternionZ.Reserve(Count);
    QuaternionW.Reserve(Count);

    // Here we resize, because this is the output of the batch
    MatrixArray.Resize(Count);
}
