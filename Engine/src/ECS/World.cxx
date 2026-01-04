#include "ECS/World.hxx"

#include "ECS/System.hxx"
#include "RHI/GenericRHI.hxx"

namespace ecs
{

RWorld::RWorld()
{
    Scene = Ref<RRHIScene>::Create(this);
    RHI::Get()->RegisterScene(Scene);
}

RWorld::~RWorld()
{
}

void RWorld::Update(float DeltaTime)
{
    fDeltaTime = DeltaTime;

    Scheduler.Update(this);
    Scene->Tick(fDeltaTime);
}

FEntityBuilder RWorld::CreateEntity()
{
    return Storage.BuildEntity();
}

void RWorld::DestroyEntity(FEntity EntityID)
{
    Storage.DestroyEntity(EntityID);
}

void RWorld::RegisterSystem(FSystem&& system)
{
    Scheduler.AddSystem(std::move(system));
}

}    // namespace ecs
