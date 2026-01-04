#pragma once

namespace ecs
{

class FPointLightComponent
{
    RTTI_DECLARE_TYPEINFO_MINIMAL(FPointLightComponent)
public:
    FColor LightColor = {255.f, 255.f, 255.f};

    float Multiplier = 1.0f;
    float MinRadius = 0.0f;
    float Radius = 0.0f;
    float Falloff = 0.0f;
    float LightSize = 0.0f;
};

class FDirectionalLightComponent
{
    RTTI_DECLARE_TYPEINFO_MINIMAL(FDirectionalLightComponent)

public:
    FColor LightColor = {255.f, 255.f, 255.f};
    float Multiplier = 1.0f;
    FVector3 Direction = {0, 0, 0};
    float ShadowAmount = 0;
};

}    // namespace ecs
