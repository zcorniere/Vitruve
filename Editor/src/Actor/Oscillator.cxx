#include "Oscillator.hxx"

#include "AssetRegistry/AssetRegistry.hxx"
#include "Engine/Core/Engine.hxx"

AOscillator::AOscillator()
{
    Minimum = {0.0f, 0.0f, -6.0f};
    Maximum = {0.0f, 0.0f, 6.0f};
    Direction = {0.0f, 0.0f, 1.0f};

    ScaleMultiplier = (float)rand() / (float)RAND_MAX / 2.0f;

    RotationMultiplier.x = (float)rand() / (float)RAND_MAX / 20.0f;
    RotationMultiplier.y = (float)rand() / (float)RAND_MAX / 20.0f;
    RotationMultiplier.z = (float)rand() / (float)RAND_MAX / 20.0f;

    Ref<RModel> UsedAsset;
    if (rand() % 2)
    {
        UsedAsset = GEngine->AssetRegistry.GetCapsuleAsset();
    }
    else
    {
        UsedAsset = GEngine->AssetRegistry.GetCubeAsset();
    }

    GetMesh()->SetAsset(UsedAsset);
    GetMesh()->SetMaterial(GEngine->AssetRegistry.GetMaterial("Shape Material"));
}

AOscillator::~AOscillator()
{
}

void AOscillator::BeginPlay()
{
    Super::BeginPlay();
    SetNamef("{}_{}", GetName(), GetMesh()->GetAsset()->GetName());
}

void AOscillator::Tick(double DeltaTime)
{
    Super::Tick(DeltaTime);

    const FVector3 Delta = Direction * Multiplier * float(DeltaTime);
    const FVector3 NewLocation = GetRootComponent()->GetRelativeTransform().GetLocation() + Delta;
    if (NewLocation.x > Maximum.x || NewLocation.x < Minimum.x)
    {
        Direction.x *= -1;
    }
    if (NewLocation.y > Maximum.y || NewLocation.y < Minimum.y)
    {
        Direction.y *= -1;
    }
    if (NewLocation.z > Maximum.z || NewLocation.z < Minimum.z)
    {
        Direction.z *= -1;
    }
    SetActorLocation(NewLocation);

    if (GetName().contains("Box"))
    {
        const FQuaternion DeltaRotation = FQuaternion::FromEulerAngles(RotationMultiplier.x * float(DeltaTime),
                                                                       RotationMultiplier.y * float(DeltaTime),
                                                                       RotationMultiplier.z * float(DeltaTime));

        const FQuaternion NewRotation = DeltaRotation * GetRootComponent()->GetRelativeTransform().GetRotation();
        SetActorRotation(NewRotation);
    }
    else if (GetName().contains("Capsule"))
    {
        const FVector3 ScaledDelta = ScaleMultiplier * float(DeltaTime);
        const FVector3 NewScale = GetRootComponent()->GetRelativeTransform().GetScale() + ScaledDelta;

        if (NewScale.x > ScaleMaximum || NewScale.x < ScaleMinimum)
        {
            ScaleMultiplier *= -1;
        }

        SetActorScale(NewScale);
    }
}
