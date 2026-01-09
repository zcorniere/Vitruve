#pragma once

#include "AssetRegistry/Asset.hxx"
#include "ECS/ECS.hxx"
#include "Engine/Math/Transform.hxx"
#include "RHI/Resources/RHIMaterial.hxx"

namespace ecs
{

struct FMeshComponent
{
    RTTI_DECLARE_TYPEINFO_MINIMAL(FMeshComponent);

public:
    Ref<RModel> Asset;
    Ref<RRHIMaterial> Material;

    FEntity RenderTarget;
};

template <typename T>
using TTransformComponent = ::Math::TTransform<T>;

using FTransformComponent = TTransformComponent<float>;
using DTransformComponent = TTransformComponent<double>;

}    // namespace ecs
