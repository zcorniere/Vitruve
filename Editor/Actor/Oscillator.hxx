#pragma once

#include "Engine/GameFramework/Actor.hxx"

class RPhysicsComponent;

class AOscillator : public AActor
{
    RTTI_DECLARE_TYPEINFO(AOscillator, AActor)
public:
    AOscillator();
    virtual ~AOscillator();

    void Tick(double DeltaTime) override;

protected:
    virtual RObject* FindComponent(RTTI::FTypeId TypeId) override;

public:
    Ref<RPhysicsComponent> PhysicsComponent;

    float Multiplier = 1.0f;
    FVector3 Direction = {0.0f, 0.0f, 0.0f};
    FVector3 Maximum = {0.0f, 0.0f, 0.0f};
    FVector3 Minimum = {0.0f, 0.0f, 0.0f};
};
