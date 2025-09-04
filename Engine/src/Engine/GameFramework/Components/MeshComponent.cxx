#include "Engine/GameFramework/Components/MeshComponent.hxx"

FBox RMeshComponent::GetBoundingBox() const
{
    if (Asset)
    {
        return Asset->GetBoundingBox().Transform(GetRelativeTransform());
    }
    return FBox();
}
