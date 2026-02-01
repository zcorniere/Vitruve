#pragma once

#include "GameFramework/Components/SceneComponent.hxx"

class RLightComponent : public RSceneComponent
{
    RTTI_DECLARE_TYPEINFO(RLightComponent, RSceneComponent)

public:
    FColor LightColor = {255.f, 255.f, 255.f};

    float Multiplier = 1.0f;
};

class RPointLightComponent : public RLightComponent
{
    RTTI_DECLARE_TYPEINFO(RPointLightComponent, RLightComponent)
public:
    float MinRadius = 0.0f;
    float Radius = 0.0f;
    float Falloff = 0.0f;
    float LightSize = 0.0f;
};

class RDirectionalLightComponent : public RLightComponent
{
    RTTI_DECLARE_TYPEINFO(RDirectionalLightComponent, RLightComponent)

public:
    FVector3 Direction = {0, 0, 0};
    float ShadowAmount = 0;
};
