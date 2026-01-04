#pragma once

#include "ECS/Component/RHIComponent.hxx"
#include "Engine/Math/Transform.hxx"
#include "Engine/Threading/Lock.hxx"
#include "RHI/RHICommandList.hxx"
#include "RHI/RHIContext.hxx"

#include "ECS/Component/CameraComponent.hxx"
#include "ECS/Component/MeshComponent.hxx"
#include "ECS/Component/RHIComponent.hxx"

#include <future>

namespace ecs
{
class RWorld;
}    // namespace ecs

class RRHIScene;
class RModel;
class AActor;
class RRHIMaterial;

enum class ERenderSceneLockType
{
    Read,
    Write,
};

struct FRenderRequestKey
{
    RRHIMaterial* Material = nullptr;
    RModel* Asset = nullptr;

    bool operator==(const FRenderRequestKey& Other) const = default;
};

namespace std
{

// std::hash specialization for FRenderRequestKey
template <>
struct hash<FRenderRequestKey>
{
    FORCEINLINE std::size_t operator()(const FRenderRequestKey& Key) const
    {
        return std::hash<RModel*>{}(Key.Asset) ^ std::hash<RRHIMaterial*>{}(Key.Material);
    }
};

}    // namespace std

struct FRHISceneUpdateBatch
{
    FRHISceneUpdateBatch(unsigned Count);

    TArray<FMatrix4, 64> MatrixArray;

    TArray<uint64> Actors;

    TArray<float, 64> PositionX;
    TArray<float, 64> PositionY;
    TArray<float, 64> PositionZ;

    TArray<float, 64> ScaleX;
    TArray<float, 64> ScaleY;
    TArray<float, 64> ScaleZ;

    TArray<float, 64> QuaternionX;
    TArray<float, 64> QuaternionY;
    TArray<float, 64> QuaternionZ;
    TArray<float, 64> QuaternionW;
};

template <ERenderSceneLockType LockType>
class TRenderSceneLock
{
public:
    TRenderSceneLock(WeakRef<RRHIScene> InScene);
    ~TRenderSceneLock();

private:
    WeakRef<RRHIScene> Scene = nullptr;
};

class RRHIScene : public RObject
{
    RTTI_DECLARE_TYPEINFO(RRHIScene, RObject);

public:
    BEGIN_PARAMETER_STRUCT(UCameraData)
    PARAMETER(FMatrix4, ViewProjection)
    PARAMETER(FMatrix4, View)
    PARAMETER(FMatrix4, Projection)
    END_PARAMETER_STRUCT();

    BEGIN_PARAMETER_STRUCT(UPointLight)
    PARAMETER(FVector3, Position)
    PARAMETER(float, Multiplier)
    PARAMETER(FVector3, Radiance)
    PARAMETER(float, MinRadius)
    PARAMETER(float, Radius)
    PARAMETER(float, Falloff)
    PARAMETER(float, LightSize)
    PARAMETER(bool, CastsShadows)
    END_PARAMETER_STRUCT();

    BEGIN_PARAMETER_STRUCT(UDirectionalLight)
    PARAMETER(FVector3, Direction)
    PARAMETER(float, ShadowAmount)
    PARAMETER(FVector3, Radiance)
    PARAMETER(float, Multiplier)
    END_PARAMETER_STRUCT();

private:
    struct UPointLightArray
    {
        uint32 LightCount;
        UPointLight Lights[1024];
    };

    struct UDirectionalLightArray
    {
        uint32 LightCount;
        UDirectionalLight Lights[4];
    };

    struct FMeshRepresentation
    {
        FTransform Transform = {};
        uint32 TransformBufferIndex = std::numeric_limits<uint32>::max();
        uint32 RenderBufferIndex = std::numeric_limits<uint32>::max();
        // WeakRef<RMeshComponent> Mesh = nullptr;
    };

    struct FActorRepresentationUpdateRequest
    {
        uint64 ActorId = 0;
        FTransform NewTransform = {};
    };

public:
    RRHIScene() = delete;
    RRHIScene(ecs::RWorld* OuterWorld);
    virtual ~RRHIScene();

    WeakRef<RRHIViewport> GetViewport(unsigned ViewportIndex = 0) const
    {
        return RenderPassTarget[ViewportIndex].Viewport;
    }

    ENGINE_API void CollectRenderablesSystem(ecs::FMeshComponent& Mesh, ecs::FTransformComponent& Transform);
    ENGINE_API void CameraSystem(ecs::FCameraComponent& Camera, ecs::FRenderTargetComponent& InRenderPassTarget,
                                 ecs::FTransformComponent& Transform);
    ENGINE_API void CollectRenderTargets(ecs::FRenderTargetComponent& RenderTarget);

    void Tick(double DeltaTime);

    ENGINE_API void TickRenderer(FFRHICommandList& CommandList);

private:
    void Async_UpdateActorRepresentations(FRHISceneUpdateBatch& Batch);

private:
    TArray<ecs::FRenderTargetComponent> RenderPassTarget;

    TArray<UPointLight> PointLights;
    Ref<RRHIBuffer> u_PointLightBuffer = nullptr;

    TArray<UDirectionalLight> DirectionalLights;
    Ref<RRHIBuffer> u_DirectionalLightBuffer = nullptr;

    bool bCameraDataDirty = true;
    UCameraData CameraData{};
    Ref<RRHIBuffer> u_CameraBuffer = nullptr;

    TMap<uint64, TResourceArray<FMatrix4>> TransformResourceArray;
    TMap<uint64, Ref<RRHIBuffer>> TransformBuffers;
    TMap<FRenderRequestKey, TArray<FMeshRepresentation*>> RenderCalls;

    TMap<uint64, TArray<FMeshRepresentation>> WorldActorRepresentation;

    FRWFifoLock ContextLock;
    FRHIContext* const Context = nullptr;

    std::future<void> AsyncTaskUpdateResult;

    std::mutex ActorAttentionMutex;
    TMap<uint64, FActorRepresentationUpdateRequest> ActorThatNeedAttention;

    template <ERenderSceneLockType LockType>
    friend class TRenderSceneLock;
};

template <ERenderSceneLockType LockType>
inline TRenderSceneLock<LockType>::TRenderSceneLock(WeakRef<RRHIScene> InScene): Scene(InScene)
{
    if constexpr (LockType == ERenderSceneLockType::Read)
    {
        VIT_PROFILE_FUNC("TRenderSceneLock - Read Lock");
        Scene->ContextLock.ReadLock();
    }
    else
    {
        VIT_PROFILE_FUNC("TRenderSceneLock - Write Lock");
        Scene->ContextLock.WriteLock();
    }
}

template <ERenderSceneLockType LockType>
inline TRenderSceneLock<LockType>::~TRenderSceneLock()
{
    if constexpr (LockType == ERenderSceneLockType::Read)
    {
        VIT_PROFILE_FUNC("TRenderSceneLock - Read Unlock");
        Scene->ContextLock.ReadUnlock();
    }
    else
    {
        VIT_PROFILE_FUNC("TRenderSceneLock - Write Unlock");
        Scene->ContextLock.WriteUnlock();
    }
}
