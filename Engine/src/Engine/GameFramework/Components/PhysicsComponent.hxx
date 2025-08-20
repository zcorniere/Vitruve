#pragma once

#include "Engine/GameFramework/Components/SceneComponent.hxx"

class AActor;
class RWorld;

class RPhysicsComponent : public RSceneComponent
{
    RTTI_DECLARE_TYPEINFO(RPhysicsComponent, RSceneComponent)
public:
    RPhysicsComponent() = default;
    ~RPhysicsComponent() = default;

    void SetMass(float InMass)
    {
        Mass = InMass;
    }
    void SetVelocity(const FVector3& InVelocity)
    {
        Velocity = InVelocity;
    }
    void SetAcceleration(const FVector3& InAcceleration)
    {
        Acceleration = InAcceleration;
    }

private:
    void HandlePhysicsTick(RWorld* const OwnerWorld, AActor* const OwnerActor, float fDeltaTime);

private:
    float Mass = 1.0f;             // Default mass (1 kg)
    FVector3 Velocity = {};        // Default velocity
    FVector3 Acceleration = {};    // Default acceleration

    friend class RWorld;
};
