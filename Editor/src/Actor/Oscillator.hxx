#pragma once

#include "GameFramework/Actor.hxx"

class AOscillator : public AActor
{
    RTTI_DECLARE_TYPEINFO(AOscillator, AActor)
public:
    AOscillator();
    virtual ~AOscillator();

    void BeginPlay() override;
    void Tick(double DeltaTime) override;

public:
    float Multiplier = 1.0f;
    FVector3 Direction = {0.0f, 0.0f, 0.0f};
    FVector3 Maximum = {0.0f, 0.0f, 0.0f};
    FVector3 Minimum = {0.0f, 0.0f, 0.0f};

    float ScaleMultiplier = 0.6f;
    float ScaleMaximum = 1.1f;
    float ScaleMinimum = 0.9f;

    FVector3 RotationMultiplier;
};
