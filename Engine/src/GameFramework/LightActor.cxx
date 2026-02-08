#include "GameFramework/LightActor.hxx"

ADirectionalLightActor::ADirectionalLightActor()
{
    LightComponent = Ref<RDirectionalLightComponent>::Create();
}
RDirectionalLightComponent* ADirectionalLightActor::GetLight()
{
    return LightComponent.Raw();
}

RObject* ADirectionalLightActor::FindComponent(RTTI::FTypeId ID)
{
    if (LightComponent->TypeId() == ID)
    {
        return LightComponent.Raw();
    }
    return nullptr;
}

APointLightActor::APointLightActor()
{
    LightComponent = Ref<RPointLightComponent>::Create();
}
RPointLightComponent* APointLightActor::GetLight()
{
    return LightComponent.Raw();
}

RObject* APointLightActor::FindComponent(RTTI::FTypeId ID)
{
    if (LightComponent->TypeId() == ID)
    {
        return LightComponent.Raw();
    }
    return nullptr;
}
