#include "Oscillator.hxx"

#include "AssetRegistry/AssetRegistry.hxx"
#include "Engine/Core/Engine.hxx"

FOscillator::FOscillator()
{
    Minimum = {0.0f, 0.0f, -6.0f};
    Maximum = {0.0f, 0.0f, 6.0f};
    Direction = {0.0f, 0.0f, 1.0f};

    ScaleMultiplier = (float)rand() / (float)RAND_MAX / 2.0f;

    RotationMultiplier.x = (float)rand() / (float)RAND_MAX / 20.0f;
    RotationMultiplier.y = (float)rand() / (float)RAND_MAX / 20.0f;
    RotationMultiplier.z = (float)rand() / (float)RAND_MAX / 20.0f;
}

void FOscillator::System(ecs::FTransformComponent& Transform, FOscillator& Oscillator)
{

    const FVector3 Delta = Oscillator.Direction * Oscillator.Multiplier;    // float(DeltaTime);
    const FVector3 NewLocation = Transform.GetLocation() + Delta;
    if (NewLocation.x > Oscillator.Maximum.x || NewLocation.x < Oscillator.Minimum.x)
    {
        Oscillator.Direction.x *= -1;
    }
    if (NewLocation.y > Oscillator.Maximum.y || NewLocation.y < Oscillator.Minimum.y)
    {
        Oscillator.Direction.y *= -1;
    }
    if (NewLocation.z > Oscillator.Maximum.z || NewLocation.z < Oscillator.Minimum.z)
    {
        Oscillator.Direction.z *= -1;
    }
    Transform.SetLocation(NewLocation);

    // if (GetName().contains("Box"))
    // {
    //     const FQuaternion DeltaRotation = FQuaternion::FromEulerAngles(RotationMultiplier.x * float(DeltaTime),
    //                                                                    RotationMultiplier.y * float(DeltaTime),
    //                                                                    RotationMultiplier.z * float(DeltaTime));

    //     const FQuaternion NewRotation = DeltaRotation * GetRootComponent()->GetRelativeTransform().GetRotation();
    //     SetActorRotation(NewRotation);
    // }
    // else if (GetName().contains("Capsule"))
    // {
    //     const FVector3 ScaledDelta = ScaleMultiplier * float(DeltaTime);
    //     const FVector3 NewScale = GetRootComponent()->GetRelativeTransform().GetScale() + ScaledDelta;

    //     if (NewScale.x > ScaleMaximum || NewScale.x < ScaleMinimum)
    //     {
    //         ScaleMultiplier *= -1;
    //     }

    //     SetActorScale(NewScale);
    // }
}
