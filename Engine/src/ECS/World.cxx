#include "ECS/World.hxx"

#include "ECS/System.hxx"
#include "RHI/GenericRHI.hxx"

namespace ecs
{

RWorld::RWorld()
{
    Scene = Ref<RRHIScene>::Create(this);
    Scene->SetNamef("{}_Scene", GetName());
    RHI::Get()->RegisterScene(Scene);

    RegisterComponent<Vitruve::FUUID>();
    RegisterComponent<FDeltaTime>();

    RegisterSystem([World = this](FDeltaTime& Time) { Time.DeltaTime = World->GetDeltaTime(); });
}

RWorld::~RWorld()
{
}

void RWorld::Update(float InDeltaTime)
{
    fDeltaTime = InDeltaTime;
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
