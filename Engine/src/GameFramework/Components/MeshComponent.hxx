#pragma once

#include "AssetRegistry/Asset.hxx"
#include "GameFramework/Components/SceneComponent.hxx"

class RMeshComponent : public RSceneComponent
{
    RTTI_DECLARE_TYPEINFO(RMeshComponent, RSceneComponent)
public:
    RMeshComponent() = default;
    ~RMeshComponent() = default;

    void SetAsset(Ref<RModel> InAsset);
    void SetMaterial(Ref<RRHIMaterial> InMaterial);

    Ref<RModel> GetAsset() const;
    Ref<RRHIMaterial> GetMaterial() const;

public:
    Ref<RModel> Asset = nullptr;
    Ref<RRHIMaterial> Material = nullptr;
};

inline void RMeshComponent::SetAsset(Ref<RModel> InAsset)
{
    Asset = InAsset;
    MarkRenderStateDirty();
}

inline void RMeshComponent::SetMaterial(Ref<RRHIMaterial> InMaterial)
{
    Material = InMaterial;
    MarkRenderStateDirty();
}

inline Ref<RModel> RMeshComponent::GetAsset() const
{
    return Asset;
}

inline Ref<RRHIMaterial> RMeshComponent::GetMaterial() const
{
    return Material;
}
