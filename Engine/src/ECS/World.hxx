#pragma once

#include "ECS/ECS.hxx"

#include "ECS/ComponentStorage.hxx"
#include "ECS/SystemScheduler.hxx"
#include "RHI/RHIScene.hxx"

namespace ecs
{

template <typename T>
struct TDeltaTime
{
    RTTI_DECLARE_TYPEINFO_MINIMAL(TDeltaTime<T>);

public:
    T DeltaTime = 0.0;

    operator T() const
    {
        return DeltaTime;
    }
};
using FDeltaTime = TDeltaTime<float>;
using DDeltaTime = TDeltaTime<double>;

class RWorld : public RObject
{
    RTTI_DECLARE_TYPEINFO(RWorld, RObject);

public:
    RWorld();
    ~RWorld();

    RRHIScene* GetScene()
    {
        return Scene.Raw();
    }

    void Update(float DeltaTime);

    ENGINE_API FEntityBuilder CreateEntity();
    ENGINE_API void DestroyEntity(FEntity EntityID);

    template <typename T>
    void RegisterComponent()
    {
        Storage.RegisterComponent<T>();
    }

    ENGINE_API void RegisterSystem(FSystem&& system);

    FSystemScheduler& GetScheduler()
    {
        return Scheduler;
    }

    FComponentStorage& GetComponentStorage()
    {
        return Storage;
    }

    float GetDeltaTime() const
    {
        return fDeltaTime;
    }

private:
    float fDeltaTime;

    Ref<RRHIScene> Scene;

    FSystemScheduler Scheduler;
    FComponentStorage Storage;
};

}    // namespace ecs

#include "ECS/System.hxx"
