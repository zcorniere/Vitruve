#pragma once

#include "GameFramework/Actor.hxx"
#include "GameFramework/Components/LightComponent.hxx"

class ENGINE_API ADirectionalLightActor : public AActor
{
    RTTI_DECLARE_TYPEINFO(ADirectionalLightActor, AActor)
public:
    ADirectionalLightActor();
    RDirectionalLightComponent* GetLight();

protected:
    virtual RObject* FindComponent(RTTI::FTypeId ID) override;

public:
    Ref<RDirectionalLightComponent> LightComponent = nullptr;
};

class ENGINE_API APointLightActor : public AActor
{
    RTTI_DECLARE_TYPEINFO(APointLightActor, AActor)
public:
    APointLightActor();
    RPointLightComponent* GetLight();

protected:
    virtual RObject* FindComponent(RTTI::FTypeId ID) override;

public:
    Ref<RPointLightComponent> LightComponent = nullptr;
};
