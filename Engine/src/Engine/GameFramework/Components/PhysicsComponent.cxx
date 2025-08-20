#include "Engine/GameFramework/Components/PhysicsComponent.hxx"

#include "Engine/GameFramework/Actor.hxx"
#include "Engine/GameFramework/World.hxx"

void RPhysicsComponent::HandlePhysicsTick(RWorld* const OwnerWorld, AActor* const OwnerActor, float DeltaTime)
{
    VIT_PROFILE_FUNC();

    FVector3 ActorLocation = OwnerActor->GetRelativeTransform().GetLocation();

    ActorLocation += Velocity * DeltaTime;
    Velocity += (Acceleration + OwnerWorld->GetGravity()) * DeltaTime;

    OwnerActor->SetActorLocation(ActorLocation);
}
