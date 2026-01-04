#include "ECS/ECS.hxx"

#include "ECS/World.hxx"
#include "Engine/Core/Engine.hxx"

#include "ECS/Component/CameraComponent.hxx"
#include "ECS/Component/LightComponent.hxx"
#include "ECS/Component/MeshComponent.hxx"
#include "ECS/Component/RHIComponent.hxx"

namespace ecs
{

Ref<RWorld> CreateWorld()
{
    Ref<RWorld> NewWorld = Ref<RWorld>::Create();

    NewWorld->RegisterComponent<ecs::FTransformComponent>();
    NewWorld->RegisterComponent<ecs::FMeshComponent>();
    NewWorld->RegisterComponent<ecs::FCameraComponent>();
    NewWorld->RegisterComponent<ecs::FRenderTargetComponent>();
    NewWorld->RegisterComponent<ecs::FPointLightComponent>();
    NewWorld->RegisterComponent<ecs::FDirectionalLightComponent>();
    // Register the basic systems
    FSystem CollectRenderablesSystem(NewWorld->GetScene(), &RRHIScene::CollectRenderablesSystem);
    NewWorld->RegisterSystem(std::move(CollectRenderablesSystem));

    FSystem CameraSystem(NewWorld->GetScene(), &RRHIScene::CameraSystem);
    NewWorld->RegisterSystem(std::move(CameraSystem));

    FSystem CollectRenderTargets(NewWorld->GetScene(), &RRHIScene::CollectRenderTargets);
    NewWorld->RegisterSystem(std::move(CollectRenderTargets));

    return NewWorld;
}

void DestroyWorld(Ref<RWorld>& World)
{
    if (GEngine->GetWorld() == World)
    {
        GEngine->SetWorld(nullptr);
    }
    check(World->GetRefCount() == 1);
    World = nullptr;
}

}    // namespace ecs
