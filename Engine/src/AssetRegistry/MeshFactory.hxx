#pragma once

#include "AssetRegistry/Asset.hxx"

namespace MeshFactory
{

Ref<RModel> CreateBox(const FVector3& size);
Ref<RModel> CreateCapsule(float radius, float height);

}    // namespace MeshFactory
