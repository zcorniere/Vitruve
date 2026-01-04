#include "ECS/SystemScheduler.hxx"

#include "ECS/World.hxx"

#include "ECS/System.hxx"

void ecs::FSystem::Call(RWorld* world) const
{
    CallWrapper(world);
}

void ecs::FSystemScheduler::AddSystem(FSystem&& system)
{
    SystemStorage.Add(std::move(system));
}

void ecs::FSystemScheduler::Update(RWorld* world)
{
    for (FSystem& system: SystemStorage)
    {
        system.Call(world);
    }
}
