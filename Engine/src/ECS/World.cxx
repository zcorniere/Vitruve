#include "ECS/World.hxx"

#include "ECS/System.hxx"
#include "RHI/GenericRHI.hxx"

namespace ecs
{

RWorld::RWorld()
{
    Scene = Ref<RRHIScene>::Create(this);
    RHI::Get()->RegisterScene(Scene);

    RegisterComponent<Vitruve::FUUID>();
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
    // Create a new entity and give it the UUID component
    return Storage.BuildEntity().WithComponent(Vitruve::FUUID());
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
