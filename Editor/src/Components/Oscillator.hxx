#pragma once

#include "ECS/Component/MeshComponent.hxx"

struct FOscillator
{
public:
    static void System(ecs::FTransformComponent& Transform, FOscillator& Oscillator);

    RTTI_DECLARE_TYPEINFO_MINIMAL(FOscillator);

public:
    FOscillator();

    float Multiplier = 1.0f;
    FVector3 Direction = {0.0f, 0.0f, 0.0f};
    FVector3 Maximum = {0.0f, 0.0f, 0.0f};
    FVector3 Minimum = {0.0f, 0.0f, 0.0f};

    float ScaleMultiplier = 0.6f;
    float ScaleMaximum = 1.1f;
    float ScaleMinimum = 0.9f;

    FVector3 RotationMultiplier;
};
