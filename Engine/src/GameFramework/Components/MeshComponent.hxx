#pragma once

#include "AssetRegistry/Asset.hxx"
#include "GameFramework/Components/SceneComponent.hxx"

class RMeshComponent : public RSceneComponent
{
    RTTI_DECLARE_TYPEINFO(RMeshComponent, RSceneComponent)
public:
    RMeshComponent() = default;
    ~RMeshComponent() = default;

    void SetAsset(Ref<RAsset> InAsset);
    void SetMaterial(Ref<RRHIMaterial> InMaterial);

    Ref<RAsset> GetAsset() const;
    Ref<RRHIMaterial> GetMaterial() const;

public:
    Ref<RAsset> Asset = nullptr;
    Ref<RRHIMaterial> Material = nullptr;
};

inline void RMeshComponent::SetAsset(Ref<RAsset> InAsset)
{
    Asset = InAsset;
    MarkRenderStateDirty();
}

inline void RMeshComponent::SetMaterial(Ref<RRHIMaterial> InMaterial)
{
    Material = InMaterial;
    MarkRenderStateDirty();
}

inline Ref<RAsset> RMeshComponent::GetAsset() const
{
    return Asset;
}

inline Ref<RRHIMaterial> RMeshComponent::GetMaterial() const
{
    return Material;
}
