#include "Engine/GameFramework/World.hxx"

#include "Engine/Core/Engine.hxx"
#include "Engine/Core/RHI/GenericRHI.hxx"
#include "Engine/Core/RHI/RHIScene.hxx"

#include "Engine/GameFramework/Actor.hxx"
#include "Engine/GameFramework/Components/PhysicsComponent.hxx"

RWorld::RWorld()
{
    Scene = Ref<RRHIScene>::Create(this);
    RHI::Get()->RegisterScene(Scene);
}

RWorld::~RWorld()
{
    RHI::Get()->UnregisterScene(Scene);
}

void RWorld::SetName(std::string_view InName)
{
    Super::SetName(InName);
    Scene->SetNamef("{:s}.Scene", InName);
}

void RWorld::AddToWorld(Ref<AActor> Actor)
{
    Actors.Add(Actor);
    OnActorAddedToWorld.Broadcast(Actor.Raw());
}

void RWorld::RemoveFromWorld(Ref<AActor> Actor)
{
    OnActorRemovedFromWorld.Broadcast(Actor.Raw());
    Actors.Remove(Actor);
}

void RWorld::Tick(double DeltaTime)
{
    VIT_PROFILE_FUNC();

    Scene->PreTick();

    {
        VIT_PROFILE_FUNC("Actor Tick - parallel");
        std::shared_ptr<std::latch> Latch =
            GEngine->GetThreadPool().ParallelFor(Actors.Size(),
                                                 [this, DeltaTime](unsigned i)
                                                 {
                                                     Ref<AActor>& Actor = Actors[i];
                                                     HandleActorTick(Actor.Raw(), DeltaTime);
                                                 });
        Latch->wait();
    }

    Scene->PostTick(DeltaTime);
}

void RWorld::HandleActorTick(AActor* const Actor, double DeltaTime)
{
    VIT_PROFILE_FUNC();

    Actor->Tick(DeltaTime);

    RPhysicsComponent* Component = Actor->GetComponent<RPhysicsComponent>();
    if (Component)
    {
        Component->HandlePhysicsTick(this, Actor, DeltaTime);
    }

    RSceneComponent* const RootComponent = Actor->GetRootComponent();
    if (RootComponent->IsTransformDirty())
    {
        UpdateActorLocation(Actor->ID(), RootComponent);
        RootComponent->ClearDirtyTransformFlag();
    }
}

void RWorld::UpdateActorLocation(uint64 ID, RSceneComponent* const RootComponent)
{
    VIT_PROFILE_FUNC();

    const FTransform RelativeTransform = RootComponent->GetRelativeTransform();
    Scene->UpdateActorLocation(ID, RelativeTransform);
}

Ref<RRHIScene> RWorld::GetScene() const
{
    return Scene;
}

FVector3 RWorld::GetGravity() const
{
    return Gravity;
}
