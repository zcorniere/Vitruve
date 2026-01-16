#include "ECS/SystemScheduler.hxx"

#include "ECS/World.hxx"

#include "ECS/System.hxx"

bool ecs::FSystem::Call(RWorld* world) const
{
    return CallWrapper(world);
}

void ecs::FSystemScheduler::AddSystem(FSystem&& system)
{
    SystemStorage.Add(std::move(system));
}

void ecs::FSystemScheduler::Update(RWorld* world)
{

    for (unsigned i = 0; i < SystemStorage.Size(); i++)
    {
        if (!SystemStorage[i].Call(world))
        {
            // TODO: remove system if they fail to execute
        }
    }
}
