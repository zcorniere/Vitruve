#pragma once

#include "ECS/ECS.hxx"

#include "ECS/ComponentStorage.hxx"
#include "ECS/SystemScheduler.hxx"
#include "RHI/RHIScene.hxx"

namespace ecs
{

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
